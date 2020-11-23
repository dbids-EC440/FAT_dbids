#ifndef _FS_H_
#define _FS_H_

/******************************************************************************/
#include <sys/types.h>

#define DISK_BLOCKS  8192      /* number of blocks on the disk                */
#define BLOCK_SIZE   4096      /* block size on "disk"                        */
#define SUCCESS 0
#define FAILURE -1
/******************************************************************************/
int make_fs(char* disk_name);
int mount_fs(char* disk_name);
int umount_fs(char* disk_name);
int fs_open(char* name);
int fs_close(int fildes);
int fs_create(char* name);
int fs_delete(char* name);
int fs_read(int fildes, void* buf, size_t nbyte);
int fs_write(int fildes, void* buf, size_t nbyte);
int fs_get_filesize(int fildes);
int fs_listfiles(char*** files);
int fs_lseek(int fildes, off_t offset);
int fs_truncate(int fildes, off_t length);
/******************************************************************************/

#endif
