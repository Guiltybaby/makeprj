/*
 * redesigned by warren zhao
 */

#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include "zshmtmpfs.h"
#include <sys/stat.h>
#include "alog.h"
#include "aos.h"
//#define V_LOG SL_LOGD
#define V_LOG SL_LOGV

//#define SILEAD_DATA_DEVICE				"/data/silead"
//#define ZSHMTMPFS_DEVICE				"/data/silead/tmp"

#define SILEAD_DATA_DEVICE				"/home/jeff_liu/prac/makefiletest/testproject/out/install"
#define ZSHMTMPFS_DEVICE				"/home/jeff_liu/prac/makefiletest/testproject/out/install/tmp"

#define ZSHMTMPFS_MAP_START				4096

typedef struct{
	int    sz;
} zshmtmpfs_handle_t;

/*
 * zshmtmpfs_create_region - creates a new zshmtmpfs region and returns the file
 * descriptor, or <0 on error
 *
 * `name' is an optional label to give the region (visible in /proc/pid/maps)
 * `size' is the size of the region, in page-aligned bytes
 */
typedef struct {
	char tfileStamp[ZSHMTMPFS_NAME_LEN+1+sizeof(int)];
} tmpFileStamp_t;

int zshmtmpfs_get_region(const char *name, int size, int* _flag)
{
	int		fd = -1;
	int		ret = -1;
	char	tfilePath[ZSHMTMPFS_NAME_LEN+1];
	tmpFileStamp_t	tfileStamp;
	struct 	stat buf;
V_LOG("1:size=%d\n",size);
	



	size += sizeof(zshmtmpfs_handle_t);

	if(_flag)
	{
		*_flag = -1;
	}

	DIR *mydir = NULL;
	if( (mydir = opendir(SILEAD_DATA_DEVICE)) == NULL )
	{
        V_LOG("zshmtmpfs_get_region2\n");	

		ret = mkdir( SILEAD_DATA_DEVICE, (S_IRWXU | S_IRWXG | S_IRWXO) );
		if (ret < 0 && ret != -17)
		{
            assert(0);
			return -1;
		}
	}

	if( (mydir = opendir(ZSHMTMPFS_DEVICE)) == NULL )
	{
V_LOG("zshmtmpfs_get_region3\n");

		ret = mkdir( ZSHMTMPFS_DEVICE, (S_IRWXU | S_IRWXG | S_IRWXO) );
		if (ret < 0 && ret != -17)
		{
assert(0);
			return -1;
		}
	}
V_LOG("zshmtmpfs_get_region4\n");	

	//check if the tmpfs is there. 
	snprintf(tfilePath,ZSHMTMPFS_NAME_LEN,"%s/%s",ZSHMTMPFS_DEVICE,"stub.txt");
	stat(tfilePath, &buf);
	V_LOG("stub.txt = %o",buf.st_mode);
	fd = open(tfilePath, O_RDONLY);
	if (fd < 0)
	{
V_LOG("NO EXISTING. MOUNT:tfilePath=%s fd =%d error = %d\n",tfilePath,fd,errno);	
		//not existing, mount tmpfs
		ret = mount("tmpfs", ZSHMTMPFS_DEVICE, "tmpfs", MS_NOSUID, 0);
		if (ret != 0)
		{
//			assert(0);
//			return -1;
		}
        umask(0x0000);  
		close(fd);
		//create stub;
		fd = open(tfilePath, O_RDWR | O_CREAT | O_EXCL, (S_IRWXU | S_IRWXG | S_IRWXO));
		if (fd < 0)
		{
assert(0);
			close(fd);
			remove(tfilePath);
			return -1;
		}
		else
		{
V_LOG("STUBBBBBBBCREATED=%d\n",fd);
		}
	}
	close(fd);

	snprintf(tfilePath,ZSHMTMPFS_NAME_LEN,"%s/%s",ZSHMTMPFS_DEVICE,name);
V_LOG("555:tfilePath=%s\n",tfilePath);	
	fd = open(tfilePath, O_RDWR);
	if (fd >= 0)
	{
V_LOG("555EXISTING KEYFILE\n");	

		//existing one;
		if(_flag)
		{
			*_flag = 0;
		}
		return fd;
	}
	else
	{
V_LOG("555NO KEYFILE:errno=%d,errstr=%s\n",errno,strerror(errno));
		close(fd);
	}

	umask(0x0000);  
	//non-existing, create one;
	fd = open(tfilePath, O_RDWR | O_CREAT | O_EXCL, (S_IRWXU | S_IRWXG | S_IRWXO));
	if (fd < 0)
	{
assert(0);
		close(fd);
		remove(tfilePath);
		return -1;
	}
	else
	{
V_LOG("6:fd=%d\n",fd);

		*((int*)(&(tfileStamp.tfileStamp[0]))) = size;
		memcpy(&(tfileStamp.tfileStamp[sizeof(int)]),tfilePath,ZSHMTMPFS_NAME_LEN+1);
		tfileStamp.tfileStamp[ZSHMTMPFS_NAME_LEN+sizeof(int)] = 0;
		ret = write(fd,tfileStamp.tfileStamp,sizeof(tfileStamp.tfileStamp));
		if( ret != sizeof(tfileStamp.tfileStamp) )
		{
V_LOG("zshmtmpfs_get_region7\n");	

			close(fd);
			remove(tfilePath);
assert(0);
			return -1;
		}
		if( lseek(fd, 0, SEEK_SET) == -1)
		{
                  V_LOG("thomas,fail! zshmtmpfs_get_region7\n");	
			close(fd);
			remove(tfilePath);
			return -1;
		}

		if(_flag)
		{
			*_flag = 1;
		}

		return fd;
	}
V_LOG("zshmtmpfs_get_region9\n");	

	return -1;
}

int zshmtmpfs_destroy_region(
	int			fd
	)
{
	tmpFileStamp_t	tfileStamp;

	int		ret = -1;

	if(fd < 0)
	{
		close(fd);
		return -1;
	}

	if( lseek(fd, 0, SEEK_SET) == -1)
	{
		close(fd);
		return -1;
	}
	ret = read(fd,tfileStamp.tfileStamp,sizeof(tfileStamp.tfileStamp));

	if(ret != sizeof(tfileStamp.tfileStamp))
	{
		close(fd);
assert(0);
		return -1;
	}
	close(fd);
	tfileStamp.tfileStamp[ZSHMTMPFS_NAME_LEN+sizeof(int)] = 0;
	if(remove( &(tfileStamp.tfileStamp[sizeof(int)]) ) != 0)
	{
assert(0);
		return -1;
	}
	return 0;
}

void* zshmtmpfs_attach_region(
	int			fd
	)
{
V_LOG("1:fd=%d\n",fd);
	if(fd < 0)
	{
		return NULL;
	}

	void				*bufp = NULL;
	int					size = 0;
	zshmtmpfs_handle_t* handle;
	int					ret = -1;

	if( lseek(fd, 0, SEEK_SET) == -1)
	{
		return NULL;
	}
	ret = read(fd,&size,sizeof(size));
V_LOG("2:size=%d:ret=%d:sizeof(size)=%lu\n",size,ret,sizeof(size));

	if(ret != sizeof(size))
	{
V_LOG("zshmtmpfs_attach_region3\n");
V_LOG("3:errno=%d,errstr=%s\n",errno,strerror(errno));

		return NULL;
	}

V_LOG("zshmtmpfs_attach_region4\n");
	lseek(fd, (ZSHMTMPFS_MAP_START+size+16), SEEK_SET);
	write(fd, "wz", 3); 

	bufp = mmap(NULL, size, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, ZSHMTMPFS_MAP_START);
V_LOG("zshmtmpfs_attach_region5:bufp=%p\n",bufp);
	if(bufp == (void*)-1)
	{
V_LOG("51:errno=%d,errstr=%s\n",errno,strerror(errno));
		return NULL;
	}

	handle = (zshmtmpfs_handle_t*)bufp;
V_LOG("zshmtmpfs_attach_region6\n");
	handle->sz = size;
V_LOG("zshmtmpfs_attach_region7\n");

	bufp = ((char*)bufp) + sizeof(zshmtmpfs_handle_t);
V_LOG("zshmtmpfs_attach_region8\n");
assert(bufp);
	return bufp;
}

int zshmtmpfs_deattach_region(
	void* _p
	)
{
	void *bufp = ((char*)_p) - sizeof(zshmtmpfs_handle_t);
	zshmtmpfs_handle_t* hanlde = (zshmtmpfs_handle_t*)bufp;
	return munmap(bufp, hanlde->sz);
}

int zshmtmpfs_get_region_name(int fd,char *name,int len)
{
	if(name) 
	{

		tmpFileStamp_t	tfileStamp;

		int		ret = -1;

		if(fd < 0)
		{
			return -1;
		}

		if( lseek(fd, 0, SEEK_SET) == -1)
		{
			return -1;
		}
		ret = read(fd,tfileStamp.tfileStamp,sizeof(tfileStamp.tfileStamp));

		if(ret != sizeof(tfileStamp.tfileStamp))
		{
assert(0);
			return -1;
		}
		tfileStamp.tfileStamp[ZSHMTMPFS_NAME_LEN+sizeof(int)] = 0;
		strncpy(name,&(tfileStamp.tfileStamp[sizeof(int)]),len-1);
		name[len-1] = 0;
	}
	else
	{
assert(0);
		return -1;
	}

	return 0;
}

int zshmtmpfs_get_size_region(int fd)
{
	int ret = -1;

	if(fd < 0)
	{
		return -1;
	}

	int					size = 0;

	if( lseek(fd, 0, SEEK_SET) == -1)
	{
		return -1;
	}
	ret = read(fd,&size,sizeof(size));

	if(ret != sizeof(size))
	{
		return -1;
	}

	return (size - sizeof(zshmtmpfs_handle_t));
}

