//Devin Bidstrup U19115995
//EC440 Operating Systems 
#include "fs.h"
#include "disk.h"

/******************************************************************************/
struct super_block
{
    short fat_idx;    // First block of the FAT
    short fat_len;    // Length of FAT in blocks
    short dir_idx;    // First block of directory
    short dir_len;    // Length of directory in blocks
    short data_idx;   // First block of file-data
};

struct dir_entry
{
    bool used;                   // Is this file-”slot” in use
    char name [MAX_F_NAME + 1]; 
    int size;                   // file size
    short head;                 // first data block of file
    int8_t ref_cnt;             // how many open file descriptors are there?// ref_cnt > 0 -> cannot delete file
};

struct file_descriptor
{
    bool used;          // fdin use
    short file;         // the first block of the file (f) to which fd refers to
    short offset;       // position of fd within f
};
/******************************************************************************/
struct super_block fs;
struct file_descriptor fildes[MAX_FILDES]; // 32 
short FAT[FAT_SIZE];                       // Will be populated with the FAT data
struct dir_entry DIR[MAX_F_NUM];           // Will be populated with the directory data

int make_fs(char* disk_name)
{
    return FAILURE;
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