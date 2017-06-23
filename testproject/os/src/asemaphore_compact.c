/*  ====================================================================================================
**
**  ---------------------------------------------------------------------------------------------------
**
**    File Name:      asemaphore.c
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

#include <sys/types.h>
#include <sys/ipc.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#ifdef ANDROID_BUILDER
#include <linux/sem.h>
#endif
#ifdef ARMLINUX_BUILDER
#include <sys/sem.h>
#endif

#include "asemaphore.h"
#include "aheap.h"
#include "ashm.h"
#include "aosmgr.h"
#include "alog.h"

#define SEMA_LOG A_LOGD 
//#define SEMA_LOG

static ASemaHandleData_t*	g_pSemaHead = NULL;

static void asema_add_local_sema(
	asema_t _sema
	)
{
	ASemaHandleData_t* t_currSema = g_pSemaHead;
	if(!t_currSema)
	{
		t_currSema = _sema;
		t_currSema->pNext = NULL;
		return;
	}
	while(t_currSema->pNext)
	{
		t_currSema = t_currSema->pNext;
	}
	t_currSema->pNext = _sema;
	((ASemaHandleData_t*)_sema)->pNext = NULL;
}

static int asema_remove_local_sema(
	asema_t _sema
	)
{
	ASemaHandleData_t* t_currSema = g_pSemaHead;
	if(!_sema || !t_currSema)
	{
		return -1;
	}

	if(t_currSema == _sema)
	{
		g_pSemaHead = t_currSema->pNext;
		((ASemaHandleData_t*)_sema)->pNext = NULL;
		return 0;
	}
	while(t_currSema->pNext)
	{
		if(t_currSema->pNext == _sema)
		{
			t_currSema->pNext = ((ASemaHandleData_t*)_sema)->pNext;
			((ASemaHandleData_t*)_sema)->pNext = NULL;
			return 0;
		}
		t_currSema = t_currSema->pNext;
	}

	return -1;
}

asema_t asema_find_local_sema(
	char* _sema_name
	)
{
	if(!_sema_name)
	{
		return NULL;
	}
	if(_sema_name[0] == 0)
	{
		return NULL;
	}

	ASemaHandleData_t* t_currSema = g_pSemaHead;
	while(1)
	{
		if(t_currSema)
		{
			if(strcmp(t_currSema->name,_sema_name) == 0)
			{
				return t_currSema;
			}
		}
		else
		{
			break;
		}
		t_currSema = t_currSema->pNext;
	}

	return NULL;
}

static int __is_thisproc_obtainer(
	ASemaHandleData_t* sema
	)
{
	pid_t t_pid = getpid();
	int i;
	for(i = 0; i < SEMA_MAX_MUTEX_VAL; i++)
	{
		if(sema->obtainer_list[i] == t_pid)
		{
			return 1;
		}
	}
	return 0;
}

int asema_get_returnsema_parents(
	asema_t  _sema, 
	char*    _relayer_name,
	int      _max_relayer_name_sz
	)
{
	ASemaHandleData_t* sema = (ASemaHandleData_t*)(_sema);
	char*	t_str;
	int		t_len;

//SEMA_LOG("sema->name=%s\n",sema->name);
	t_str = "remoteapi.sema_return@";
	t_len = strlen(t_str);
	//return sema
	if(strncmp(sema->name,t_str,t_len) == 0)
	{
		char   t_relayer_name[OSTAGName_Len+1];
		pid_t  t_client_pid;
		int    i;

		for(i=0;(sema->name[t_len+i] != '@' && sema->name[t_len+i] != 0);i++)
		{
			t_relayer_name[i] = sema->name[t_len+i];
		}
		t_relayer_name[i] = 0;
//SEMA_LOG("t_relayer_name=%s\n",t_relayer_name);

		t_client_pid = aosmgr_str2int( (char*)(&(sema->name[t_len+i+1])) );
//SEMA_LOG("t_client_pid=%d,(char*)(&(sema->name[t_len+i+1]))=%s\n",t_client_pid,(char*)(&(sema->name[t_len+i+1])));

		assert(t_client_pid >= 0);
		if(_relayer_name)
		{
			strncpy(_relayer_name,t_relayer_name,_max_relayer_name_sz);
			_relayer_name[_max_relayer_name_sz-1] = 0;
		}
//SEMA_LOG("_relayer_name=%s\n",_relayer_name);
		return t_client_pid;
	}
	else
	{
		if(_relayer_name)
		{
			_relayer_name[0] = 0;
		}
		return -1;
	}
}

static int __asema_proccrash_deadlock(
	aosmgr_t		_h,
	aosmgr_tag_t*	in_tag
	)
{
	pid_t t_pid = getpid();
SEMA_LOG("1in_tag->name=%s-pid=%d\n",in_tag->name,getpid());
	ASemaHandleData_t* semaphore = (ASemaHandleData_t*)(&(in_tag->handle));
SEMA_LOG("2in_tag->name=%s-semaphore->mutexed=%d-pid=%d\n",in_tag->name,semaphore->mutexed,getpid());
	if(semaphore->mutexed)
	{
SEMA_LOG("3in_tag->name=%s-semaphore->mutexed=%d-pid=%d\n",in_tag->name,semaphore->mutexed,getpid());
int j;
for(j = 0; j < SEMA_MAX_MUTEX_VAL/16; j++)
{
	printf("obtainers: %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d ",
		semaphore->obtainer_list[0],semaphore->obtainer_list[1],semaphore->obtainer_list[2],semaphore->obtainer_list[3],
		semaphore->obtainer_list[4],semaphore->obtainer_list[5],semaphore->obtainer_list[6],semaphore->obtainer_list[7],
		semaphore->obtainer_list[8],semaphore->obtainer_list[9],semaphore->obtainer_list[10],semaphore->obtainer_list[11],
		semaphore->obtainer_list[12],semaphore->obtainer_list[13],semaphore->obtainer_list[14],semaphore->obtainer_list[15]
		);
}
printf("\n\n");

		int i;
		for(i = 0; i < SEMA_MAX_MUTEX_VAL; i++)
		{
			if(semaphore->obtainer_list[i] == t_pid)
			{
SEMA_LOG("1release it:semaphore->name=%s-pid=%d\n",semaphore->name,getpid());
				asema_release(semaphore,1);
				//break;
			}
		}
	}
	else
	{
		char*	t_str;
		int		t_len;
		t_str = "remoteapi.sema_return@";
		t_len = strlen(t_str);
		if(strncmp(semaphore->name,t_str,t_len) == 0)
		{
			char   t_relayer_name[OSTAGName_Len+1];
			pid_t  t_client_pid;

			t_client_pid = asema_get_returnsema_parents(
				semaphore,
				t_relayer_name,
				OSTAGName_Len
				);

			if(aosmgr_str2int( t_relayer_name ) == t_pid)
			{
				if(semaphore->waiting_num > 0)
				{
SEMA_LOG("2release it:semaphore->name=%s-pid=%d\n",semaphore->name,getpid());
					asema_release(semaphore,1);
				}
			}
		}

		int i;
		for(i = 0; i < SEMA_MAX_WAITING_LOG_NUM; i++)
		{
			if(semaphore->waiting_list[i] == t_pid)
			{
				semaphore->waiting_list[i] = -1;
				if(semaphore->waiting_num > 0)
				{
					semaphore->waiting_num--;
				}
			}
		}
	}

	return 0;
}

static int __asema_gc(
	aosmgr_t		_h,
	aosmgr_tag_t*	in_tag
	)
{
	SEMA_LOG("enter pid=%d\n",getpid());
	ASemaHandleData_t* sema = (ASemaHandleData_t*)(&(in_tag->handle));
	char*	t_str;
	int		t_len;
	int     ret = -1;

	t_str = "remoteapi.sema_return@";
	t_len = strlen(t_str);
	//return sema
	if(strncmp(sema->name,t_str,t_len) == 0)
	{
		char   t_relayer_name[OSTAGName_Len+1];
		pid_t  t_client_pid;

		t_client_pid = asema_get_returnsema_parents(
			sema, 
			t_relayer_name,
			OSTAGName_Len
			);

		if(aosmgr_IsProcessDead(t_client_pid))
		{
			asema_release(sema,1);
			asema_destroy(sema);
			ret = 0;
		}
		else
		{
			if(aosmgr_str2int( t_relayer_name ) != -1)
			{
				if(aosmgr_IsProcessDead(aosmgr_str2int(t_relayer_name)))
				{
					asema_release(sema,1);
					asema_destroy(sema);
					ret = 0;
				}
			}
		}
	}
	else
	{
		t_str = "remoteapi.sema_call@";
		t_len = strlen(t_str);
		//call sema
		if(strncmp(sema->name,t_str,t_len) == 0)
		{
			if(aosmgr_IsProcessDead(in_tag->pid))
			{
				if(aosmgr_str2int( (char*)(&(sema->name[t_len])) ) != -1)
				{
					asema_release(sema,1);
					asema_destroy(sema);
					ret = 0;
				}
			}
		}
		else
		{
//			if(aosmgr_IsProcessDead(mgr->tag[i].pid))
//			{
//				asema_release(sema,1);
//				asema_destroy(sema);
//			}
		}
	}
	SEMA_LOG("leave pid=%d\n",getpid());
	return ret;
}

/** asema_create
 * @brief  Creates a new semaphore or obtains the semaphore identifier of an existing semaphore.
 * @param  [IN] _name is semaphoreset name.
 * @param  [IN] _val  is the init val of the semaphore.
 * @exception NULL
 * @return the newly-created semaphore identifier.
 */
asema_t __asema_create(
	char*          _name, 
	unsigned short _val, 
	unsigned short _maxval, 
	int            _is_shared,
	int            _is_mutex
	)
{
	SEMA_LOG("enter...\n");

	aosmgr_gc_register(
		AOSTAGTYPE_SEMA,
		__asema_gc
		);
//SEMA_LOG("1...\n");
//	aosmgr_release_deadlock_sema_register(
//		__asema_proccrash_deadlock
//		);

	ASemaHandleData_t* semaphore = NULL;

	if(_is_shared)
	{
SEMA_LOG("2...\n");
		aosmgr_t os = aosmgr_get();
        assert(os);
		semaphore = aosmgr_op_gethandle(os,_name,0);
SEMA_LOG("3...\n");

		if(semaphore)
		{
			//
			SEMA_LOG("repeat creating or duplicated name with other semaphore\n");
SEMA_LOG("4...\n");
			semaphore = NULL;
			assert(0);
		}
		else
		{
			aosmgr_op_reg(os, _name, 0, AOSTAGTYPE_SEMA, 0);
			semaphore = (ASemaHandleData_t*)aosmgr_op_gethandle(os,_name,0);
SEMA_LOG("5...\n");
			assert(semaphore);
			semaphore->shared = 1;
		}
	}
	else
	{
SEMA_LOG("6...\n");
		semaphore = AHEAP_Alloc(sizeof(ASemaHandleData_t));
SEMA_LOG("7...\n");
		assert(semaphore);
		semaphore->shared = 0;
SEMA_LOG("8...\n");
		asema_add_local_sema(semaphore);
SEMA_LOG("9...\n");
	}

	if(semaphore->shared)
	{
		pthread_mutexattr_t mattr;
		assert(0 == pthread_mutexattr_init(&mattr));
		assert(0 == pthread_mutexattr_setpshared(&mattr,PTHREAD_PROCESS_SHARED));
		assert(0 == pthread_mutex_init(&(semaphore->thread_flag_mutex), &mattr));

		pthread_condattr_t c_mattr;
		assert(0 == pthread_condattr_init(&c_mattr));
		assert(0 == pthread_condattr_setpshared(&c_mattr,PTHREAD_PROCESS_SHARED));
		assert(0 == pthread_cond_init(&(semaphore->thread_flag_cv), &c_mattr));
	}
	else
	{
		assert(0 == pthread_mutex_init(&(semaphore->thread_flag_mutex), NULL));
		assert(0 == pthread_cond_init(&(semaphore->thread_flag_cv), NULL));
	}
	semaphore->mutexed = _is_mutex;
SEMA_LOG("10...\n");
	if(_is_mutex)
	{
		//these 2 can be removed; for simplized debugging
		assert(_val == 1);
		assert(_maxval == 1);

		assert(_maxval <= SEMA_MAX_MUTEX_VAL);
	}
	semaphore->thread_flag = _val;
	//assert(_maxval <= SEMA_MAX_MUTEX_VAL);
	semaphore->maxval = _maxval;
	assert(semaphore->thread_flag <= semaphore->maxval);
	strncpy(semaphore->name,_name,OSTAGName_Len);
	semaphore->name[OSTAGName_Len] = 0;
SEMA_LOG("11...\n");

	semaphore->waiting_num = 0;
	int i;
	for(i = 0; i < SEMA_MAX_WAITING_LOG_NUM; i++)
	{
		semaphore->waiting_list[i] = -1;
	}
SEMA_LOG("12...\n");
	semaphore->obtainer_num = 0;
	for(i = 0; i < SEMA_MAX_MUTEX_VAL; i++)
	{
		semaphore->obtainer_list[i] = -1;
	}

	semaphore->stamp_releasing = 0;
	semaphore->stamp = 0xbabeface;
	SEMA_LOG("leave...\n");
	return semaphore;
}

asema_t asema_get_handle_ipc(
	char*          _name
	)
{
	SEMA_LOG("enter...\n");

	ASemaHandleData_t* semaphore = NULL;

	aosmgr_t os = aosmgr_get();
    assert(os);
	semaphore = aosmgr_op_gethandle(os,_name,0);

	if(semaphore && (semaphore->stamp != 0xbabeface))
	{
		semaphore = NULL;
	}

	SEMA_LOG("leave...\n");
	return semaphore;
}

/** asema_destroy
 * @brief  Delete the specified semaphore, the times the semaphore referenced reduce 1 per launching until the count equal 0, then destroy the semaphore set.
 * @patam  [IN] _s the specified semaphore.
 * @exception NULL
 * @return    NULL
 */
void asema_destroy(
	asema_t _s
	)
{
	SEMA_LOG("enter...\n");
	int i = 0;
	ASemaHandleData_t* sem = (ASemaHandleData_t*)_s;
	assert(sem);

	int curr_waits = sem->waiting_num;
	while(sem->waiting_num > 0)
	{
		i = 0;
		curr_waits = sem->waiting_num;
		asema_release(sem,1);
		while(curr_waits == sem->waiting_num)
		{
			usleep(1000*30);
			i++;
			if(i > 10)
			{
				if(sem->waiting_num)
				{
					sem->waiting_num--;
				}
				break;
			}
		}
	}
	usleep(100*1000);
SEMA_LOG("usleepend...\n");

	i = 0;
	while(sem->stamp_releasing > 0)
	{
		if(i++ > 200)
		{
			break;
		}
		usleep(1000*10);
	}

	sem->stamp = 0;
	usleep(100*1000);

//	assert(0 == pthread_mutex_destroy(&(sem->thread_flag_mutex)));
//	assert(0 == pthread_cond_destroy(&(sem->thread_flag_cv)));
	pthread_mutex_destroy(&(sem->thread_flag_mutex));
	pthread_cond_destroy(&(sem->thread_flag_cv));

	if(sem->shared)
	{
		aosmgr_tag_t* ptag = handle_to_osmgr_tag(sem);
		aosmgr_t os = aosmgr_get();
		assert(os);
SEMA_LOG("sem->thread_flag=%d-sem->name=%s-ptag->name=%s-ptag->id=%d-pid=%d\n",
		 sem->thread_flag,sem->name,ptag->name,ptag->id,getpid());
		aosmgr_op_unreg(
			os, 
			ptag->name, 
			ptag->id
			);
	}
	else
	{
		asema_remove_local_sema(sem);
		AHEAP_Delete(sem);
	}
	sem = NULL;

	SEMA_LOG("leave...\n");
}

/** asemaset_acquire
 * @brief  wait to obtain a semaphore
 * @param  [IN] _s the specified semaphore.
 * @exception NULL
 * @return    A return value of OSSTATUSFailure represents the <asema_acquire> operation failed, OSSTATUSSuccess sucessful.
 */
aosstatus_t asema_acquire(
	asema_t _s
	)
{
	return asema_acquire_timed(_s, TICKS_FOREVER);
}

/** asemaset_acquire_timed
 * @brief  wait to obtain a semaphore
 * @param  [IN] _s the specified semaphore.
 * @param  [IN] _ms    timeout to acquire semaphore (millisecond).
 * @exception NULL
 * @return    A return value of OSSTATUSFailure represents the <asema_acquire> operation failed, OSSTATUSSuccess sucessful.
 */
aosstatus_t asema_acquire_timed(
	asema_t       _s, 
	unsigned long _ms
	)
{
	SEMA_LOG("enter...\n");
	int		i;
	pid_t	t_pid;

	aosstatus_t result = OSSTATUSSuccess;
	int cond_var_wait_res = 0;
	ASemaHandleData_t* sem = (ASemaHandleData_t*)_s;
	assert(sem);

	if(sem->stamp != 0xbabeface)
	{
		return OSSTATUSFailure;
	}

	//obtain
	int timeout = 0;
	int mutex_res = 0;
	aosmgr_sig_maskall();
SEMA_LOG("%d-%s-sem->thread_flag_mutex_lock-ENTER-pid=%d\n",sem->thread_flag,sem->name,getpid());
	if(1)
	{
CONTINUE:
		mutex_res = pthread_mutex_trylock(&(sem->thread_flag_mutex));
		if(mutex_res != 0)
		{
			usleep(100);
			timeout++;
			if(timeout < 10*1000*10)
			{
				goto CONTINUE;
			}
SEMA_LOG("%d-%s-sem->thread_flag_mutex_lock-timeout-pid=%d\n",sem->thread_flag,sem->name,getpid());
		}
	}
	else
	{
		pthread_mutex_lock(&(sem->thread_flag_mutex));
	}
SEMA_LOG("%d-%s-sem->thread_flag_mutex_lock-EXIT-pid=%d\n",sem->thread_flag,sem->name,getpid());

	sem->waiting_num++;
	for(i = 0; i < SEMA_MAX_WAITING_LOG_NUM; i++)
	{
		//TODO cleaning dead waiters.
		//
		if(sem->waiting_list[i] == -1)
		{
			sem->waiting_list[i] = getpid();
			break;
		}
	}

	assert(sem->thread_flag >= 0);
	while( sem->thread_flag == 0 )
	{
SEMA_LOG("%d-%s-sem->1 22222-pid=%d\n",sem->thread_flag,sem->name,getpid());

		if(_ms == TICKS_FOREVER)
		{
SEMA_LOG("%d-%s-sem->thread_flag_cv-WAIT-ENTTER-pid=%d\n",sem->thread_flag,sem->name,getpid());
 			aosmgr_sig_unmaskall();
			pthread_cond_wait(&(sem->thread_flag_cv), &(sem->thread_flag_mutex));
			aosmgr_sig_maskall();
SEMA_LOG("%d-%s-sem->thread_flag_cv-WAIT-EXIT-pid=%d\n",sem->thread_flag,sem->name,getpid());
		}
		else
		{
			struct timeval now;
			struct timespec abstime;
			gettimeofday(&now, NULL);
			abstime.tv_sec  = now.tv_sec + _ms / 1000;
			abstime.tv_nsec = now.tv_usec * 1000 + (_ms % 1000) * 1000000;

SEMA_LOG("%d-%s-sem->thread_flag_cv-timed-WAIT-ENTTER-pid=%d\n",sem->thread_flag,sem->name,getpid());
			aosmgr_sig_unmaskall();
			cond_var_wait_res = pthread_cond_timedwait(&(sem->thread_flag_cv), &(sem->thread_flag_mutex),&abstime);
			aosmgr_sig_maskall();
SEMA_LOG("%d-%s-sem->thread_flag_cv-timed-WAIT-EXIT-pid=%d\n",sem->thread_flag,sem->name,getpid());

			if(cond_var_wait_res == ETIMEDOUT)
			{
SEMA_LOG("%d-%s-sem->2-pid=%d\n",sem->thread_flag,sem->name,getpid());

				result = OSSTATUSTimeout;
				goto EXIT;
			}
		}
	}

SEMA_LOG("%d-%s-sem->3-pid=%d\n",sem->thread_flag,sem->name,getpid());

	if(sem->thread_flag > 0)
	{
SEMA_LOG("%d-%s-sem->4-pid=%d\n",sem->thread_flag,sem->name,getpid());

		sem->thread_flag--;
	}

	if(sem->mutexed)
	{
		for(i = 0; i < SEMA_MAX_MUTEX_VAL; i++)
		{
			if(sem->obtainer_list[i] == -1)
			{
				sem->obtainer_list[i] = getpid();
				sem->obtainer_num++;
SEMA_LOG("%d-%s-sem->5-i=%d-sem->obtainer_num=%d-pid=%d\n",sem->thread_flag,sem->name,i,sem->obtainer_num,getpid());
				break;
			}
		}
SEMA_LOG("\n%d-%s-sem->6-pid=%d\n",sem->thread_flag,sem->name,getpid());
int j;
for(j = 0; j < SEMA_MAX_MUTEX_VAL/16; j++)
{
	printf("obtainers: %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d ",
		sem->obtainer_list[0],sem->obtainer_list[1],sem->obtainer_list[2],sem->obtainer_list[3],
		sem->obtainer_list[4],sem->obtainer_list[5],sem->obtainer_list[6],sem->obtainer_list[7],
		sem->obtainer_list[8],sem->obtainer_list[9],sem->obtainer_list[10],sem->obtainer_list[11],
		sem->obtainer_list[12],sem->obtainer_list[13],sem->obtainer_list[14],sem->obtainer_list[15]
		);
}
printf("\n\n");
		if(i == SEMA_MAX_MUTEX_VAL)
		{
			assert(0);
		}
	}

EXIT:

	t_pid = getpid();
	for(i = 0; i < SEMA_MAX_WAITING_LOG_NUM; i++)
	{
		if(sem->waiting_list[i] == t_pid)
		{
			sem->waiting_list[i] = -1;
			break;
		}
	}
	if(sem->waiting_num > 0)
	{
		sem->waiting_num--;
	}

SEMA_LOG("%d-%s-sem->thread_flag_mutex_unlock-ENTER-pid=%d\n",sem->thread_flag,sem->name,getpid());
	pthread_mutex_unlock (&(sem->thread_flag_mutex));
SEMA_LOG("%d-%s-sem->thread_flag_mutex_unlock-EXIT-pid=%d\n",sem->thread_flag,sem->name,getpid());
	aosmgr_sig_unmaskall();

	SEMA_LOG("leave...\n");
	return result;
}

/** asemaset_release
 * @brief  Release a semaphore
 * @param  [IN] _s the specified semaphore.
 * @param  [IN] _which which semaphore of the semaphore is operated.
 * @exception NULL
 * @return    A return value of OSSTATUSFailure represents the <asemaset_acquire> operation failed, OSSTATUSSuccess sucessful.
 */
aosstatus_t asema_release(
	asema_t _s,
	int     _is_crash_handler
	)
{
	SEMA_LOG("enter...\n");

	aosstatus_t result = OSSTATUSSuccess;
	ASemaHandleData_t* sem = (ASemaHandleData_t*)_s;
	assert(sem);

	if(sem->stamp != 0xbabeface)
	{
		return OSSTATUSFailure;
	}

	//release
	int timeout = 0;
	int mutex_res = 0;
	aosmgr_sig_maskall();
SEMA_LOG("%d-%s-sem->thread_flag_mutex_lock-ENTER-pid=%d\n",sem->thread_flag,sem->name,getpid());
	if(_is_crash_handler)
	{
CONTINUE:
		mutex_res = pthread_mutex_trylock(&(sem->thread_flag_mutex));
		if(mutex_res != 0)
		{
			usleep(100);
			timeout++;
			if(timeout < 10*1000*10)
			{
				goto CONTINUE;
			}
SEMA_LOG("%d-%s-sem->thread_flag_mutex_lock-timeout-pid=%d\n",sem->thread_flag,sem->name,getpid());
		}
	}
	else
	{
		pthread_mutex_lock(&(sem->thread_flag_mutex));
	}
SEMA_LOG("%d-%s-sem->thread_flag_mutex_lock-EXIT-pid=%d\n",sem->thread_flag,sem->name,getpid());
	sem->stamp_releasing++;

	if(sem->thread_flag < sem->maxval)
	{
SEMA_LOG("%d-%s-sem->1-pid=%d\n",sem->thread_flag,sem->name,getpid());

		if(sem->mutexed)
		{
			int i;
			pid_t t_pid = getpid();
			for(i = 0; i < SEMA_MAX_MUTEX_VAL; i++)
			{
				if(sem->obtainer_list[i] == t_pid)
				{
					sem->obtainer_list[i] = -1;
					if(sem->obtainer_num > 0)
					{
						sem->obtainer_num--;
					}
SEMA_LOG("%d-%s-sem->11-i=%d-sem->obtainer_num=%d-pid=%d\n",sem->thread_flag,sem->name,i,sem->obtainer_num,getpid());
					break;
				}
			}
SEMA_LOG("%d-%s-sem->12-pid=%d\n",sem->thread_flag,sem->name,getpid());
int j;
for(j = 0; j < SEMA_MAX_MUTEX_VAL/16; j++)
{
	printf("obtainers: %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d ",
		sem->obtainer_list[0],sem->obtainer_list[1],sem->obtainer_list[2],sem->obtainer_list[3],
		sem->obtainer_list[4],sem->obtainer_list[5],sem->obtainer_list[6],sem->obtainer_list[7],
		sem->obtainer_list[8],sem->obtainer_list[9],sem->obtainer_list[10],sem->obtainer_list[11],
		sem->obtainer_list[12],sem->obtainer_list[13],sem->obtainer_list[14],sem->obtainer_list[15]
		);
}
printf("\n\n");
		}

		if(sem->thread_flag == 0)
		{
			sem->thread_flag++;
SEMA_LOG("%d-%s-sem->thread_flag_cv-SIGNAL-ENTTER-pid=%d\n",sem->thread_flag,sem->name,getpid());
			pthread_cond_signal(&(sem->thread_flag_cv));
SEMA_LOG("%d-%s-sem->thread_flag_cv-SIGNAL-EXIT-pid=%d\n",sem->thread_flag,sem->name,getpid());
		}
		else
		{
			sem->thread_flag++;
		}
	}

	if(sem->stamp_releasing > 0)
	{
		sem->stamp_releasing--;
	}
	/* Unlock the mutex. */
SEMA_LOG("%d-%s-sem->thread_flag_mutex_unlock-ENTER-pid=%d\n",sem->thread_flag,sem->name,getpid());
	pthread_mutex_unlock (&(sem->thread_flag_mutex));
SEMA_LOG("%d-%s-sem->thread_flag_mutex_unlock-EXIT-pid=%d\n",sem->thread_flag,sem->name,getpid());
	aosmgr_sig_unmaskall();

	SEMA_LOG("leave...\n");
	return result;
}

int asema_value(
	asema_t _s
	)
{
	ASemaHandleData_t* sem = (ASemaHandleData_t*)_s;
	assert(sem);

	return sem->thread_flag;
}
