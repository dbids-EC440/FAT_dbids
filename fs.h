#ifndef _FS_H_
#define _FS_H_

/******************************************************************************/
#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define DISK_BLOCKS  8192      /* number of blocks on the disk                */
#define BLOCK_SIZE   4096      /* block size on "disk"                        */
#define MAX_F_NAME   15        /* maximum size of file name                   */
#define MAX_F_NUM    64        /* maximum number of files                     */
#define MAX_FILDES   32        /* maximum number of file descriptors          */
#define FAT_SIZE     8250     /* maximum number of entries in FAT             */
#define SUCCESS      0
#define FAILURE      -1

//Super Block: 10 bytes
//DIR :        1792 bytes
//DIR + Super Block --> 1802 bytes or 1 block
//FAT : 5 blocks in meta --> 8192 blocks addressable but only 8186 remain + 64 eof = 8250
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
