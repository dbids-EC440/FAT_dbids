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
struct file_descriptor fildesArray[MAX_FILDES]; // 32 
unsigned short FAT[FAT_SIZE];                   // Will be populated with the FAT data
struct dir_entry DIR[MAX_F_NUM];                // Will be populated with the directory data

//Writes the superblock to the disk based on the current fs
int writeSuperBlock()
{
    char buffer[BLOCK_SIZE] = {0};
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
    int index = 0;
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
    int index = 0;
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

//Reorganizes the FAT after something is removed from it, or a new block is allocated to a file
//i.e. defragments it
int FATsync()
{
    unsigned short emptyIndex;
    int defrag = 0;
    int i;
    for (i = 0; i < FAT_SIZE; i++)
    {
        //Once an empty block is found, when you find the next file in the FAT, copy it
        if (defrag && (FAT[i] != EMPTY))
        {
            while (FAT[i] != END_OF_FILE)
            {
                FAT[emptyIndex + i] = FAT[i];
                i++;
            }
            FAT[emptyIndex + i] = FAT[i];
            defrag = 0;
        }
        //If you find an empty block then set emptyIndex to it
        else if (FAT[i] == EMPTY)
        {
            defrag = 1;
            emptyIndex = i;
        }        
    }
    
    return -1;
}

//Creates an empty file system on virtual disk with disk_name
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
    for (i = 0; i < FAT_SIZE; i++)
    {
        FAT[i] = EMPTY;
    }
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

//Mounts fs from virtual disk disk_name
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

    //Set all file descriptors to not used
    int i;
    for (i = 0; i < MAX_FILDES; i++)
    {
        fildesArray[i].used = 0;
    }

    return SUCCESS;
}

//Unmounts fs from virtual disk disk_name
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
    
    //Check that all file descriptors are not used
    int i;
    for (i = 0; i < MAX_FILDES; i++)
    {
        if(fildesArray[i].used)
        {
            return FAILURE;
        }  
    }
    
    return SUCCESS; 
}

//File specified by name is opened for reading/writing
int fs_open(char* name)
{
    //Check the length of name
    int length = strnlen(name, MAX_F_NAME+1);
    if (length == MAX_F_NAME+1)
    {
        return FAILURE;
    }
    
    //Find the next open file descriptor
    int fildes;
    for (fildes = 0; fildes < MAX_FILDES; fildes++)
    {
        if(!fildesArray[fildes].used)
            break;
    }
    //Check that there was an open file descriptor
    if (fildes == MAX_FILDES)
    {
        return FAILURE;
    }

    //Find the directory entry for this file
    int directoryExists;
    int d;
    for (d = 0; (d < MAX_F_NUM) && (!directoryExists); d++)
    {
        if (strncmp(DIR[d].name, name, length) == 0)
        {
            directoryExists = 1;            
        }            
    }

    //Check if a directory entry exists for the file (it should)
    if (!directoryExists)
    {
        return FAILURE;
    }

    //Change directory ref count for the file and update file descriptor array
    DIR[d].ref_cnt++;
    fildesArray[fildes].file = DIR[d].head;
    fildesArray[fildes].offset = 0;

    return fildes;
}

//File descriptor fildes is closed
int fs_close(int fildes)
{
    //Check that the file descriptor is within the bounds
    if ((fildes < 0) || (fildes >= MAX_FILDES))
    {
        return FAILURE;
    }
        
    //Check that the file is open
    if(!fildesArray[fildes].used) 
    {
        return FAILURE;
    }    
        
    //Close the file descriptor
    fildesArray[fildes].used = 0;

    return SUCCESS;
}

//Creates a new file with name name in the fs
int fs_create(char* name)
{
    //Check the length of name
    int length = strnlen(name, MAX_F_NAME+1);
    if (length == MAX_F_NAME+1)
    {
        return FAILURE;
    }
    
    //Simultaneously check that a directory entry doesn't exist for this file
    //and that there are less than 64 files
    int d;
    int emptyDir = -1;
    for (d = 0; (d < MAX_F_NUM); d++)
    {
        if (strcmp(DIR[d].name, name) == 0)
        {
            return FAILURE;          
        }

        if ((emptyDir == -1) && (!DIR[d].used))
            emptyDir = d;
    }
    
    //Check if there are already 64 files in use
    if (emptyDir == -1)
    {
        return FAILURE;
    }
    
    //Find the next open block to use
    int b, f;
    int emptyBlock = 0;
    for (b = fs.data_idx; (b < DISK_BLOCKS) && (!emptyBlock); b++)
    {
        //Check if block b exists in the FAT
        for (f = 0; (f < FAT_SIZE) && (FAT[f] != b); f++);
        
        //If it does not than we have found the first empty block
        if (f == FAT_SIZE)
            emptyBlock = b;
    }
    //Check that we have found an empty block
    if (emptyBlock == 0)
    {
        return FAILURE;
    }

    //Allocate the directory information
    DIR[emptyDir].used = 1;
    strncpy(DIR[emptyDir].name, name, MAX_F_NAME+1);
    DIR[emptyDir].size = 0;
    DIR[emptyDir].head = (unsigned short) emptyBlock;
    DIR[emptyDir].ref_cnt = 0;

    //Find an empty index in the FAT
    f = 0;
    while (FAT[f] != EMPTY && (f < FAT_SIZE))
    {
         f++;
    }
    
    //Check that if found a place in the FAT
    if ((f >= FAT_SIZE) || (f+1 >= FAT_SIZE))
    {
        return FAILURE;
    }

    //Add the block to the FAT
    FAT[f] = (unsigned short) emptyBlock;
    FAT[f+1] = END_OF_FILE;

    return SUCCESS;
}

//Deletes the file named name and frees all corresponding meta_info and data
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