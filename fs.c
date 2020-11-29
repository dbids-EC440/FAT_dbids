//Devin Bidstrup U19115995
//EC440 Operating Systems 
#include "fs.h"
#include "disk.h"

/******************************************************************************/
struct super_block
{
    unsigned short fat_idx;    // First block of the FAT
    unsigned short fat_len;    // Length of FAT in blocks
    unsigned short dir_idx;    // First block of directory
    unsigned short dir_len;    // Length of directory in blocks
    unsigned short data_idx;   // First block of file-data
};

struct dir_entry
{
    int used;                   // Is this file-”slot” in use
    char name [MAX_F_NAME + 1]; 
    int size;                   // file size
    unsigned short head;        // first data block of file
    int8_t ref_cnt;             // how many open file descriptors are there?// ref_cnt > 0 -> cannot delete file
};

struct file_descriptor
{
    int used;                    // fdin use
    unsigned short file;         // the first block of the file (f) to which fd refers to
    unsigned short offset;       // position of fd within f
};
/******************************************************************************/
struct super_block fs;
struct file_descriptor fildes[MAX_FILDES]; // 32 
unsigned short FAT[FAT_SIZE];                       // Will be populated with the FAT data
struct dir_entry DIR[MAX_F_NUM];           // Will be populated with the directory data

//Writes the superblock to the disk based on the current fs
int writeSuperBlock()
{
    char buffer[BLOCK_SIZE];
    sprintf(&buffer[0], "%4hu%4hu%4hu%4hu%4hu", fs.fat_idx, fs.fat_len, fs.dir_idx, fs.dir_len, fs.data_idx);
    
    if(block_write(SUPERBLOCK_LOC, &buffer[0]) == -1)
    {
        return FAILURE;
    }

    return SUCCESS;
}

//Reads the superblock from the disk and updates the fs
int readSuperBlock()
{
    //Read the superblock from the disk
    char buffer[BLOCK_SIZE];
    if(block_read(SUPERBLOCK_LOC, buffer) == -1)
    {
        return FAILURE;
    }

    sscanf(&buffer[0], "%4hu%4hu%4hu%4hu%4hu", &fs.fat_idx, &fs.fat_len, &fs.dir_idx, &fs.dir_len, &fs.data_idx);

    return SUCCESS;
}

//Writes the formatted directory to the disk
int writeDirectory(unsigned short dir_idx)
{
    //Get all of the directory contents into a string
    int i;
    int index;
    char w_dir_str[BLOCK_SIZE] = {0};
    for (i = 0; i < MAX_F_NUM; i++)
    {
        index += sprintf(&w_dir_str[index], "%1d%16s%8d%4hu%2hhd", DIR[i].used, DIR[i].name, DIR[i].size, DIR[i].head, DIR[i].ref_cnt);
    }

    //Write the directory string to disk
    if (block_write(dir_idx, &w_dir_str[0]) == -1)
    {
        return FAILURE;
    }

    return SUCCESS;
}

//Reads the formatted directory from the disk
int readDirectory(unsigned short dir_idx)
{
    //Define strings for reading
    char r_dir_str[BLOCK_SIZE];

    //Read the directory from the disk
    if (block_read(dir_idx, r_dir_str) == -1)
    {
        return FAILURE;
    }

    //Parse the read directory string to the directory struct
    int i;
    for (i = 0; i < MAX_F_NUM; i++)
    {
        sscanf(&r_dir_str[i*36], "%1d%16s%8d%4hu%2hhd", &DIR[i].used, DIR[i].name, &DIR[i].size, &DIR[i].head, &DIR[i].ref_cnt);
        //sscanf(&r_dir_str[i*dirSize], "%d ", &DIR[i].used);
        //strncpy(DIR[i].name, &r_dir_str[(i*dirSize)+sizeof(int)], sizeof(char[MAX_F_NAME + 1]));
        //sscanf(&r_dir_str[(i*dirSize) + sizeof(int) + sizeof(char[MAX_F_NAME + 1])], "%d%hu%hhd", &DIR[i].size, &DIR[i].head, &DIR[i].ref_cnt);
    }
    return SUCCESS;
}

//Writes the FAT to the disk
int writeFAT(unsigned short fat_idx, unsigned int fat_len)
{
    //Assmble the string representing the FAT
    int i;
    char FAT_str[FAT_LEN * BLOCK_SIZE] = {0};
    int index;
    for (i = 0; i < FAT_SIZE; i++)
    {
        index += sprintf(&FAT_str[index], "%4hu", FAT[i]);
    }

    //Write the string to each block
    for (i = fat_idx; i < fat_len + fat_idx; i++)
    {
        if (block_write(i, &FAT_str[BLOCK_SIZE * (i - fat_idx)]) == -1)
        {
            return FAILURE;
        }
    }

    return SUCCESS;
}

//Reads the FAT from the disk and updates the global var
int readFAT(unsigned short fat_idx, unsigned int fat_len)
{
    //Read the FAT from the disk, need to loop to get all the blocks it is within
    char FAT_str[FAT_LEN * BLOCK_SIZE] = {0};
    int i;
    for (i = fat_idx; i < fat_len+fat_idx; i++)
    {
        if (block_read(i, &FAT_str[BLOCK_SIZE * (i - fat_idx)]) == -1)
        {
            return FAILURE;
        }
    }
    
    for (i = 0; i < FAT_SIZE; i++)
        sscanf(&FAT_str[i], "%4hu", &FAT[i]);

    return SUCCESS;
}

int make_fs(char* disk_name)
{
    //Call make_disk
    if (make_disk(disk_name) == -1)
    {
        return FAILURE;
    }

    //Open the disk
    if (open_disk(disk_name) == -1)
    {
        return FAILURE;
    }

    //Initialize default values for superblock
    fs.fat_idx = FAT_IDX;
    fs.fat_len = FAT_LEN;
    fs.dir_idx = DIR_IDX;
    fs.dir_len = DIR_LEN;
    fs.data_idx = DATA_IDX;

    //Write Empty Superblock
    if (writeSuperBlock() == -1)
    {
        return FAILURE;
    }

    //Write Empty Directory
    int i;
    for (i = 0; i < MAX_F_NUM; i++)
    {
        DIR[i].used = 0;
    }
    if (writeDirectory(fs.dir_idx) == -1)
    {
        return FAILURE;
    }

    //Write Empty FAT
    if (writeFAT(fs.fat_idx, fs.fat_len) == -1)
    {
        return FAILURE;
    }

    //Close the disk
    if (close_disk() == -1)
    {
        return FAILURE;
    }

    return SUCCESS;
}

int mount_fs(char* disk_name)
{
    //Open the disk
    if (open_disk(disk_name) == -1)
    {
        return FAILURE;
    }

    //Read the super block
    if (readSuperBlock() == -1)
    {
        return FAILURE;
    }

    //Read the directory
    if (readDirectory(fs.dir_idx) == -1)
    {
        return FAILURE;
    }

    //Read the FAT
    if (readFAT(fs.fat_idx, fs.fat_len) == -1)
    {
        return FAILURE;
    }

    return SUCCESS;
}

int umount_fs(char* disk_name)
{
    //Write Updated Directory
    if (writeDirectory(fs.dir_idx) == -1)
    {
        return FAILURE;
    }

    //Write Updated FAT
    if (writeFAT(fs.fat_idx, fs.fat_len) == -1)
    {
        return FAILURE;
    }

    //Close the disk
    if (close_disk() == -1)
    {
        return FAILURE;
    }
   
   return SUCCESS; 
}

int fs_open(char* name)
{
    return FAILURE;
}

int fs_close(int fildes)
{
    return FAILURE;
}

int fs_create(char* name)
{
    return FAILURE;
}

int fs_delete(char* name)
{
    return FAILURE;
}

int fs_read(int fildes, void* buf, size_t nbyte)
{
    return FAILURE;
}

int fs_write(int fildes, void* buf, size_t nbyte)
{
    return FAILURE;
}

int fs_get_filesize(int fildes)
{
    return FAILURE;
}

int fs_listfiles(char*** files)
{
    return FAILURE;
}

int fs_lseek(int fildes, off_t offset)
{
    return FAILURE;
}

int fs_truncate(int fildes, off_t length)
{
    return FAILURE;
}