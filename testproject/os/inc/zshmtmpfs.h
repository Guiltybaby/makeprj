/*
 * redesigned by warren zhao
 */

#ifndef __AOS_ZSHM_TMPFS_H__
#define __AOS_ZSHM_TMPFS_H__

#define ZSHMTMPFS_NAME_LEN				255

int zshmtmpfs_get_region(const char *name, int size, int* _flag);

int zshmtmpfs_destroy_region(int fd);

void* zshmtmpfs_attach_region(int fd);

int zshmtmpfs_deattach_region(void* _p);

int zshmtmpfs_get_region_name(int fd,char *name,int len);

int zshmtmpfs_get_size_region(int fd);

#endif