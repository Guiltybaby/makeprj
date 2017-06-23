/*  ====================================================================================================
**
**  ---------------------------------------------------------------------------------------------------
**
**	File Name:  	aosmgr.h
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

#ifndef ___AOSMGR_H___
#define ___AOSMGR_H___

#include <pthread.h>
#include "aostypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************/
/* Global Macros*/
/*******************************************************************************/
/******************************************************************************/
/* Global Typedefs*/
/******************************************************************************/

#define RESUME_SIG		SIGUSR1
#define SUSPEND_SIG		SIGUSR2

typedef void* aosmgr_t;

#define OSTAGFUNCName_Len  128

typedef int AOSTagType_t;
enum
{
	AOSTAGTYPE_SEMA,
	AOSTAGTYPE_SEMASET,
	AOSTAGTYPE_Q,
	AOSTAGTYPE_BUF,
	AOSTAGTYPE_PROC,
	AOSTAGTYPE_TOTAL
};

#define SEMA_MAX_WAITING_LOG_NUM	48
#define SEMA_MAX_MUTEX_VAL			16
typedef struct _ASemaHandleData_t
{
	int				stamp;
	int				stamp_releasing;
	char			name[OSTAGName_Len+1];
	int				shared;
	int				mutexed;
	unsigned int	maxval;
	int				thread_flag;
	pthread_cond_t	thread_flag_cv;
	pthread_mutex_t	thread_flag_mutex;
	unsigned int	waiting_num;
	unsigned int	obtainer_num;
	pid_t			waiting_list[SEMA_MAX_WAITING_LOG_NUM];
	pid_t			obtainer_list[SEMA_MAX_MUTEX_VAL];
	struct _ASemaHandleData_t*	pNext;
} ASemaHandleData_t;

typedef struct _ASema_t
{
	asemasetid_t   sems;
	unsigned short maxval;
} ASema_t;


typedef union _res_handle_t
{
	ASemaHandleData_t	s;
	ASema_t             s_shared_only;
} res_handle_t;

typedef struct _aosmgr_tag
{
	//v_cnt_t count;
	akey_t        key;
	aid_t         id;
	AOSTagType_t  type;
	pid_t         pid;
	char          name[OSTAGName_Len+1];
	res_handle_t  handle;
} aosmgr_tag_t;

typedef struct _aosmgr_ipc_tag
{
	char          args[OSTAGArgs_Len+1];
	pid_t         relayer_pid;
	pid_t         client_pid;
	char          relayer_name[OSTAGName_Len+1];
	char          func_name[OSTAGFUNCName_Len+1];
	char          inf_name[OSTAGName_Len+1];
	int           ret;
	int           ready;
	int           reg;
} aosmgr_ipc_tag;

#define RAPI_INVALID_RET_VAL             0xEFFFFFFF
#define handle_to_osmgr_tag(ptr)         ( (aosmgr_tag_t*)(container_of(ptr, aosmgr_tag_t, handle)) )

typedef int (*AOSMGR_DEADLOCK_SEMA_CB_t)(
	aosmgr_t		_h,
	aosmgr_tag_t*	in_tag
	);

void aosmgr_release_deadlock_sema_register(
	AOSMGR_DEADLOCK_SEMA_CB_t	in_cb
	);

typedef int (*AOSMGR_GC_CB_t)(
	aosmgr_t		_h,
	aosmgr_tag_t*	in_tag
	);

void aosmgr_gc_register(
	AOSTagType_t	in_tag_type,
	AOSMGR_GC_CB_t	in_gc_cb
	);

#define OSTAG_Total     186
#define OSTAG_IPC_Total 128

#define MASK_INT_LEN    (sizeof(int))
#define MASK_NUM(___ttl)    ((___ttl + MASK_INT_LEN - 1) / MASK_INT_LEN)

typedef struct _vosmgr_t
{
	ashmid_t         shm_id;

	int              entries;
	unsigned int     tag_mark      [ MASK_NUM(OSTAG_Total) ];
	aosmgr_tag_t     tag           [ OSTAG_Total ];

	unsigned int     tag_ipc_mark  [ MASK_NUM(OSTAG_IPC_Total) ];
	aosmgr_ipc_tag   tag_ipc       [ OSTAG_IPC_Total ];

	int              mutex_flag_sema1;
	int              mutex_flag_rapi1;
	int              skip_sema_tag_flag;
	int              skip_sema_tag_ipc_flag;
} vosmgr_t, *pvosmgr_t;

//******************************************************************************
// Global Function Prototypes
//******************************************************************************
void aosmgr_sig_maskall();
void aosmgr_sig_unmaskall();


//caller should release the *o_tids pointered memory
//return is number of tid
int aosmgr_get_alltids(
	const char*	pathname,
	pid_t**		o_tids
	);
int aosmgr_str2int(const char *str);
int aosmgr_IsProcessDead(pid_t in_pid);
/** aosmgr_get
 * @brief  to create os manager.
 * @exception NULL
 * @param     NULL
 * @return    newly-created os manager.
 */
//__attribute__ ((constructor))
aosmgr_t aosmgr_get();

/** aosmgr_destroy
 * @brief  to destroy os manager.
 * @exception NULL
 * @param     [IN] _h is the os manager you want to delete, the times the semaphore referenced reduce 1 per launching 
                      until the count equal 0, then destroy the os manager.
 * @return    NULL.
 * destroy is OS level. pls do not call it in process domain.
 */
void aosmgr_destroy(aosmgr_t _h);

void* aosmgr_op_gethandle(
	aosmgr_t _h, 
	char*    _name,
	int      _id
	);

int   aosmgr_op_getid(
	aosmgr_t _h, 
	char*    _name
	);

char* aosmgr_op_getnm(
	aosmgr_t _h, 
	int      _id
	);

int aosmgr_op_reg(
	aosmgr_t      _h, 
	char*         _name, 
	int           _id,
	AOSTagType_t  _type,
	void*         _handle
	);

int aosmgr_op_unreg(
	aosmgr_t _h, 
	char*    _name, 
	int      _id
	);

int aosmgr_args_clientds_num(
	aosmgr_t _h,
	char*    _relayer_name
	);

aosmgr_ipc_tag* aosmgr_args_alloc(
	aosmgr_t     _h, 
	const char*  _relayer_name,		//
	pid_t        _client_pid		//
	);

aosmgr_ipc_tag* aosmgr_args_handle(
	aosmgr_t     _h, 
	const char*  _relayer_name,
	pid_t        _client_pid		//
	);

int aosmgr_args_free(
	aosmgr_t _h, 
	char*    _relayer_name,
	pid_t    _client_pid
	);

char* aosmgr_args_reg(
	aosmgr_ipc_tag*		_ipc_tag, 
	const char*			_func_name,
	const char*			_inf_name
	);

void aosmgr_args_unreg(
	aosmgr_ipc_tag*		_ipc_tag
	);

aosmgr_ipc_tag* aosmgr_args_next(
	aosmgr_t        _h, 
	const char*     _relayer_name,
	int*            _curr_idx
	);

#ifdef __cplusplus
}
#endif

#endif /// ___AOSMGR_H___
