/*  ====================================================================================================
**
**  ---------------------------------------------------------------------------------------------------
**
**    File Name:      ashm.c
**    
**    Description:    This file contains the implementation of OS wrapper.
**
**                    this is kernal code of SW framework.
**                    It contributes one of functionalities of SW Platform. 
**                    If the checkin is CR not PR, to add change History to this file head part 
**                    will be appreciated.
**
**  ---------------------------------------------------------------------------------------------------
**
**  Author:			Warren Zhao
**
** -------------------------------------------------------------------------
**
**    Change History:
**    
**    Initial revision
**
**====================================================================================================*/

#include "ashm.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <sys/mman.h>
#ifdef ANDROID_BUILDER
#include <linux/shm.h>
#endif
#ifdef ARMLINUX_BUILDER
#include <sys/shm.h>
#endif

#include "aheap.h"

#define VSHM_LOG(f,...)	printf("pid-%d:%-15s:%04d => "f, getpid(),__FUNCTION__, __LINE__,##__VA_ARGS__); //printf(f,##__VA_ARGS__);
//#define VSHM_LOG

#define _ASHM_TMPFS_MEM_
//#define _ASHM_ZSHMEM_
//#define _ASHM_ARMLINUX_SHMEM_

#define SHAREDMEM_KEY_PREFIX			"zshmem_key_"

#ifdef _ASHM_TMPFS_MEM_
#include "zshmtmpfs.h"
#endif
#ifdef _ASHM_ZSHMEM_
#include "zshmapi.h"
#endif


/** ashm_create
 * @brief  to create a shared memory.
 * @exception NULL
 * @param     [IN] _key is the key to create shared memory 
 * @param     [IN] _sz  shared memory size
 * @param     [IN/OUT] _flag Judge if the share memory is newly constructed.
 * @return    the newly-created shared memory.
 */
ashmid_t ashm_create(
	akey_t _key, 
	int    _sz, 
	int*   _flag
	)
{
#ifdef _ASHM_TMPFS_MEM_
	char   regionName[256+1];
	snprintf(regionName,256,"%s%d",SHAREDMEM_KEY_PREFIX,_key);
VSHM_LOG("ashm_create1\n");
	return zshmtmpfs_get_region(regionName, _sz, _flag);
#endif //_ASHM_TMPFS_MEM_

#ifdef _ASHM_ZSHMEM_
	int    shmfd = -1;
	char   regionName[256+1];
	snprintf(regionName,256,"%s%d",SHAREDMEM_KEY_PREFIX,_key);
VSHM_LOG("zshmem:regionName=%s\n",regionName);

	shmfd = zshmem_get_region_handle(regionName);
VSHM_LOG("zshmem:shmfd=%d\n",shmfd);
	if(shmfd < 0)
	{
VSHM_LOG("zshmem2\n");
		shmfd = zshmem_create_region(
			regionName, 
			_sz
			);
VSHM_LOG("zshmem3:shmfd=%d\n",shmfd);
		if(_flag)
		{
			*_flag = 1;
		}
	}
	else
	{
		if(_flag)
		{
			*_flag = 0;
		}
	}
	
	return shmfd;

#endif

#ifdef _ASHM_ARMLINUX_SHMEM_
	VSHM_LOG("enter...key[%d],sz[%d]\n", _key, _sz);
	
	ashmid_t          res;
	struct shmid_ds shmbuffer;
	
	if (_key == 0 || _key == IPC_PRIVATE)
	{
		res = shmget(IPC_PRIVATE, _sz, IPC_CREAT | IPC_EXCL | 0600);
		if (-1 == res)
		{
			VSHM_LOG("Error: errno[%d]\n", errno);
			goto EXIT;
		}
		if (_flag != 0)
		{
			*_flag = 1;
		}
	}
	else
	{
		res = shmget(_key, _sz, IPC_CREAT | IPC_EXCL | 0600);
		if (-1 == res)
		{
			/// if(ENOENT != errno)
			/// {
			///     VSHM_LOG("Error: errno[%d]\n", errno);
			///     goto EXIT;
			/// }
			
			res = shmget(_key, _sz, IPC_CREAT | 0600);
			
			if (-1 == res )
			{
				VSHM_LOG("Error: errno[%d]\n", errno);
				goto EXIT;
			}
			
			if (_flag != 0) 
			{
				*_flag = 0;
			}
		}
		else
		{
			if (_flag != 0) 
			{
				*_flag = 1;
			}
		}
	}
	
EXIT:
	VSHM_LOG("leave...\n");
	return res;
#endif 
}

/** ashm_exist
 * @brief  to unregister an access assigned to interrupt no in virtual soft interrupt vectors table.
 * @exception NULL
 * @param [IN] _id is the assigned interrupt no.
 * @return true or false
 */
Boolean ashm_exist(akey_t _key)
{
#ifdef _ASHM_TMPFS_MEM_
	int    shmfd = -1;
	char   regionName[256+1];
	snprintf(regionName,256,"%s%d",SHAREDMEM_KEY_PREFIX,_key);
	shmfd = zshmtmpfs_get_region(regionName, 4,	0);
	if(shmfd >= 0)
	{
		return ATrue;
	}
	return AFalse;
#endif //_ASHM_TMPFS_MEM_

#ifdef _ASHM_ZSHMEM_
	int    shmfd = -1;
	char   regionName[256+1];
	snprintf(regionName,256,"%s%d",SHAREDMEM_KEY_PREFIX,_key);

	shmfd = zshmem_get_region_handle(regionName);
	if(shmfd >= 0)
	{
		return ATrue;
	}
	return AFalse;

#endif
#ifdef _ASHM_ARMLINUX_SHMEM_
	Boolean bRes = ATrue;
	int id = shmget(_key, 0, IPC_EXCL);
	if (id == -1)
	{
		bRes = AFalse;
	}
	return bRes;
#endif 
}

/** ashm_destroy
 * @brief  to Destroys all resources associated with the passed share memory.
 * @exception NULL
 * @param [IN] _h is the shared memory. 
 * @return NULL
 */
void ashm_destroy(ashmid_t _h)
{
#ifdef _ASHM_TMPFS_MEM_
	zshmtmpfs_destroy_region(_h);
#endif //_ASHM_TMPFS_MEM_

#ifdef _ASHM_ZSHMEM_

	VSHM_LOG("enter...\n");
	
	if (_h >= 0)
	{
		zshmem_destroy_region(_h);
	}
	
	VSHM_LOG("leave...\n");

#endif

#ifdef _ASHM_ARMLINUX_SHMEM_
	VSHM_LOG("enter...\n");
	
	if (_h > 0)
	{
		shmctl(_h, IPC_RMID, 0);
	}
	
	VSHM_LOG("leave...\n");
#endif 
}

/** ashm_attach
 * @brief  to get the address to share memory you created or got.
 * @exception NULL
 * @param [IN] _h is the shared memory. 
 * @return the addr.
 */
void* ashm_attach(ashmid_t _h)
{
#ifdef _ASHM_TMPFS_MEM_
VSHM_LOG("ashm_attach1\n");
	return zshmtmpfs_attach_region(_h);
#endif //_ASHM_TMPFS_MEM_

#ifdef _ASHM_ZSHMEM_

	return zshmem_attach_region(_h);

#endif

#ifdef _ASHM_ARMLINUX_SHMEM_
	VSHM_LOG("enter...\n");
	void* res = 0;
	
	if (_h >= 0)
	{
		res = shmat(_h, NULL, 0);
		if (-1 == (int)res)
		{
			VSHM_LOG("Error: errno[%d]\n", errno);
			res = 0;
		}
	}
	
	VSHM_LOG("leave...\n");
	return res;
#endif 
}

int ashm_deattach(void* _p)
{
#ifdef _ASHM_TMPFS_MEM_
	return zshmtmpfs_deattach_region(_p);
#endif //_ASHM_TMPFS_MEM_

#ifdef _ASHM_ZSHMEM_
	return zshmem_deattach_region(_p);
#endif

#ifdef _ASHM_ARMLINUX_SHMEM_
	VSHM_LOG("enter...\n");
	int res = 0;
	
	if (_p != NULL)
	{
		res = shmdt(_p);
		if (-1 == (int)res)
		{
			VSHM_LOG("Error: errno[%d]\n", errno);
		}
	}
	
	VSHM_LOG("leave...\n");
	return res;
#endif 
}

