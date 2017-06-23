/*  ====================================================================================================
**
**  ---------------------------------------------------------------------------------------------------
**
**	File Name:  	aosmgr.c
**	
**	Description:	This file contains the implementation of OS wrapper.
**
**					this is kernal code of SW framework.
**					It contributes one of functionalities of SW Platform. 
**					If the checkin is CR not PR, to add change History to this file head part 
**					will be appreciated.
**
**  ---------------------------------------------------------------------------------------------------
**
**  Author:			Warren Zhao
**
** -------------------------------------------------------------------------
**
**	Change History:
**	
**	Initial revision
**
**====================================================================================================*/

#include <sys/types.h>
#include <sys/syscall.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>
#include <unistd.h>

#include "aos.h"
#include "aosmgr.h"
#include "ashm.h"
#include "asemaphore.h"
#include "aheap.h"
#include "alog.h"

/***************************************
 * Local Macros
***************************************/
#define V_LOG A_LOGD
//#define V_LOG
#define V_ERR A_LOGD

//FIXME: add atom protection;
#define MARK_GT(___src,___idx)  (___src[___idx / MASK_INT_LEN] & s_mask[___idx % MASK_INT_LEN])
#define MARK_ST(___src,___idx)  ___src[___idx / MASK_INT_LEN] = ___src[___idx / MASK_INT_LEN] | s_mask[___idx % MASK_INT_LEN]
#define MARK_CL(___src,___idx)  ___src[___idx / MASK_INT_LEN] = ___src[___idx / MASK_INT_LEN] & (~s_mask[___idx % MASK_INT_LEN])

/***************************************
 * Local Typedefs
***************************************/

static Semaphore_t	sema_tag;
static Semaphore_t	sema_tag_ipc;

/***************************************
 * Variable
***************************************/
static const unsigned int s_mask[] = 
{
    0x00000001, 0x00000002, 0x00000004, 0x00000008,
    0x00000010, 0x00000020, 0x00000040, 0x00000080,
    0x00000100, 0x00000200, 0x00000400, 0x00000800,
    0x00001000, 0x00002000, 0x00004000, 0x00008000,
    0x00010000, 0x00020000, 0x00040000, 0x00080000,
    0x00100000, 0x00200000, 0x00400000, 0x00800000,
    0x01000000, 0x02000000, 0x04000000, 0x08000000,
    0x10000000, 0x20000000, 0x40000000, 0x80000000
};

#define AOSMGR_OP_OBTAIN() \
	{ \
		if(mgr && sema_tag && !(mgr->skip_sema_tag_flag) && strcmp(_name,"sema.system.lib.slos.osmgr.sema_tag") != 0) \
		{ \
			V_LOG("\nAOSMGR_TAG_OBTAIN_ENTER-pid=%d\n",getpid()); \
			aosmgr_sig_maskall(); \
			ASEMAPHORE_Obtain( sema_tag, TICKS_FOREVER ); \
			V_LOG("\nAOSMGR_TAG_OBTAIN_EXIT-pid=%d\n",getpid()); \
		} \
	}

#define AOSMGR_OP_RELEASE() \
	{ \
		if(mgr && sema_tag && !(mgr->skip_sema_tag_flag) && strcmp(_name,"sema.system.lib.slos.osmgr.sema_tag") != 0) \
		{ \
			V_LOG("\nAOSMGR_TAG_REL_ENTER-pid=%d\n",getpid()); \
			ASEMAPHORE_Release( sema_tag ); \
			aosmgr_sig_unmaskall(); \
			V_LOG("\nAOSMGR_TAG_REL_EXIT-pid=%d\n",getpid()); \
		} \
	}

#define AOSMGR_IPC_OBTAIN() \
	{ \
		if(mgr && sema_tag_ipc && !(mgr->skip_sema_tag_ipc_flag)) \
		{ \
			V_LOG("\nAOSMGR_IPC_OBTAIN_ENTER-pid=%d\n",getpid()); \
			aosmgr_sig_maskall(); \
			ASEMAPHORE_Obtain( sema_tag_ipc, TICKS_FOREVER ); \
			V_LOG("\nAOSMGR_IPC_OBTAIN_EXIT-pid=%d\n",getpid()); \
		} \
	}

#define AOSMGR_IPC_RELEASE() \
	{ \
		if(mgr && sema_tag_ipc && !(mgr->skip_sema_tag_flag)) \
		{ \
			V_LOG("\nAOSMGR_IPC_REL_ENTER-pid=%d\n",getpid()); \
			ASEMAPHORE_Release( sema_tag_ipc ); \
			aosmgr_sig_unmaskall(); \
			V_LOG("\nAOSMGR_IPC_REL_EXIT-pid=%d\n",getpid()); \
		} \
	}


//#define AOSMGR_OP_OBTAIN() 
//#define AOSMGR_OP_RELEASE() 
//#define AOSMGR_IPC_OBTAIN() 
//#define AOSMGR_IPC_RELEASE() 

static sigset_t	new_set;
static sigset_t	old_set;
static int sig_mask_ref = 0;

void aosmgr_sig_maskall()
{
	sig_mask_ref++;
printf("\naosmgr::aosmgr_sig_maskall::sig_mask_ref=%d::pid=%d\n",sig_mask_ref,getpid());
	if(sig_mask_ref == 1)
	{
		sigfillset(&new_set);
		//if(sigprocmask(SIG_SETMASK, &new_set, &old_set) != 0)
		if(sigprocmask(SIG_SETMASK, &new_set, NULL) != 0)
		{
			perror("<<!!!!!!!!!!!!!sigprocmask!!!!!!!!!!!!!!");
			//exit(1);
			//assert(0);
		}
printf("\naosmgr::aosmgr_sig_maskall::masked::pid=%d\n",getpid());
	}
}

void aosmgr_sig_unmaskall()
{
printf("\naosmgr::aosmgr_sig_unmaskall::sig_mask_ref=%d::pid=%d\n",sig_mask_ref,getpid());
	if(sig_mask_ref > 0)
	{
		sig_mask_ref--;
		if(sig_mask_ref == 0)
		{
			sigemptyset(&new_set);
			if(sigprocmask(SIG_SETMASK, &new_set, NULL) != 0)
			//if(sigprocmask(SIG_SETMASK, &old_set, NULL) != 0)
			{
				perror(">>!!!!!!!!!!!!!sigprocmask!!!!!!!!!!!!!!");
				//exit(1);
			}
printf("\naosmgr::aosmgr_sig_unmaskall::unmasked::pid=%d\n",getpid());
		}
	}
}

static AOSMGR_GC_CB_t s_deadlock_sema_cb = NULL;

void aosmgr_release_deadlock_sema_register(
	AOSMGR_GC_CB_t	in_cb
	)
{
	if(!s_deadlock_sema_cb)
	{
		s_deadlock_sema_cb = in_cb;
	}
}

static int _release_deadlock_sema()
{
	int       i;
	vosmgr_t* mgr = (vosmgr_t*)aosmgr_get();
printf("\naosmgr::_release_deadlock_sema::begin::tid=%ld\n",gettid());
	//check if there is an existing entry 
	for (i = 0; i < OSTAG_Total; i++)
	{
		if ( MARK_GT(mgr->tag_mark, i) )
		{
			if ( mgr->tag[i].type == AOSTAGTYPE_SEMA )
			{
				(*s_deadlock_sema_cb)(mgr,&(mgr->tag[i]));
			}
		}
	}
printf("\naosmgr::_release_deadlock_sema::end::tid=%ld\n",gettid());

	return 0;
}

static int crash_signal_gooff = 0;

static struct sigaction _SIGRESUME_oldact;
static struct sigaction _SIGSUSPEND_oldact;
static sigset_t wait_mask;
static __thread int suspended = 0; // per-thread flag

static void resume_handler(int sig)
{
printf("\naosmgr::resume tid=%ld--sig=%d\n",gettid(),sig);
	suspended = 0;
}

static void suspend_handler(int sig)
{
printf("\naosmgr::suspended tid=%ld--sig=%d\n",gettid(),sig);
	if(crash_signal_gooff)
	{
		if(suspended) 
		{
			return;
		}
printf("\naosmgr::2suspended tid=%ld--sig=%d\n",gettid(),sig);
		suspended = 1;
		while(suspended)
		{
			sigsuspend(&wait_mask);
		}
	}
	else
	{
		_SIGSUSPEND_oldact.sa_handler(sig);
	}
}

// to suspend a thread use pthread_kill(thread_id, SUSPEND_SIG)
// to resume a thread use pthread_kill(thread_id, RESUME_SIG)
static void init_pthread_suspending()
{
	struct sigaction sa;

//	sigfillset(&wait_mask);
//	sigdelset(&wait_mask, SUSPEND_SIG);
//	sigdelset(&wait_mask, RESUME_SIG);

//	sigfillset(&sa.sa_mask);
//	sa.sa_flags = 0;
	//sa.sa_handler = resume_handler;
	//sigaction(RESUME_SIG, &sa, &_SIGRESUME_oldact);

	sa.sa_handler = suspend_handler;
	sigaction(SUSPEND_SIG, &sa, &_SIGSUSPEND_oldact);
}

//caller should release the *o_tids pointered memory
//return is number of tid
int aosmgr_get_alltids(
	const char*	pathname,
	pid_t**		o_tids
	)
{
	pid_t*			filenames; 
	DIR*			dir; 
	struct dirent*	ent; 
	int				n = 0; 
	int				max = 30;

	filenames = (pid_t*)AHEAP_Alloc(max * sizeof(pid_t)); 
	//filenames[0] = -1; 
	
	dir = opendir(pathname); 

	if(!dir)
	{
		return -1;
	}

	while((ent = readdir(dir)))
	{
		if(n >= max)
		{
			RESIZE(filenames,sizeof(pid_t) * (max+30));
			max += 30;
		}
		filenames[n] = aosmgr_str2int(ent->d_name); 
		n++;
	}
	closedir(dir); 
	*o_tids = filenames;

	return n;
}

static struct sigaction _SIGINT_oldact;
static struct sigaction _SIGQUIT_oldact;
static struct sigaction _SIGILL_oldact;
static struct sigaction _SIGABRT_oldact;
static struct sigaction _SIGFPE_oldact;
static struct sigaction _SIGKILL_oldact;
static struct sigaction _SIGSEGV_oldact;
static struct sigaction _SIGTERM_oldact;
static struct sigaction _SIGSTOP_oldact;
static struct sigaction _SIGTSTP_oldact;

static void crash_signals_handler(int sig)
{
	printf("Enter sig = %d; tid=%ld\n",sig,gettid());
	crash_signal_gooff = 1;

	//init_pthread_suspending();
	sigfillset(&new_set);
	sigdelset(&new_set, SUSPEND_SIG);
	sigdelset(&new_set, RESUME_SIG);
	if(sigprocmask(SIG_SETMASK, &new_set, NULL) != 0)
	{
		perror("<<!!!!!!!!!!!!!sigprocmask!!!!!!!!!!!!!!");
		//exit(1);
		//assert(0);
	}

	int		tid_N = 0;
	pid_t*	t_pTids = 0;
	int		i;
	char	t_pathname[256];

	snprintf(t_pathname,255,"/proc/%d/task",getpid());

	tid_N = aosmgr_get_alltids(
		t_pathname,
		&t_pTids
		);
printf("1tgkill::tid_N=%d--t_pTids[2]=%d,t_pTids[3]=%d\n",tid_N,t_pTids[2],t_pTids[3]);
	if( tid_N != -1 && tid_N != 0 )
	{
		for(i = 0; i < tid_N; i++)
		{
			if( t_pTids[i] != -1 && t_pTids[i] != gettid() )
			{
printf("2tgkill::ctid=%ld--t_pTids[i]=%d\n",gettid(),t_pTids[i]);
				tgkill(getpid(), t_pTids[i], SUSPEND_SIG);
printf("3tgkill::ctid=%ld--t_pTids[i]=%d\n",gettid(),t_pTids[i]);
				//pthread_kill(thread_id, SUSPEND_SIG);
			}
		}
	}
printf("4tgkill::tid_N=%d--t_pTids[0]=%d,t_pTids[1]=%d\n",tid_N,t_pTids[0],t_pTids[1]);
	AHEAP_Delete(t_pTids);
printf("5tgkill::tid_N=%d\n",tid_N);

	switch(sig)
	{
	case SIGINT:
		printf("\nSIGINT=%d\n",sig);
		_release_deadlock_sema();
		crash_signal_gooff = 0;
		_SIGINT_oldact.sa_handler(sig);
		break;
	case SIGQUIT:
		printf("\nSIGQUIT=%d\n",sig);
		_release_deadlock_sema();
		crash_signal_gooff = 0;
		_SIGQUIT_oldact.sa_handler(sig);
		break;
	case SIGILL:
		printf("\nSIGILL=%d\n",sig);
		_release_deadlock_sema();
		crash_signal_gooff = 0;
		_SIGILL_oldact.sa_handler(sig);
		break;
	case SIGABRT:
		printf("\nSIGABRT=%d\n",sig);
		_release_deadlock_sema();
		crash_signal_gooff = 0;
		_SIGABRT_oldact.sa_handler(sig);
		break;
	case SIGFPE:
		printf("\nSIGFPE=%d\n",sig);
		_release_deadlock_sema();
		crash_signal_gooff = 0;
		_SIGFPE_oldact.sa_handler(sig);
		break;
//	case SIGKILL:
//		printf("\nSIGKILL=%d\n",sig);
//		_release_deadlock_sema();
//		crash_signal_gooff = 0;
//		_SIGKILL_oldact.sa_handler(sig);
//		break;
	case SIGSEGV:
		printf("\nSIGSEGV=%d\n",sig);
		_release_deadlock_sema();
		crash_signal_gooff = 0;
		_SIGSEGV_oldact.sa_handler(sig);
		break;
	case SIGTERM:
		printf("\nSIGTERM=%d\n",sig);
		_release_deadlock_sema();
		crash_signal_gooff = 0;
		_SIGTERM_oldact.sa_handler(sig);
		break;
//	case SIGSTOP:
//		printf("\nSIGSTOP=%d\n",sig);
//		_release_deadlock_sema();
//		crash_signal_gooff = 0;
//		_SIGSTOP_oldact.sa_handler(sig);
//		break;
	case SIGTSTP:
		printf("\nSIGTSTP=%d\n",sig);
		_release_deadlock_sema();
		crash_signal_gooff = 0;
		_SIGTSTP_oldact.sa_handler(sig);
		break;
	default:
		printf("\ndefault=%d\n",sig);
		assert(0);
		break;
	}
}
 
static void setup_signals(void)
{
	struct sigaction act;
	act.sa_handler = crash_signals_handler;
	//sigaddset(&act.sa_mask, SIGQUIT); 
	//act.sa_flags = SA_RESETHAND | SA_NODEFER; 
 
	sigaction(SIGINT, &act, &_SIGINT_oldact);
	sigaction(SIGQUIT, &act, &_SIGQUIT_oldact);
	sigaction(SIGILL, &act, &_SIGILL_oldact);
	sigaction(SIGABRT, &act, &_SIGABRT_oldact);
	sigaction(SIGFPE, &act, &_SIGFPE_oldact);
//	sigaction(SIGKILL, &act, &_SIGKILL_oldact);
	sigaction(SIGSEGV, &act, &_SIGSEGV_oldact);
	sigaction(SIGTERM, &act, &_SIGTERM_oldact);
//	sigaction(SIGSTOP, &act, &_SIGSTOP_oldact);
	sigaction(SIGTSTP, &act, &_SIGTSTP_oldact);

	init_pthread_suspending();
}

int aosmgr_str2int(const char *str)
{
	int i=0,tmp=0; 

	while(str[i]!='\0') 
	{
		if(str[i]>='0'&&str[i]<='9')
		{
			tmp=tmp*10+(str[i]-'0');
		}
		else
		{
			return -1;
		}
		i++; 
	}
	return tmp;
}

int aosmgr_IsProcessDead(pid_t in_pid)
{
	if(in_pid == -1 || in_pid == 0)
	{
		return 0;
	}

	DIR*   mydir = NULL;
	char   proc_file[65];
printf("\n1AGc::aosmgr_IsProcessDead:in_pid=%d-pid=%d\n",in_pid,getpid());

	//this is case for assert(1)
	snprintf(proc_file,64,"/proc/%d",in_pid);
	proc_file[64] = 0;
printf("\n2AGc::aosmgr_IsProcessDead:proc_file=%s-pid=%d\n",proc_file,getpid());
	mydir = opendir(proc_file);
	//zombie? currently there should not be this case, right?
	if( mydir == NULL )
	{
printf("\n3AGc::aosmgr_IsProcessDead:proc_file=%s-pid=%d\n",proc_file,getpid());
		return 1;
	}
	else
	{
printf("\n4AGc::aosmgr_IsProcessDead:proc_file=%s-pid=%d\n",proc_file,getpid());

		closedir(mydir);
		return 0;
	}
}

static AOSMGR_GC_CB_t s_gc_cb[AOSTAGTYPE_TOTAL] = {
	NULL,NULL,NULL,NULL,NULL
};

void aosmgr_gc_register(
	AOSTagType_t	in_tag_type,
	AOSMGR_GC_CB_t	in_gc_cb
	)
{
	if(in_tag_type <= AOSTAGTYPE_TOTAL)
	{
		s_gc_cb[in_tag_type] = in_gc_cb;
	}
}

static int aosmgr_gc(
	aosmgr_t      _h
	)
{
	V_LOG("enter pid=%d\n",getpid());
	pvosmgr_t mgr = (pvosmgr_t)_h;
	int i   = 0;
	int ret = -1;

	mgr->skip_sema_tag_flag = 1;

	//check if there is an existing entry 
	for (i = 0; i < OSTAG_Total; i++)
	{
		if ( MARK_GT(mgr->tag_mark, i) )
		{
			switch ( mgr->tag[i].type )
			{
			case AOSTAGTYPE_SEMA:
			case AOSTAGTYPE_SEMASET:
			case AOSTAGTYPE_Q:
			case AOSTAGTYPE_BUF:
			case AOSTAGTYPE_PROC:
				if( (mgr->tag[i].type == AOSTAGTYPE_SEMA) && 
					(
					(strcmp(mgr->tag[i].name,"sema.system.lib.slos.osmgr.sema_tag_ipc") == 0) || 
					(strcmp(mgr->tag[i].name,"sema.system.lib.slos.osmgr.sema_tag") == 0)
					)
					)
				{
					break;
				}
				if(s_gc_cb[mgr->tag[i].type])
				{
					if( (*(s_gc_cb[mgr->tag[i].type]))(mgr,&(mgr->tag[i])) != -1 )
					{
						ret = 0;
					}
				}

			case AOSTAGTYPE_TOTAL:
			default:
				break;
			}
		}
	}

	mgr->skip_sema_tag_flag = 0;

	V_LOG("leave pid=%d\n",getpid());
	return ret;
}

/*******************************************************************************
 * 
 * Function Name: aosmgr_get
 * 
 * Description: 
 * 
 * Notes:
*******************************************************************************/
//for each process, it will have this virtual space for its whole life.
//for each device, the osmgr data area will live for the whole device runtime life.
static pvosmgr_t s_pmgr = NULL;
//__attribute__ ((constructor))
aosmgr_t aosmgr_get()        /// returns newly-created os manager
{
	V_LOG("enter...\n");
	int       shm_flag;
	int       shm_sz = sizeof(vosmgr_t);
V_LOG("aosmgr_get1 shm_sz = %d\n",shm_sz);	
	if(s_pmgr)
	{
		return s_pmgr;
	}
V_LOG("aosmgr_get2\n");	

	ashmid_t  t_shm = ashm_create(OSMGR_Key, shm_sz, &shm_flag);
	if (t_shm < 0)
	{
V_LOG("aosmgr_get3\n");	
		V_LOG("error: ashm_create()\n");
		goto EXIT;
	}
	
	// The share memory is newly constructed
	if (shm_flag == 1)
	{
V_LOG("4:fd(t_shm)=%d\n",t_shm);	

		V_LOG("The os manager is newly created\n");
		assert(s_pmgr==NULL);
		s_pmgr = (pvosmgr_t)ashm_attach(t_shm);
		if (s_pmgr == 0)
		{
V_LOG("aosmgr_get5\n");	
			V_ERR("error: ashm_attach()\n");
			goto EXIT;
		}
		
		memset(s_pmgr, 0, shm_sz);
		s_pmgr->shm_id = t_shm;

		sema_tag = ASEMAPHORE_Create_IPC(
			(char *)"sema.system.lib.slos.osmgr.sema_tag",
			1,
			1,
			OSSUSPEND_FIFO,
			1
			);
		assert(sema_tag);

		sema_tag_ipc = ASEMAPHORE_Create_IPC(
			(char *)"sema.system.lib.slos.osmgr.sema_tag_ipc",
			1,
			1,
			OSSUSPEND_FIFO,
			1
			);
		assert(sema_tag_ipc);
	}
	else
	{
V_LOG("aosmgr_get6\n");	

		if(s_pmgr==NULL)
		{
V_LOG("aosmgr_get7\n");	

			V_LOG("The os manager is acquired\n");
			s_pmgr = (pvosmgr_t)ashm_attach(t_shm);

			sema_tag = ASEMAPHORE_GetHandle_IPC((char *)"sema.system.lib.slos.osmgr.sema_tag");
			assert(sema_tag);
			sema_tag_ipc = ASEMAPHORE_GetHandle_IPC((char *)"sema.system.lib.slos.osmgr.sema_tag_ipc");
			assert(sema_tag_ipc);
		}
	}

	//setup_signals();
	
EXIT:
	V_LOG("leave...\n");
	return s_pmgr;
}

/*******************************************************************************
 * 
 * Function Name: aosmgr_destroy
 * 
 * Description: 
 * 
 * Notes:   only sw recovery will call this function. no process should call it.
*******************************************************************************/
void aosmgr_destroy(aosmgr_t _h)
{
	V_LOG("enter...\n");
	int i = 0;
	pvosmgr_t mgr = (pvosmgr_t)_h;
	if (mgr != 0)
	{
		/*
		V_LOG("Destroy\n");
		for(i = 0; i < OSTAG_Total; i++)
		{
			if (MARK_GT(mgr->tag_mark, i))
			{
				// TODO: (do what you wanna do)
				V_LOG("name:[%s], id:[%d]\n", mgr->tag[i].name, mgr->tag[i].id);
				asema_destroy(mgr->tag[i].id);
			}
		}
		*/

		if(sema_tag)
		{
			ASEMAPHORE_Destroy( sema_tag );
		}

		if(sema_tag_ipc)
		{
			ASEMAPHORE_Destroy( sema_tag_ipc );
		}

		ashmid_t  t_shm_id = mgr->shm_id;
		ashm_deattach(mgr);
		ashm_destroy(t_shm_id);
	}
	s_pmgr = NULL;
	V_LOG("leave...\n");
}

/*******************************************************************************
 * 
 * Function Name: aosmgr_op_getid
 * 
 * Description: 
 * 
 * Notes:
*******************************************************************************/
void* aosmgr_op_gethandle(
	aosmgr_t _h, 
	char*    _name,
	int      _id
	)
{
	V_LOG("enter...\n");
	
	int i  = 0;
	void* t_handle = 0;
	pvosmgr_t mgr = (pvosmgr_t)_h;

	/// Check the validity of para
	if (mgr == 0 || _name == 0 || _id < 0)
	{
		V_ERR("error: paras is invalid\n");
		goto EXIT;
	}

	//search tag array for the specified one 
	for (i = 0; i < OSTAG_Total; i++)
	{
		if(_id == 0)
		{
			if (MARK_GT(mgr->tag_mark, i) && strcmp(_name,mgr->tag[i].name) == 0)
			{
				t_handle = &(mgr->tag[i].handle);
				V_LOG("Data: name:[%s], ID:[%d], CNT:[%d] registed!\n", mgr->tag[i].name, mgr->tag[i].id, 1);
				goto EXIT;
			}
		}
		else
		{
			if (MARK_GT(mgr->tag_mark, i) && _id == mgr->tag[i].id)
			{
				t_handle = &(mgr->tag[i].handle);
				V_LOG("Data: name:[%s], ID:[%d], CNT:[%d] registed!\n", mgr->tag[i].name, mgr->tag[i].id, 1);
				goto EXIT;
			}
		}
	}
		
EXIT:

	V_LOG("leave...\n");
	return t_handle;
}


/*******************************************************************************
 * 
 * Function Name: aosmgr_op_getid
 * 
 * Description: 
 * 
 * Notes:
*******************************************************************************/
int aosmgr_op_getid(
	aosmgr_t _h, 
	char*    _name
	)
{
	V_LOG("enter...\n");
	
	int i  = 0;
	int id = -1;
	pvosmgr_t mgr = (pvosmgr_t)_h;

	/// Check the validity of para
	if (mgr == 0 || _name == 0)
	{
		V_ERR("error: paras is invalid\n");
		goto EXIT;
	}
	
	for (; i < OSTAG_Total; i++)
	{
		if (MARK_GT(mgr->tag_mark, i) && strcmp(mgr->tag[i].name, _name) == 0)
		{
			id = mgr->tag[i].id;
			break;
		}
	}
	
EXIT:

	V_LOG("leave...\n");
	return id;
}

/*******************************************************************************
 * 
 * Function Name: aosmgr_op_getnm
 * 
 * Description: 
 * 
 * Notes:
*******************************************************************************/
char* aosmgr_op_getnm(
	aosmgr_t _h, 
	int      _id
	)
{
	V_LOG("enter...\n");
	
	int i     = 0;
	char* ret = 0;
	pvosmgr_t mgr = (pvosmgr_t)_h;

	/// Check the validity of para
	if (mgr == 0 || _id <= 0)
	{
		V_ERR("error: paras is invalid\n");
		goto EXIT;
	}
	
	for (; i < OSTAG_Total; i++)
	{
		if (MARK_GT(mgr->tag_mark, i) && _id == mgr->tag[i].id)
		{
			ret = mgr->tag[i].name;
			break;
		}
	}
	
EXIT:
	V_LOG("leave...\n");
	return ret;
}

/*******************************************************************************
 * 
 * Function Name: aosmgr_op_reg
 * 
 * Description: 

 * @return  A return value of -1 represents the <aosmgr_op_reg> operation failed, >=0 represents the count that the semaphore is referenced.
 * 
 * Notes:
*******************************************************************************/
int aosmgr_op_reg(
	aosmgr_t      _h, 
	char*         _name, 
	int           _id,
	AOSTagType_t  _type,
	void*         _handle
	)
{
	V_LOG("enter...\n");
	
	int i   = 0;
	int ret = 0;
	int len = 0;
	pvosmgr_t mgr = (pvosmgr_t)_h;

	AOSMGR_OP_OBTAIN();

	/// Check the validity of para
	if (mgr == 0 || _name == 0 || _id < 0)
	{
		V_ERR("error: paras is invalid\n");
		ret = -1;
		goto EXIT;
	}
	
	len = strlen(_name);
	if (len >= OSTAGName_Len)
	{
		V_ERR("error: the length of name is too longer than needed.\n");
		ret = -1;
		goto EXIT;
	}
	
	//check if there is an existing entry 
	for (i = 0; i < OSTAG_Total; i++)
	{
		if(_id == 0)
		{
			if (MARK_GT(mgr->tag_mark, i) && strcmp(_name,mgr->tag[i].name) == 0)
			{
				ret = 1;
				V_LOG("Data: name:[%s], ID:[%d], CNT:[%d] registed!\n", mgr->tag[i].name, mgr->tag[i].id, 1);
				goto EXIT;
			}
		}
		else
		{
			if (MARK_GT(mgr->tag_mark, i) && _id == mgr->tag[i].id)
			{
				ret = 1;
				V_LOG("Data: name:[%s], ID:[%d], CNT:[%d] registed!\n", mgr->tag[i].name, mgr->tag[i].id, 1);
				goto EXIT;
			}
		}
	}

REALLOC:
	// Add a new-created entry 
	for (i = 0; i < OSTAG_Total; i++)
	{
		if (MARK_GT(mgr->tag_mark, i) == 0)
		{
			MARK_ST(mgr->tag_mark, i);
			mgr->tag[i].id = _id;
			mgr->tag[i].type = _type;
			if(_handle)
			{
				mgr->tag[i].handle =  *((res_handle_t*)_handle);
			}
			mgr->tag[i].pid = getpid();
			strncpy(mgr->tag[i].name, _name, OSTAGName_Len);
			mgr->tag[i].name[OSTAGName_Len] = 0;
			ret = 1;
			V_LOG("Data: name:[%s], ID:[%d], CNT:[%d] registed!\n", mgr->tag[i].name, mgr->tag[i].id, 1);
			goto EXIT;
		}
	}
	
	if( (ret != -1) )
	{
		ret = -1;
		if(aosmgr_gc(_h) != -1)
		{
			goto REALLOC;
		}
	}
	V_ERR("error: Data: name:[%s], ID:[%d] cannot be registed because of full array!\n", _name, _id);

EXIT:
	AOSMGR_OP_RELEASE();

	V_LOG("leave...\n");
	return ret;
}

/*******************************************************************************
 * 
 * Function Name: aosmgr_op_unreg
 * 
 * Description: 
 * 
 * Notes:
*******************************************************************************/
int aosmgr_op_unreg(
	aosmgr_t _h, 
	char*    _name, 
	int      _id
	)
{
	V_LOG("enter..._name=%s\n",_name);
	
	int i   = 0;
	int ret = 0;
	int len = 0;
	pvosmgr_t mgr = (pvosmgr_t)_h;

	AOSMGR_OP_OBTAIN();

	//Check the validity of para
	if (mgr == 0 || (_name == 0 && _id < 0))
	{
		V_ERR("error: paras is invalid\n");
		ret = -1;
		goto EXIT;
	}
	
	if (_name != 0)
	{
		for (i = 0; i < OSTAG_Total; i++)
		{
			if (MARK_GT(mgr->tag_mark, i) && strcmp(mgr->tag[i].name, _name) == 0)
			{
				mgr->tag[i].name[0] = 0;
				mgr->tag[i].pid = 0;
				MARK_CL(mgr->tag_mark, i);
				ret = 1;
				
				V_LOG("Data: name:[%s], ID:[%d], CNT:[%d] uregisted!\n", mgr->tag[i].name, mgr->tag[i].id, 1);
				break;
			}
		}
	}
	else
	{
		for (i = 0; i < OSTAG_Total; i++)
		{
			if (MARK_GT(mgr->tag_mark, i) && mgr->tag[i].id == _id)
			{
				mgr->tag[i].name[0] = 0;
				mgr->tag[i].pid = 0;
				MARK_CL(mgr->tag_mark, i);
				ret = 1;
				V_LOG("Data: name:[%s], ID:[%d], CNT:[%d] uregisted!\n", mgr->tag[i].name, mgr->tag[i].id, 1);
				break;
			}
		}
	}
	
EXIT:
	AOSMGR_OP_RELEASE();

	V_LOG("leave...\n");
	return ret;
}

//////remote call related stuffs//////

/*******************************************************************************
 * 
 * Function Name: aosmgr_args_alloc
 * 
 * Description: 

 * @return  NULL=failure, NOT NULL is args buffer;
 * 
 * Notes:
*******************************************************************************/
static int aosmgr_ipc_gc(
	aosmgr_t      _h
	)
{
	V_LOG("enter pid=%d\n",getpid());
	pvosmgr_t mgr = (pvosmgr_t)_h;
	int i   = 0;
	int ret = -1;

	mgr->skip_sema_tag_ipc_flag = 1;

	for (i = 0; i < OSTAG_IPC_Total; i++)
	{
		if ( MARK_GT(mgr->tag_ipc_mark, i) )
		{
			if(aosmgr_IsProcessDead(mgr->tag_ipc[i].client_pid))
			{
				aosmgr_args_free(
					_h,
					mgr->tag_ipc[i].relayer_name,
					mgr->tag_ipc[i].client_pid
					);
				ret = 0;
			}
			else
			{
				if(aosmgr_str2int( mgr->tag_ipc[i].relayer_name ) != -1)
				{
					if(aosmgr_IsProcessDead(aosmgr_str2int( mgr->tag_ipc[i].relayer_name )))
					{
						aosmgr_args_free(
							_h,
							mgr->tag_ipc[i].relayer_name,
							mgr->tag_ipc[i].client_pid
							);
						ret = 0;
					}
				}
			}
		}
	}

	mgr->skip_sema_tag_ipc_flag = 0;

	V_LOG("leave pid=%d\n",getpid());
	return ret;
}

int aosmgr_args_clientds_num(
	aosmgr_t _h,
	char*    _relayer_name
	)
{
	V_LOG("enter::_relayer_name=%s\n",_relayer_name);

	int       i   = 0;
	int       ret = 0;
	int       len = 0;
	pvosmgr_t mgr = (pvosmgr_t)_h;

	//Check the validity of para
	if (mgr == 0 || _relayer_name == 0)
	{
		V_ERR("error: paras is invalid\n");
		ret = -1;
		goto EXIT;
	}
	if(_relayer_name[0] == 0)
	{
		ret = -1;
		//assert(0);
		goto EXIT;
	}
	
	for (i = 0; i < OSTAG_IPC_Total; i++)
	{
		if ( MARK_GT(mgr->tag_ipc_mark, i) && (strcmp(mgr->tag_ipc[i].relayer_name,_relayer_name)==0) )
		{
			ret++;
		}
	}
	
EXIT:
	V_LOG("leave...\n");
	return ret;
}

aosmgr_ipc_tag* aosmgr_args_alloc(
	aosmgr_t     _h, 
	const char*  _relayer_name,		//
	pid_t        _client_pid		//
	)
{
	V_LOG("enter...\n");
V_LOG(": _relayer_name=%s,_client_pid=%d\n",_relayer_name,_client_pid);

	int					i = 0;
	aosmgr_ipc_tag*		ret = NULL;
	int					len = 0;
	pvosmgr_t			mgr = (pvosmgr_t)_h;
	int					gc_res = 0;

	AOSMGR_IPC_OBTAIN();

	/// Check the validity of para
	if (mgr == 0 || _relayer_name == 0 || _client_pid <= 0)
	{
		V_ERR("error: args are invalid\n");
		ret = NULL;
		//assert(0);
		goto EXIT;
	}
	if(_relayer_name[0] == 0)
	{
		ret = NULL;
		//assert(0);
		goto EXIT;
	}	
	if ( (strlen(_relayer_name) >= OSTAGName_Len) )
	{
		V_ERR("error: the length of name is too longer than needed.\n");
		ret = NULL;
		//assert(0);
		goto EXIT;
	}
	
	//check if there is an existing entry 
	for (i = 0; i < OSTAG_IPC_Total; i++)
	{
		if ( MARK_GT(mgr->tag_ipc_mark, i) && _client_pid == mgr->tag_ipc[i].client_pid && (strcmp(mgr->tag_ipc[i].relayer_name,_relayer_name) == 0) )
		{
			ret = NULL;
			V_ERR("Data: relayer_name:[%s], _client_pid:[%d], already registed!\n", mgr->tag_ipc[i].relayer_name,_client_pid);
			//only one entry for each process. no reentry to rapi.
			//assert(0);
			goto EXIT;
		}
	}
/*
	if(aosmgr_args_clientds_num(mgr,_relayer_name) >= SEMA_MAX_MUTEX_VAL)
	{
		V_ERR("Data: relayer_name:[%s], _client_pid:[%d], clients reached max client num = %d!\n", mgr->tag_ipc[i].relayer_name,_client_pid,SEMA_MAX_MUTEX_VAL);
		ret = NULL;
		goto EXIT;
	}
*/
REALLOC:
	// Add a new-created entry 
	for (i = 0; i < OSTAG_IPC_Total; i++)
	{
		if (MARK_GT(mgr->tag_ipc_mark, i) == 0)
		{
			MARK_ST(mgr->tag_ipc_mark, i);
			mgr->tag_ipc[i].relayer_pid = 0;
			mgr->tag_ipc[i].client_pid = _client_pid;
			strncpy(mgr->tag_ipc[i].relayer_name, _relayer_name, OSTAGName_Len);
			mgr->tag_ipc[i].relayer_name[OSTAGName_Len] = 0;
			mgr->tag_ipc[i].inf_name[0] = 0;
			mgr->tag_ipc[i].func_name[0] = 0;
			mgr->tag_ipc[i].ret = RAPI_INVALID_RET_VAL;
			mgr->tag_ipc[i].ready = 0;
			mgr->tag_ipc[i].reg = 0;
			ret = &(mgr->tag_ipc[i]);
			V_LOG("Data: relayer_name:[%s], _client_pid:[%d],registry created!\n", mgr->tag_ipc[i].relayer_name,_client_pid);
			goto EXIT;
		}
	}
	
	ret = NULL;
	if( gc_res != -1 )
	{
		gc_res = -1;
		if(aosmgr_ipc_gc(_h) != -1)
		{
			goto REALLOC;
		}
	}
	V_ERR("error: Data: relayer_name:[%s],_client_pid:[%d] cannot be registed because of full array!\n", _relayer_name,_client_pid);

EXIT:
	AOSMGR_IPC_RELEASE();
	V_LOG("leave...\n");
	return ret;
}

aosmgr_ipc_tag* aosmgr_args_handle(
	aosmgr_t     _h, 
	const char*  _relayer_name,
	pid_t        _client_pid		//
	)
{
	V_LOG("enter...\n");
V_LOG(": _relayer_name=%s,_client_pid=%d\n",_relayer_name,_client_pid);
	
	int					i = 0;
	aosmgr_ipc_tag*		ret = NULL;
	int					len = 0;
	pvosmgr_t			mgr = (pvosmgr_t)_h;

	/// Check the validity of para
	if (mgr == 0 || _relayer_name == 0 || _client_pid <= 0)
	{
		V_ERR("error: args are invalid\n");
		ret = NULL;
		assert(0);
		goto EXIT;
	}
	if(_relayer_name[0] == 0)
	{
		ret = NULL;
		assert(0);
		goto EXIT;
	}
	
	if ( (strlen(_relayer_name) >= OSTAGName_Len) )
	{
		V_ERR("error: the length of name is too longer than needed.\n");
		ret = NULL;
		assert(0);
		goto EXIT;
	}
	
	//check if there is an existing entry 
	for (i = 0; i < OSTAG_IPC_Total; i++)
	{
		if ( MARK_GT(mgr->tag_ipc_mark, i) && _client_pid == mgr->tag_ipc[i].client_pid && (strcmp(mgr->tag_ipc[i].relayer_name,_relayer_name) == 0) )
		{
			ret = &(mgr->tag_ipc[i]);
			V_LOG("Data: relayer_name:[%s], already registed!\n", mgr->tag_ipc[i].relayer_name);
			//only one entry for each process. no reentry to rapi.
			//assert(0);
			goto EXIT;
		}
	}
	
	ret = NULL;
	V_ERR("relayer_name:[%s] cannot be found!\n", _relayer_name);

EXIT:
	V_LOG("leave...\n");
	return ret;
}

/*******************************************************************************
 * 
 * Function Name: aosmgr_args_unreg
 * 
 * Description: 
 * 
 * Notes:
*******************************************************************************/
int aosmgr_args_free(
	aosmgr_t _h, 
	char*    _relayer_name,
	pid_t    _client_pid
	)
{
	V_LOG("enter... free::_relayer_name=%s,_client_pid=%d\n",_relayer_name,_client_pid);
	
	int       i   = 0;
	int       ret = 0;
	int       len = 0;
	pvosmgr_t mgr = (pvosmgr_t)_h;

	AOSMGR_IPC_OBTAIN();

	//Check the validity of para
	if (mgr == 0 || _relayer_name == 0)
	{
		V_ERR("error: paras is invalid\n");
		ret = -1;
		goto EXIT;
	}
	if(_relayer_name[0] == 0)
	{
		ret = -1;
		//assert(0);
		goto EXIT;
	}
	
	for (i = 0; i < OSTAG_IPC_Total; i++)
	{
		if ( MARK_GT(mgr->tag_ipc_mark, i) && (strcmp(mgr->tag_ipc[i].relayer_name,_relayer_name)==0) && 
			(_client_pid <= 0 || _client_pid == mgr->tag_ipc[i].client_pid) )
		{
			//the client is using it. skip this recycle at this time moment.
			if(mgr->tag_ipc[i].reg)
			{
				break;
			}
			mgr->tag_ipc[i].client_pid = 0;
			mgr->tag_ipc[i].relayer_name[0] = 0;
			mgr->tag_ipc[i].func_name[0] = 0;
			mgr->tag_ipc[i].inf_name[0] = 0;
			mgr->tag_ipc[i].ret = RAPI_INVALID_RET_VAL;
V_LOG(": mgr->tag_ipc[i].ret=%x;\n",mgr->tag_ipc[i].ret);
			mgr->tag_ipc[i].relayer_pid = 0;
			mgr->tag_ipc[i].ready = 0;
			mgr->tag_ipc[i].reg = 0;
			MARK_CL(mgr->tag_ipc_mark, i);

			ret = 0;

			V_LOG("Data: relayer_name:[%s] unregisted!\n", _relayer_name);
			if(_client_pid > 0)
			{
				break;
			}
		}
	}
	
EXIT:
	AOSMGR_IPC_RELEASE();
	V_LOG("leave...\n");
	return ret;
}

char* aosmgr_args_reg(
	aosmgr_ipc_tag*		_ipc_tag, 
	const char*			_func_name,
	const char*			_inf_name
	)
{
	char*     ret = NULL;

	/// Check the validity of para
	if ( _func_name == 0 || _inf_name == 0 )
	{
		V_ERR("error: args are invalid\n");
		ret = NULL;
		assert(0);
		goto EXIT;
	}
	if ( _func_name[0] == 0 || _inf_name[0] == 0 )
	{
		ret = NULL;
		assert(0);
		goto EXIT;
	}
	if ( (strlen(_func_name) >= OSTAGFUNCName_Len) || (strlen(_inf_name) >= OSTAGName_Len) )
	{
		V_ERR("error: the length of name is too longer than needed.\n");
		ret = NULL;
		assert(0);
		goto EXIT;
	}

	if(_ipc_tag->client_pid == getpid() && _ipc_tag->inf_name[0] == 0 && _ipc_tag->func_name[0] == 0)
	{
		_ipc_tag->reg = 1;
		strncpy(_ipc_tag->inf_name, _inf_name, OSTAGName_Len);
		_ipc_tag->inf_name[OSTAGName_Len] = 0;
V_LOG(": _ipc_tag->inf_name=%s;\n",_ipc_tag->inf_name);
		strncpy(_ipc_tag->func_name, _func_name, OSTAGFUNCName_Len);
		_ipc_tag->func_name[OSTAGFUNCName_Len] = 0;
		_ipc_tag->ret = RAPI_INVALID_RET_VAL;
V_LOG(": _ipc_tag->ret=%x;\n",_ipc_tag->ret);
		_ipc_tag->ready = 0;
		ret = _ipc_tag->args;
		V_LOG("Data: relayer_name:[%s], registry got!\n", _ipc_tag->relayer_name);
	}
	else
	{
		V_ERR("error: args tag alloc failed:_ipc_tag->client_pid=%d-_ipc_tag->inf_name=%s-_ipc_tag->func_name=%s-pid=%d.\n",_ipc_tag->client_pid,_ipc_tag->inf_name,_ipc_tag->func_name,getpid());
		ret = NULL;
	}

EXIT:

	return ret;
}

void aosmgr_args_unreg(
	aosmgr_ipc_tag*		_ipc_tag
	)
{
	if( _ipc_tag->client_pid != getpid() )
	{
		V_ERR("error: args tag is recycled or allocated to other client pid.\n");
		return;
	}
//	if(_ipc_tag->func_name[0] == 0 || _ipc_tag->func_name[0] == 0)
//	{
//		V_ERR("error: args tag info is not correct.\n");
//	}
	_ipc_tag->inf_name[0] = 0;
	_ipc_tag->func_name[0] = 0;
	_ipc_tag->ret = RAPI_INVALID_RET_VAL;
	_ipc_tag->ready = 0;
	_ipc_tag->reg = 0;
}

aosmgr_ipc_tag* aosmgr_args_next(
	aosmgr_t        _h, 
	const char*     _relayer_name,
	int*            _curr_idx
	)
{
	V_LOG("enter...\n");
V_LOG(": _relayer_name=%s\n",_relayer_name);
	
	int					i = 0;
	aosmgr_ipc_tag*		ret = NULL;
	int					len = 0;
	pvosmgr_t			mgr = (pvosmgr_t)_h;

	/// Check the validity of para
	if (mgr == 0 || _relayer_name == 0 || _curr_idx == 0)
	{
		V_ERR("error: args are invalid\n");
		ret = NULL;
		assert(0);
		goto EXIT;
	}
	if(_relayer_name[0] == 0)
	{
		ret = NULL;
		assert(0);
		goto EXIT;
	}
	
	if ( (strlen(_relayer_name) >= OSTAGName_Len) )
	{
		V_ERR("error: the length of name is too longer than needed.\n");
		ret = NULL;
		assert(0);
		goto EXIT;
	}
	
	//check if there is an existing entry 
	for (i = *_curr_idx; i < OSTAG_IPC_Total; i++)
	{
		if ( MARK_GT(mgr->tag_ipc_mark, i) && (strcmp(mgr->tag_ipc[i].relayer_name,_relayer_name) == 0) && mgr->tag_ipc[i].ready )
		{
			ret = &(mgr->tag_ipc[i]);
			*_curr_idx = ((i + 1) % OSTAG_IPC_Total);
			goto EXIT;
		}
	}

	for (i = 0; i < *_curr_idx; i++)
	{
		if ( MARK_GT(mgr->tag_ipc_mark, i) && (strcmp(mgr->tag_ipc[i].relayer_name,_relayer_name) == 0)  && mgr->tag_ipc[i].ready )
		{
			ret = &(mgr->tag_ipc[i]);
			*_curr_idx = ((i + 1) % OSTAG_IPC_Total);
			goto EXIT;
		}
	}
	
	ret = NULL;
V_LOG(": _relayer_name=%s: no args entry left\n",_relayer_name);

EXIT:
	V_LOG("leave...\n");
	return ret;
}
