//Devin Bidstrup U19115995
//EC440 Operating Systems 
#include "fs.h"
#include "disk.h"

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