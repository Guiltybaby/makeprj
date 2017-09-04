/*******************************************************************************
 * @file vsenaphoreset.h
 * @brief declare a set of methods to access semaphore set.
 * @author  zangxiaohua
 * @date    20090427
 * @version 1.0
 * @history
 * 1. created on 
 * 2. 
*******************************************************************************/
#ifndef ___VSEMAPHORESET_H___
#define ___VSEMAPHORESET_H___

#include "aostypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***************************************
 * Global Macros
***************************************/

/***************************************
 * Global Typedefs
***************************************/

/***************************************
 * Global Function Prototypes
***************************************/

/** asemaset_create
 * @brief  Creates a new semaphore or obtains the semaphore key of an existing semaphore.
 * @param  [IN] _name is semaphoreset name.
 * @param  [IN] _sems is the number of available semaphores.
 * @param  [IN] _vals is a set of the init val of each semaphore.
 * @exception NULL
 * @return the newly-created semaphoreset identification.
 */
asemasetid_t asemaset_create(char* _name, int _sems, unsigned short* _vals);

/** asemaset_delete
 * @brief  Delete the specified semaphore set, the count the semaphore set is referenced reduce 1 per launching until the count equal 0, then destroy the semaphore set.
 * @patam  [IN] _s the specified semaphore set.
 * @exception NULL
 * @return    NULL
 */
void asemaset_destroy(asemasetid_t _s);

/** asemaset_acquire
 * @brief  wait to obtain a semaphore
 * @param  [IN] _s the specified semaphore set.
 * @param  [IN] _which which semaphore of the semaphore set is operated.
 * @param  [IN] _how   the number need to be acquired.
 * @exception NULL
 * @return    A return value of OSSTATUSFailure represents the <asemaset_acquire> operation failed, OSSTATUSSuccess sucessful.
 */
aosstatus_t asemaset_acquire(asemasetid_t _s, int _which, int _how);

/** asemaset_acquire_timed
 * @brief  wait to obtain a semaphore
 * @param  [IN] _s the specified semaphore set.
 * @param  [IN] _which which semaphore of the semaphore set is operated.
 * @param  [IN] _how   the number need to be acquired.
 * @param  [IN] _ms    timeout to acquire semaphore (millisecond).
 * @exception NULL
 * @return    A return value of OSSTATUSFailure represents the <asemaset_acquire> operation failed, OSSTATUSSuccess sucessful.
 */
aosstatus_t asemaset_acquire_timed(asemasetid_t _s, int _which, int _how, unsigned long _ms);

/** asemaset_release
 * @brief  Release a semaphore
 * @param  [IN] _s the specified semaphore set.
 * @param  [IN] _which which semaphore of the semaphore set is operated.
 * @param  [IN] _how   the number need to be released.
 * @exception NULL
 * @return    A return value of OSSTATUSFailure represents the <asemaset_acquire> operation failed, OSSTATUSSuccess sucessful.
 */
aosstatus_t asemaset_release(asemasetid_t _s, int _which, int _how);

int asemaset_getval(asemasetid_t _s, int _which);

#ifdef __cplusplus
}
#endif

#endif /// ___VSEMAPHORESET_H___
