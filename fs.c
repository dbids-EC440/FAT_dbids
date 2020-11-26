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
void writeSuperBlock()
{
    char* buffer = malloc(sizeof(struct super_block));
    sprintf(buffer, "%hu%hu%hu%hu%hu", fs.fat_idx, fs.fat_len, fs.dir_idx, fs.dir_len, fs.data_idx);
    block_write(0, buffer);
    free(buffer);
}

//Reads the superblock from the disk and updates the fs
void readSuperBlock()
{
    //Read the superblock from the disk
    char* buffer = malloc(sizeof(struct super_block));
    block_read(0, buffer);

    //Use strncpy to parse the read string and assigned the values to the superblock
    char* readShort = malloc(sizeof(unsigned short));
    strncpy(readShort, &buffer[0], 2);
    fs.fat_idx = (unsigned short) readShort;
    strncpy(readShort, &buffer[2], 2);
    fs.fat_len = (unsigned short) readShort;
    strncpy(readShort, &buffer[4], 2);
    fs.dir_idx = (unsigned short) readShort;
    strncpy(readShort, &buffer[6], 2);
    fs.dir_len = (unsigned short) readShort;
    strncpy(readShort, &buffer[8], 2);
    fs.data_idx = (unsigned short) readShort;

    //Free the memory
    free(buffer);
    free(readShort);
}

//Writes the formatted directory to the disk
void writeDirectory()
{
    int i;
    char* dir_entry_str = malloc(sizeof(struct dir_entry));
    char* directory_str = malloc(sizeof(struct dir_entry));
    DIR[0].used = 1;
    sprintf(directory_str, "%hhd%s%d%hu%hhd", DIR[0].used, DIR[0].name, DIR[0].size, DIR[0].head, DIR[0].ref_cnt);
    for (i = 1; i < MAX_F_NUM; i++)
    {
        DIR[i].used = 0;
        sprintf(dir_entry_str, "%hhd%s%d%hu%hhd", DIR[i].used, DIR[i].name, DIR[i].size, DIR[i].head, DIR[i].ref_cnt);
        strncat(directory_str, dir_entry_str);
    }
    block_write(0, directory_str);
    free(dir_entry_str);
    free(directory_str);
}

//Reads the formatted directory from the disk
void readDirectory()
{

}

//Writes the FAT to the disk
void writeFAT()
{
    int i;
    char* element = malloc(sizeof(unsigned short));
    char* FAT_str = malloc(sizeof(unsigned short));
    sprintf(FAT_str, "%hu", FAT[0]);
    for (i = 1; i < FAT_SIZE; i++)
    {
        sprintf(element, "%hu", FAT[i]);
        strncat(FAT_str, element);
    }
    block_write(0, FAT_str);
    free(FAT_str);
    free(element);
}

//Reads the FAT from the disk and updates the global var
void readFAT()
{

}

int make_fs(char* disk_name)
{
    int error;

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

    //Write Empty Superblock
    //Initialize default values first
    fs.fat_idx = 1;
    fs.fat_len = 5;
    fs.dir_idx = 1;
    fs.dir_len = 1;
    fs.data_idx = 7;
    writeSuperBlock();

    //Write Empty Directory
    writeDirectory();

    //Write Empty FAT
    writeFAT();

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
    return FAILURE;
}

int umount_fs(char* disk_name)
{
   return FAILURE; 
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