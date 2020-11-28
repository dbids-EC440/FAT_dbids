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
    bool used;                   // Is this file-”slot” in use
    char name [MAX_F_NAME + 1]; 
    int size;                   // file size
    unsigned short head;        // first data block of file
    int8_t ref_cnt;             // how many open file descriptors are there?// ref_cnt > 0 -> cannot delete file
};

struct file_descriptor
{
    bool used;          // fdin use
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
    //char* buffer = (char*) malloc(sizeof(struct super_block));
    char buffer[BLOCK_SIZE];
    sprintf(&buffer[0], "%hu%hu%hu%hu%hu", fs.fat_idx, fs.fat_len, fs.dir_idx, fs.dir_len, fs.data_idx);
    
    int error = block_write(0, buffer);
    if (error == -1)
    {
        return FAILURE;
    }

    //free(buffer);

    return SUCCESS;
}

//Reads the superblock from the disk and updates the fs
int readSuperBlock()
{
    //Read the superblock from the disk
    //char* buffer = (char*) malloc(sizeof(struct super_block));
    char buffer[BLOCK_SIZE];
    int error = block_read(0, buffer);
    if (error == -1)
    {
        return FAILURE;
    }
    
    sscanf(&buffer[0], "%hu%hu%hu%hu%hu", fs.fat_idx, fs.fat_len, fs.dir_idx, fs.dir_len, fs.data_idx);
    //Use strncpy to parse the read string and assigned the values to the superblock
    //char* readShort = (char*) malloc(sizeof(unsigned short));
    /*char readShort[sizeof(unsigned short)];
    strncpy(readShort, &buffer[0], 2);
    fs.fat_idx = (unsigned short) *readShort;
    strncpy(readShort, &buffer[2], 2);
    fs.fat_len = (unsigned short) *readShort;
    strncpy(readShort, &buffer[4], 2);
    fs.dir_idx = (unsigned short) *readShort;
    strncpy(readShort, &buffer[6], 2);
    fs.dir_len = (unsigned short) *readShort;
    strncpy(readShort, &buffer[8], 2);
    fs.data_idx = (unsigned short) *readShort;*/

    //Free the memory
    //free(buffer);
    //free(readShort);

    return SUCCESS;
}

//Writes the formatted directory to the disk
int writeDirectory(unsigned short dir_idx)
{
    //Get all of the directory contents into a string
    int i;
    int index;
    //char* dir_entry_str = (char*) malloc(sizeof(struct dir_entry));
    //char* directory_str = (char*) malloc(MAX_F_NUM * sizeof(struct dir_entry));
    //char dir_entry_str[sizeof(struct dir_entry)];
    char w_dir_str[(MAX_F_NUM * sizeof(struct dir_entry)) + 1] = {0};
    //sprintf(w_dir_str, "%hhd%s%d%hu%hhd", DIR[0].used, DIR[0].name, DIR[0].size, DIR[0].head, DIR[0].ref_cnt);
    for (i = 0; i < MAX_F_NUM; i++)
    {
        index += sprintf(&w_dir_str[index], "%hhd%s%d%hu%hhd", DIR[i].used, DIR[i].name, DIR[i].size, DIR[i].head, DIR[i].ref_cnt);
        //strncat(w_dir_str, dir_entry_str, sizeof(struct dir_entry));
    }

    //Write the directory string to disk
    int error = block_write(dir_idx, w_dir_str);
    if (error == -1)
    {
        return FAILURE;
    }

    //free(dir_entry_str);
    //free(directory_str);

    return SUCCESS;
}

//Reads the formatted directory from the disk
int readDirectory(unsigned short dir_idx)
{
    int i;
    int dirSize = sizeof(struct dir_entry);

    //Define strings for reading
    //char* readBool = (char*) malloc(sizeof(bool));
    //char* readInt = (char*) malloc(sizeof(int));
    //char* readShort = (char*) malloc(sizeof(unsigned short));
    //char* readIntEight = (char*) malloc(sizeof(int8_t));
    //char* r_dir_str = (char*) malloc(MAX_F_NUM * dirSize);
    char r_dir_str[BLOCK_SIZE];

    //Read the directory from the disk
    int error = block_read(dir_idx, r_dir_str);
    if (error == -1)
    {
        return FAILURE;
    }

    //Parse the read directory string to the directory struct
    for (i = 0; i < MAX_F_NUM; i++)
    {
        sscanf(&r_dir_str[i+dirSize], "%hhd%s%d%hu%hhd", DIR[i].used, DIR[i].name, DIR[i].size, DIR[i].head, DIR[i].ref_cnt);
        //strIndex = 0;

        //strncpy(readBool, &directory_str[(i*dirSize)+strIndex], sizeof(bool));
        //DIR[i].used = (bool) *readBool;
        //memcpy(&DIR[i].used, &r_dir_str[(i*dirSize)+strIndex], sizeof(bool));
        //strIndex += sizeof(bool);

        //strncpy(DIR[i].name, &r_dir_str[(i*dirSize)+strIndex], sizeof(char[MAX_F_NAME + 1]));
        //strIndex += sizeof(char[MAX_F_NAME + 1]);

        //strncpy(readInt, &directory_str[(i*dirSize)+strIndex], sizeof(int));
        //DIR[i].size = (int) *readInt;
        //memcpy(&DIR[i].size, &r_dir_str[(i*dirSize)+strIndex], sizeof(int));
        //strIndex += sizeof(int);

        //strncpy(readShort, &directory_str[(i*dirSize)+strIndex], sizeof(unsigned short));
        //DIR[i].head = (unsigned short) *readShort;
        //memcpy(&DIR[i].head, &r_dir_str[(i*dirSize)+strIndex], sizeof(unsigned short));
        //strIndex += sizeof(unsigned short);

        //strncpy(readIntEight, &directory_str[(i*dirSize)+strIndex], sizeof(int8_t));
        //DIR[i].ref_cnt = (int8_t) *readIntEight;
        //memcpy(&DIR[i].ref_cnt, &r_dir_str[(i*dirSize)+strIndex], sizeof(int8_t));
    }

    //Free the string
    //free(readBool);
    //free(readInt);
    //free(readShort);
    //free(readIntEight);
    //free(r_dir_str);

    return SUCCESS;
}

//Writes the FAT to the disk
int writeFAT(unsigned short fat_idx, unsigned int fat_len)
{
    //Assmble the string representing the FAT
    int i;
    char FAT_str[((FAT_SIZE*sizeof(unsigned short)) / BLOCK_SIZE) + (FAT_SIZE % BLOCK_SIZE != 0) * BLOCK_SIZE] = {0};
    int index;
    for (i = 0; i < FAT_SIZE; i++)
    {
        index += sprintf(&FAT_str[index], "%hu", FAT[i]);
    }

    //Write the string to each block
    int i;
    for (i = fat_idx; i < fat_len+fat_idx; i++)
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
    char FAT_str[((FAT_SIZE*sizeof(unsigned short)) / BLOCK_SIZE) + (FAT_SIZE % BLOCK_SIZE != 0) * BLOCK_SIZE] = {0};
    int i;
    for (i = fat_idx; i < fat_len+fat_idx; i++)
    {
        if (block_read(i, &FAT_str[BLOCK_SIZE * (i - fat_idx)]) == -1)
        {
            return FAILURE;
        }

        //strncpy(&FAT_str[FAT_SIZE * (i - fat_idx)], &buffer[0], BLOCK_SIZE);
    }
    
    for (i = 0; i < FAT_SIZE; i++)
        sscanf(&FAT_str[i*sizeof(unsigned short)], "%hu", FAT[i]);

    return SUCCESS;
}

int make_fs(char* disk_name)
{
    //Call make_disk
    int error = make_disk(disk_name);
    if(error == -1)
    {
        return FAILURE;
    }

    //Open the disk
    error = open_disk(disk_name);
    if(error == -1)
    {
        return FAILURE;
    }

    //Initialize default values for superblock
    fs.fat_idx = 2;
    fs.fat_len = 5;
    fs.dir_idx = 1;
    fs.dir_len = 1;
    fs.data_idx = 7;

    //Write Empty Superblock
    error = writeSuperBlock();
    if(error == -1)
    {
        return FAILURE;
    }

    //Write Empty Directory
    int i;
    for (i = 0; i < MAX_F_NUM; i++)
    {
        DIR[i].used = 0;
    }
    error = writeDirectory(fs.dir_idx);
    if(error == -1)
    {
        return FAILURE;
    }

    //Write Empty FAT
    error = writeFAT(fs.fat_idx);
    if(error == -1)
    {
        return FAILURE;
    }

    //Close the disk
    error = close_disk();
    if(error == -1)
    {
        return FAILURE;
    }

    return SUCCESS;
}

int mount_fs(char* disk_name)
{
    //Open the disk
    int error = open_disk(disk_name);
    if(error == -1)
    {
        return FAILURE;
    }

    //Read the super block
    error = readSuperBlock();
    if(error == -1)
    {
        return FAILURE;
    }

    //Read the directory
    error = readDirectory(fs.dir_idx);
    if(error == -1)
    {
        return FAILURE;
    }

    //Read the FAT
    error = readFAT(fs.fat_idx);
    if(error == -1)
    {
        return FAILURE;
    }

    return SUCCESS;
}

int umount_fs(char* disk_name)
{
    //Write Updated Directory
    int error = writeDirectory(fs.dir_idx);
    if(error == -1)
    {
        return FAILURE;
    }

    //Write Updated FAT
    writeFAT(fs.fat_idx);
    if(error == -1)
    {
        return FAILURE;
    }

    //Close the disk
    error = close_disk();
    if(error == -1)
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