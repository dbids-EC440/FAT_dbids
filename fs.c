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
    char* buffer = (char*) malloc(sizeof(struct super_block));
    sprintf(buffer, "%hu%hu%hu%hu%hu", fs.fat_idx, fs.fat_len, fs.dir_idx, fs.dir_len, fs.data_idx);
    block_write(0, buffer);
    free(buffer);
}

//Reads the superblock from the disk and updates the fs
void readSuperBlock()
{
    //Read the superblock from the disk
    char* buffer = (char*) malloc(sizeof(struct super_block));
    block_read(0, buffer);

    //Use strncpy to parse the read string and assigned the values to the superblock
    char* readShort = (char*) malloc(sizeof(unsigned short));
    strncpy(readShort, &buffer[0], 2);
    fs.fat_idx = (unsigned short) *readShort;
    strncpy(readShort, &buffer[2], 2);
    fs.fat_len = (unsigned short) *readShort;
    strncpy(readShort, &buffer[4], 2);
    fs.dir_idx = (unsigned short) *readShort;
    strncpy(readShort, &buffer[6], 2);
    fs.dir_len = (unsigned short) *readShort;
    strncpy(readShort, &buffer[8], 2);
    fs.data_idx = (unsigned short) *readShort;

    //Free the memory
    free(buffer);
    free(readShort);
}

//Writes the formatted directory to the disk
void writeDirectory(unsigned short dir_idx)
{
    int i;
    char* dir_entry_str = (char*) malloc(sizeof(struct dir_entry));
    char* directory_str = (char*) malloc(MAX_F_NUM * sizeof(struct dir_entry));
    DIR[0].used = 1;
    sprintf(directory_str, "%hhd%s%d%hu%hhd", DIR[0].used, DIR[0].name, DIR[0].size, DIR[0].head, DIR[0].ref_cnt);
    for (i = 1; i < MAX_F_NUM; i++)
    {
        DIR[i].used = 0;
        sprintf(dir_entry_str, "%hhd%s%d%hu%hhd", DIR[i].used, DIR[i].name, DIR[i].size, DIR[i].head, DIR[i].ref_cnt);
        strncat(directory_str, dir_entry_str, sizeof(struct dir_entry));
    }
    block_write(dir_idx, directory_str);
    free(dir_entry_str);
    free(directory_str);
}

//Reads the formatted directory from the disk
void readDirectory(unsigned short dir_idx)
{
    int i;
    int dirSize = sizeof(struct dir_entry);
    int strIndex = 0;

    //Define strings for reading
    char* readBool = (char*) malloc(sizeof(bool));
    char* readInt = (char*) malloc(sizeof(int));
    char* readShort = (char*) malloc(sizeof(unsigned short));
    char* readIntEight = (char*) malloc(sizeof(int8_t));

    //Read the directory from the disk
    char* directory_str = (char*) malloc(MAX_F_NUM * dirSize);
    block_read(dir_idx, directory_str);

    //Parse the read string to the struct
    for (i = 0; i < MAX_F_NUM; i++)
    {
        strIndex = 0;

        strncpy(readBool, &directory_str[(i*dirSize)+strIndex], sizeof(bool));
        DIR[i].used = (bool) *readBool;
        strIndex += sizeof(bool);

        strncpy(DIR[i].name, &directory_str[(i*dirSize)+strIndex], sizeof(char[MAX_F_NAME + 1]));
        strIndex += sizeof(char[MAX_F_NAME + 1]);

        strncpy(readInt, &directory_str[(i*dirSize)+strIndex], sizeof(int));
        DIR[i].size = (int) *readInt;
        strIndex += sizeof(int);

        strncpy(readShort, &directory_str[(i*dirSize)+strIndex], sizeof(unsigned short));
        DIR[i].head = (unsigned short) *readShort;
        strIndex += sizeof(unsigned short);

        strncpy(readIntEight, &directory_str[(i*dirSize)+strIndex], sizeof(int8_t));
        DIR[i].ref_cnt = (int8_t) *readIntEight;
    }

    free(readBool);
    free(readInt);
    free(readShort);
    free(readIntEight);
    free(directory_str);
}

//Writes the FAT to the disk
void writeFAT(unsigned short fat_idx)
{
    //Assmble the string representing the FAT
    int i;
    char* element = (char*) malloc(sizeof(unsigned short));
    char* FAT_str = (char*) malloc(FAT_SIZE*sizeof(unsigned short));
    sprintf(FAT_str, "%hu", FAT[0]);
    for (i = 1; i < FAT_SIZE; i++)
    {
        sprintf(element, "%hu", FAT[i]);
        strncat(FAT_str, element, sizeof(unsigned short));
    }

    //Write the whole string to the block
    block_write(fat_idx, FAT_str);

    //Free temp variables
    free(FAT_str);
    free(element);
}

//Reads the FAT from the disk and updates the global var
void readFAT(unsigned short fat_idx)
{
    //Read the FAT from the disk
    char* FAT_str = (char*) malloc(FAT_SIZE * sizeof(unsigned short));
    block_read(fat_idx, FAT_str);

    //Use strncpy to parse the read string and assigned the values to the superblock
    int i;
    for (i = 0; i < FAT_SIZE; i++)
        FAT[i] = (unsigned short) FAT_str[i*sizeof(unsigned short)];

    free(FAT_str);
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
    writeSuperBlock();

    //Write Empty Directory
    writeDirectory(fs.dir_idx);

    //Write Empty FAT
    writeFAT(fs.fat_idx);

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
    readSuperBlock();

    //Read the directory
    readDirectory(fs.dir_idx);

    //Read the FAT
    readFAT(fs.fat_idx);

    //Close the disk
    error = close_disk();
    if(error == -1)
    {
        return FAILURE;
    }

    return SUCCESS;
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