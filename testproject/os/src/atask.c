/*  ====================================================================================================
**
**  ---------------------------------------------------------------------------------------------------
**
**    File Name:      atask.cpp
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
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include "atask.h"
#include "aosmgr.h"


/***************************************
 * Global Macros
***************************************/

//#define VL_LOGV SL_LOGD
#define VL_LOGV
#define VL_ERR printf

/***************************************
 * Local Typedefs
***************************************/
/** @brief This is the vthread attribute flag. */
typedef struct
{
    unsigned int detach   : 1; /**< detach status, please refer to ATASK_DETACH_Status    */
    unsigned int scope    : 1; /**< contention scope, please refer to ATASK_Scope         */
    unsigned int policy   : 2; /**< scheduling policy, please refer to ATASK_SCHED_Policy */
    unsigned int priority : 8; /**< priority, please refer to ATASK_Priority              */
    unsigned int reserve  : 8;
} VTHREADFlag_t;

/** @brief This is the vthread structure. */
typedef struct
{
    TEntry_t func;
    TArgv_t  arg;
}ENTRYParms_t, *PENTRYParms_t;

typedef struct
{
    char				name[ATASKNAME_LEN + 1];
    pthread_t			id;
    union
    {
        VTHREADFlag_t	attr;
        unsigned int	flag;
    } u;
    ENTRYParms_t		entry;
} vthread_t, *pvthread_t;



static void* THREADEntry(void* _parm)
{
    PENTRYParms_t entryparms = (PENTRYParms_t)_parm;
    //sigset_t      set;
    //sigfillset(&set);
    //int nRes = pthread_sigmask(SIG_BLOCK, &set, NULL);
    //if (nRes != 0)
    //{
    //    VL_ERR("pthread_sigmask() nRes:[%d]\n", nRes);
    //}

    return entryparms->func(entryparms->arg);
}

//******************************************************************************
//
// Function Name:    ATASK_Create
//
// Description:        This function creates a task
//
// Notes:
//
//******************************************************************************
Task_t ATASK_Create(                        // returns the newly-created task
    TEntry_t        _entry,                 // task function entry point
    TArgv_t         _argv,
    TName_t         _task_name,          // task name
    TPriority_t     _priority,           // task priority, bigger is higher
    TStackSize_t    _stack_size             // task stack size (in UInt8)
    )
{
	return ATASK_CreateExt(                    // returns the newly-created task
		_entry,                 // task function entry point
		_argv,
		_task_name,          // task name
		_priority,           // task priority, bigger is higher
		_stack_size,             // task stack size (in UInt8)
		ATASK_SCHED_RR
		);
}

Task_t ATASK_CreateExt(                    // returns the newly-created task
    TEntry_t				_entry,                 // task function entry point
    TArgv_t					_argv,
    TName_t					_task_name,          // task name
    TPriority_t				_priority,           // task priority, bigger is higher
    TStackSize_t			_stack_size,         // task stack size (in UInt8)
	ATASK_SCHED_Policy_t	_policy
    )
{
    VL_LOGV("enter...\n");
    int nRes = 0;
    size_t stacksize;
    int priority_max;
    int priority_min;
    struct sched_param param;
    pthread_t   pth;
    /*sigset_t    set;*/
    ENTRYParms_t entry_parm;

    vthread_t* th       = (vthread_t*)malloc(sizeof(vthread_t));
    th->u.attr.detach   = ATASK_CREATE_DETACHED;
    th->u.attr.scope    = ATASK_SCOPE_SYSTEM;
    th->u.attr.policy   = _policy;
    th->u.attr.priority = _priority;

    pthread_attr_t p_attr;
    pthread_attr_init(&p_attr);

    nRes = th->u.attr.detach == ATASK_CREATE_JOINABLE ?
        pthread_attr_setdetachstate(&p_attr, PTHREAD_CREATE_JOINABLE) :
        pthread_attr_setdetachstate(&p_attr, PTHREAD_CREATE_DETACHED);
    if (nRes != 0)
    {
        VL_ERR("pthread_attr_setdetachstate() nRes:[%d]\n", nRes);
        //pthread_attr_destroy(&p_attr);
        //free(th);
        //th = 0;
        //goto EXIT;
    }

    //nRes = th->u.attr.scope == ATASK_SCOPE_PROCESS ?
    //    pthread_attr_setscope(&p_attr, PTHREAD_SCOPE_PROCESS) : 
    //    pthread_attr_setscope(&p_attr, PTHREAD_SCOPE_SYSTEM);
    //if (nRes != 0)
    //{
    //    VL_ERR("pthread_attr_setscope() nRes:[%d]\n", nRes);
    //    //pthread_attr_destroy(&p_attr);
    //    //free(th);
    //    //th = 0;
    //    //goto EXIT;
    //}

    if (th->u.attr.policy == ATASK_SCHED_RR)
    {
        nRes = pthread_attr_setschedpolicy(&p_attr, SCHED_RR);
        priority_max = sched_get_priority_max(SCHED_RR);
        priority_min = sched_get_priority_min(SCHED_RR);
    }
    else if (th->u.attr.policy == ATASK_SCHED_FIFO)
    {
        nRes = pthread_attr_setschedpolicy(&p_attr, SCHED_FIFO);
        priority_max = sched_get_priority_max(SCHED_FIFO);
        priority_min = sched_get_priority_min(SCHED_FIFO);
    }
    else
    {
        nRes = pthread_attr_setschedpolicy(&p_attr, SCHED_OTHER);
        priority_max = sched_get_priority_max(SCHED_OTHER);
        priority_min = sched_get_priority_min(SCHED_OTHER);
    }

    if (nRes != 0)
    {
        VL_ERR("pthread_attr_setschedpolicy() nRes:[%d]\n", nRes);
        //pthread_attr_destroy(&p_attr);
        //free(th);
        //th = 0;
        //goto EXIT;
    }

    if (th->u.attr.priority < IDLE || th->u.attr.priority > HIGHEST)
    {
        th->u.attr.priority = NORMAL;
    }

    pthread_attr_getstacksize(&p_attr, &stacksize);
    if (_stack_size > stacksize)
    {
        nRes = pthread_attr_setstacksize(&p_attr, _stack_size);
        if (nRes != 0)
        {
            VL_ERR("pthread_attr_setstaticsize() nRes:[%d]\n", nRes);
            pthread_attr_destroy(&p_attr);
            free(th);
            th = 0;
            goto EXIT;
        }
    }

    pthread_attr_getschedparam(&p_attr, &param);
    param.sched_priority = 
        priority_min + 
        ((priority_max - priority_min) * (th->u.attr.priority) / HIGHEST); /* sched_priority will be the priority of the thread */
	//(HIGHEST - th->u.attr.priority)
    nRes = pthread_attr_setschedparam(&p_attr, &param);
//printf("\nnRes=%d--param.sched_priority=%d--priority_min=%d--priority_max=%d--th->u.attr.priority=%d--HIGHEST=%d\n",nRes,param.sched_priority,priority_min,priority_max,th->u.attr.priority,HIGHEST);
    if (nRes != 0)
    {
        VL_ERR("pthread_attr_setschedparam() nRes:[%d]\n", nRes);
        //pthread_attr_destroy(&p_attr);
        //free(th);
        //th = 0;
        //goto EXIT;
    }

    if (_task_name != 0)
    {
        strncpy(th->name, _task_name, ATASKNAME_LEN);
        th->name[ATASKNAME_LEN] = 0;
    }
    else
    {
        th->name[0] = 0;
    }

    /*
    sigemptyset(&set);
    nRes = pthread_sigmask(SIG_UNBLOCK, &set, NULL);
    if (nRes != 0)
    {
        VL_ERR("pthread_sigmask() nRes:[%d]\n", nRes);
        pthread_attr_destroy(&p_attr);
        free(th);
        th = 0;
        goto EXIT;
    }*/

    th->entry.func = _entry;
    th->entry.arg  = _argv;
    nRes = pthread_create(&pth, &p_attr, THREADEntry, (void*)&th->entry);
    if (nRes != 0)
    {
        VL_ERR("first try pthread_create() failure nRes:[%d]\n", nRes);
        pthread_attr_destroy(&p_attr);
		nRes = pthread_create(&pth, NULL, THREADEntry, (void*)&th->entry);
		if (nRes != 0)
		{
			VL_ERR("final try fpthread_create() failur nRes:[%d]\n", nRes);
	        free(th);
        	th = 0;
        	goto EXIT;
		}
    }
    th->id = pth;

EXIT:
    VL_LOGV("leave...\n");
    return (Task_t)th;
}

//******************************************************************************
//
// Function Name:    ATASK_Destroy
//
// Description:        This function destroys a task
//
// Notes:
//
//******************************************************************************

void ATASK_Destroy(
    Task_t        t
    )
{
    VL_LOGV("enter...\n");
    pvthread_t pth = (pvthread_t)t;
    assert(pth != 0);
    VL_LOGV( "ATASK_Destroy: ID\t= %d\n", pth->id);
#if 0	//pthread_cancel is not supported by android linux due to it will invoke bigger C lib.
    pthread_cancel(pth->id);
#else
	//TO DO: add alternative C call.
	assert(0);//no suggestion to use this asyn task destroy function but let task exit itself instead.
#endif
    free(pth);
    VL_LOGV("leave...\n");
}


//******************************************************************************
//
// Function Name:    ATASK_ChangePriority
//
// Description:        This function changes the selected task's priority, returning
//                    the previous task priority.
//
// Notes:
//
//******************************************************************************
                                        // returns previous task priority
TPriority_t ATASK_ChangePriority(        // change task priority
    Task_t        t,                            // task pointer
    TPriority_t	  new_priority            // new task priority
    )
{
    VL_LOGV("enter...\n");
    struct sched_param param;
    int policy;
    int priority_max;
    int priority_min;
    pvthread_t pth = (pvthread_t)t;
    assert(pth != 0);

    TPriority_t old_priority = (TPriority_t)pth->u.attr.policy;

    pth->u.attr.priority = new_priority;
    pthread_getschedparam(pth->id, &policy, &param);
    priority_max = sched_get_priority_max(SCHED_RR);
    priority_min = sched_get_priority_min(SCHED_RR);
    param.sched_priority = 
            priority_min + 
            ((priority_max - priority_min) * (HIGHEST - pth->u.attr.priority) / HIGHEST);

    pthread_setschedparam(pth->id, SCHED_RR, &param);

    VL_LOGV("leave...\n");
    return old_priority;
}

//******************************************************************************
//
// Function Name:    ATASK_Suspend
//
// Description:  This function suspends a task.
//
// Notes:
//
//******************************************************************************
void ATASK_Suspend(                        // Suspend a task
    Task_t        t                            // Task to suspend
    )
{
	vthread_t* th = (vthread_t*)t; 
	pthread_kill(th->id,SUSPEND_SIG);
}

//******************************************************************************
//
// Function Name:    ATASK_Resume
//
// Description:        This function resumes a suspended task.
//
// Notes:
//
//******************************************************************************

void ATASK_Resume(                        // Resume a task
    Task_t        t                            // Task to resume
    )
{
	vthread_t* th = (vthread_t*)t; 
	pthread_kill(th->id,RESUME_SIG);
}

void ATASK_Sleep(						// suspend task, until timeout
	Ticks_t			timeout						// task sleep timeout
	)
{
    usleep(timeout * 1000);
}

//******************************************************************************
//
// Function Name:    ATASK_GetThreadName
//
// Description:        This returns the ASCII name of a thread in question.  A thread
//                    is defined as either a HISR or task.
//
// Notes:
//
//******************************************************************************

OSStatus_t ATASK_GetThreadName(            // get ASCII name of thread
    Task_t    t,                            // task or HISR pointer
    TName_t p_name                        // location to store the ASCII name
    )
{
    VL_LOGV("enter...\n");
    pvthread_t pth = (pvthread_t)t;
    assert(pth != 0);
    assert(p_name);

    OSStatus_t ret = OSSTATUSSuccess;

    strncpy(p_name, pth->name, ATASKNAME_LEN);

    VL_LOGV("leave...\n");
    return( ret );
}

//******************************************************************************
//
// Function Name:    ATASK_GetTaskName
//
// Description:    This returns the ASCII name of task in queation.  If valid
//                no task is found, then name is set to empty string.
//                
//
// Notes:
//
//******************************************************************************
OSStatus_t ATASK_GetTaskName(            // get ASCII name of task
    Task_t        t,                            // task pointer
    TName_t     p_name                        // location to store the ASCII name
    )
{
    VL_LOGV("enter...\n");
    pvthread_t pth = (pvthread_t)t;
    assert(pth != 0);
    assert(p_name);

    OSStatus_t ret = OSSTATUSSuccess;

    strncpy(p_name, pth->name, ATASKNAME_LEN);

    VL_LOGV("leave...\n");
    return( ret );
}


//******************************************************************************
//
// Function Name:    ATASK_GetHISRName
//
// Description:    This returns the ASCII name of HISR in queation.  If valid
//                no HISR is found, then name is set to empty string.
//                
//
// Notes:
//
//******************************************************************************
OSStatus_t ATASK_GetHISRName(            // get ASCII name of HISR
    Task_t    t,                            // HISR pointer
    TName_t p_name                        // location to store the ASCII name
    )
{
    VL_LOGV("enter...\n");
    pvthread_t pth = (pvthread_t)t;
    assert(pth != 0);
    assert(p_name);

    OSStatus_t ret = OSSTATUSSuccess;

    strncpy(p_name, pth->name, ATASKNAME_LEN);

    VL_LOGV("leave...\n");
    return( ret );
}
