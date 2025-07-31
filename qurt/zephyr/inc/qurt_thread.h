/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QURT_THREAD_H
#define QURT_THREAD_H

#include "qurt_types.h"
#include "qurt_timer.h"
#include <stringl.h>

/** @addtogroup thread_macros
@{ */
// TODO: Only Legacy behaviour is supported now
#define QURT_THREAD_ATTR_CREATE_LEGACY 0
#define QURT_THREAD_ATTR_CREATE_JOINABLE 1
#define QURT_THREAD_ATTR_CREATE_DETACHED 2
/** @} */ /* end_addtogroup thread_macros */

#define QURT_THREAD_ATTR_NAME_MAXLEN 16      /**< Maximum name length. */
#define QURT_THREAD_ATTR_TCB_PARTITION_RAM 0 /**< Creates threads in RAM/DDR. */
#define QURT_THREAD_ATTR_TCB_PARTITION_TCM 1 /**< Creates threads in TCM. */
/** @cond rest_reg_dist */
/** @addtogroup thread_macros
@{ */
#define QURT_THREAD_ATTR_TCB_PARTITION_DEFAULT QURT_THREAD_ATTR_TCB_PARTITION_RAM /**< Backward compatibility. */
#define QURT_THREAD_ATTR_PRIORITY_MIN 0
#define QURT_THREAD_ATTR_ASID_DEFAULT 0                                     /**< ASID. */
#define QURT_THREAD_ATTR_AFFINITY_DEFAULT (-1)                              /**< Affinity. */
#define QURT_THREAD_ATTR_AUTOSTACK_DEFAULT 0                                /**< Default autostack v2 disabled thread*/
#define QURT_THREAD_ATTR_TIMETEST_ID_DEFAULT (-2)                           /**< Timetest ID. */
#define QURT_THREAD_ATTR_STID_DEFAULT 0                                     /**< STID. */
/** @} */                                                                   /* end_addtogroup thread_macros */
/** @endcond*/

typedef unsigned int qurt_thread_t;

/** @cond rest_reg_dist  */
/** Thread attributes */
typedef struct _qurt_thread_attr {

    char name[QURT_THREAD_ATTR_NAME_MAXLEN]; /**< Thread name. */
    unsigned char tcb_partition;             /**< Indicates whether the thread TCB resides in RAM or
                                                  on chip memory (in other words, TCM). */
    unsigned char stid;                      /**< Software thread ID used to configure the stid register
                                                  for profiling pusposes. */
    unsigned short priority;                 /**< Thread priority. */
    unsigned char autostack : 1;             /**< Autostack v2 enabled thread. */
    unsigned char reserved : 7;              /**< Reserved bits. */
    unsigned char bus_priority;              /**< Internal bus priority. */
    unsigned short timetest_id;              /**< Timetest ID. */
    unsigned int stack_size;                 /**< Thread stack size. */
    void *stack_addr;                        /**< Pointer to the stack address base, the range of the stack is
                                                  (stack_addr, stack_addr+stack_size-1). */
    unsigned short detach_state;             /**< Detach state of the thread */
} qurt_thread_attr_t;
/** @endcond */

/**@ingroup func_qurt_thread_attr_set_stack_size
  @xreflabel{sec:set_stack_size}
  Sets the thread stack size attribute.\n
  Specifies the size of the memory area to use for a call stack of a thread.

  The thread stack address (Section @xref{sec:set_stack_addr}) and stack size specify the memory area used as a
  call stack for the thread. The user is responsible for allocating the memory area used for
  the stack.

  @datatypes
  #qurt_thread_attr_t

  @param[in,out] attr Pointer to the thread attribute structure.
  @param[in] stack_size Size (in bytes) of the thread stack.

  @return
  None.

  @dependencies
  None.
*/
static inline void qurt_thread_attr_set_stack_size(qurt_thread_attr_t *attr, unsigned int stack_size)
{
    attr->stack_size = stack_size;
}

/**@ingroup func_qurt_thread_attr_set_stack_addr
  @xreflabel{sec:set_stack_addr}
  Sets the thread stack address attribute. \n
  Specifies the base address of the memory area to use for a call stack of a thread.

  stack_addr must contain an address value that is 8-byte aligned.

  The thread stack address and stack size (Section @xref{sec:set_stack_size}) specify the memory area used as a
  call stack for the thread. \n
  @note1hang The user is responsible for allocating the memory area used for the thread
             stack. The memory area must be large enough to contain the stack that the thread
             creates.

  @datatypes
  #qurt_thread_attr_t

  @param[in,out] attr Pointer to the thread attribute structure.
  @param[in] stack_addr  Pointer to the 8-byte aligned address of the thread stack.

  @return
  None.

  @dependencies
  None.
*/
static inline void qurt_thread_attr_set_stack_addr(qurt_thread_attr_t *attr, void *stack_addr)
{
    attr->stack_addr = stack_addr;
}

/**@ingroup func_qurt_thread_attr_set_detachstate
  Sets the thread detach state with which thread is created.
  Thread detach state is either joinable or detached; specified by the following values:
  - #QURT_THREAD_ATTR_CREATE_JOINABLE  \n
  - #QURT_THREAD_ATTR_CREATE_DETACHED  \n

  When a detached thread is created (QURT_THREAD_ATTR_CREATE_DETACHED), its thread
  ID and other resources are reclaimed as soon as the thread exits. When a joinable thread
  is created (QURT_THREAD_ATTR_CREATE_JOINABLE), it is assumed that some
  thread waits to join on it using a qurt_thread_join() call.
  By default, detached state is QURT_THREAD_ATTR_CREATE_LEGACY
  If detached state is QURT_THREAD_ATTR_CREATE_LEGACY then other
  thread can join before thread exits but it will not wait other thread to join.

  @note1hang For a joinable thread (QURT_THREAD_ATTR_CREATE_JOINABLE), it is very
             important that some thread joins on it after it terminates, otherwise
             the resources of that thread are not reclaimed, causing memory leaks.

  @datatypes
  #qurt_thread_attr_t

  @param[in,out] attr Pointer to the thread attribute structure.
  @param[in] detachstate Thread detach state.

  @return
  None.

  @dependencies
  None.
*/
static inline void qurt_thread_attr_set_detachstate(qurt_thread_attr_t *attr, unsigned short detachstate)
{
    if (detachstate == QURT_THREAD_ATTR_CREATE_JOINABLE || detachstate == QURT_THREAD_ATTR_CREATE_DETACHED) {
        attr->detach_state = detachstate;
    }
}

/**@ingroup func_qurt_thread_attr_set_priority
  Sets the thread priority to assign to a thread.
  Thread priorities are specified as numeric values in the range 1 to 254, with 1 representing
  the highest priority.
  Priority 0 and 255  are internally used by the kernel for special purposes.

  @datatypes
  #qurt_thread_attr_t

  @param[in,out] attr Pointer to the thread attribute structure.
  @param[in] priority Thread priority.

  @return
  None.

  @dependencies
  None.
*/
static inline void qurt_thread_attr_set_priority(qurt_thread_attr_t *attr, unsigned short priority)
{
    attr->priority = priority;
}

/**@ingroup func_qurt_thread_attr_set_name
  Sets the thread name attribute.\n
  This function specifies the name to use by a thread.
  Thread names identify a thread during debugging or profiling.
  Maximum name length is 16 charactes  \n
  @note1hang Thread names differ from the kernel-generated thread identifiers used to
  specify threads in the API thread operations.

  @datatypes
  #qurt_thread_attr_t

  @param[in,out] attr Pointer to the thread attribute structure.
  @param[in] name     Pointer to the character string containing the thread name.

  @return
  None.

  @dependencies
  None.
*/
static inline void qurt_thread_attr_set_name(qurt_thread_attr_t *attr, char *name)
{
    strlcpy(attr->name, name, QURT_THREAD_ATTR_NAME_MAXLEN);
    attr->name[QURT_THREAD_ATTR_NAME_MAXLEN - 1] = 0;
}

/**@ingroup func_qurt_thread_get_thread_id
  Gets the thread identifier of the thread with the matching name in the same process
  of the caller.

  @datatypes
  #qurt_thread_t

  @param[out] thread_id Pointer to the thread identifier.
  @param[in]  name      Pointer to the name of the thread.

  @return
  #QURT_EINVALID -- No thread with matching name in the process of the caller \n
  #QURT_EOK      -- Success

  @dependencies
  None.
 */
int qurt_thread_get_thread_id(qurt_thread_t *thread_id, char *name);

/**@ingroup func_qurt_thread_attr_init
  Initializes the structure used to set the thread attributes when a thread is created.
  After an attribute structure is initialized, Explicity set the individual attributes in the structure
  using the thread attribute operations.

  The initialize operation sets the following default attribute values: \n
  - Name -- NULL string \n
  - TCB partition -- QURT_THREAD_ATTR_TCB_PARTITION_DEFAULT
  - Priority -- QURT_THREAD_ATTR_PRIORITY_DEFAULT \n
  - Autostack -- QURT_THREAD_ATTR_AUTOSTACK_DEFAULT \n
  - Bus priority -- QURT_THREAD_ATTR_BUS_PRIO_DEFAULT \n
  - Timetest ID -- QURT_THREAD_ATTR_TIMETEST_ID_DEFAULT \n
  - stack_size -- 0 \n
  - stack_addr -- NULL \n
  - detach state -- #QURT_THREAD_ATTR_CREATE_LEGACY \n
  - STID -- #QURT_THREAD_ATTR_STID_DEFAULT

  @datatypes
  #qurt_thread_attr_t

  @param[in,out] attr Pointer to the thread attribute structure.

  @return
  None.

  @dependencies
  None.
*/
void qurt_thread_attr_init(qurt_thread_attr_t *attr);

/**@ingroup func_qurt_thread_attr_get
  Gets the attributes of the specified thread.

  @datatypes
  #qurt_thread_t \n
  #qurt_thread_attr_t

  @param[in]  thread_id Thread identifier.
  @param[out] attr      Pointer to the destination structure for thread attributes.

  @return
  #QURT_EOK -- Success. \n
  #QURT_EINVALID -- Invalid argument.

  @dependencies
  None.
 */
int qurt_thread_attr_get(qurt_thread_t thread_id, qurt_thread_attr_t *attr);

/**@ingroup func_qurt_thread_exit
   @xreflabel{sec:qurt_thread_exit}
   Stops the current thread, awakens threads joined to it, then destroys the stopped
   thread.

   Threads that are suspended on the current thread (by performing a thread join
   Section @xref{sec:thread_join}) are awakened and passed a user-defined status value
   that indicates the status of the stopped thread.

   @note1hang Exit must be called in the context of the thread to stop.

   @param[in]   status User-defined thread exit status value.

   @return
   None.

   @dependencies
   None.
 */
void qurt_thread_exit(int status);

/**@ingroup func_qurt_thread_create
  @xreflabel{hdr:qurt_thread_create}
  Creates a thread with the specified attributes, and makes it executable.

  @datatypes
  #qurt_thread_t \n
  #qurt_thread_attr_t

  @param[out]  thread_id    Returns a pointer to the thread identifier if the thread was
                             successfully created.
  @param[in]   attr         Pointer to the initialized thread attribute structure that specifies
                             the attributes of the created thread.
  @param[in]   entrypoint   C function pointer, which specifies the main function of a thread.
  @param[in]   arg          Pointer to a thread-specific argument structure


  @return
  #QURT_EOK -- Thread created.
  #QURT_EFAILED -- Thread not created.
  #QURT_EINVALID -- Invalid thread attributes passed.

  @dependencies
  None.
 */
int qurt_thread_create(qurt_thread_t *thread_id, qurt_thread_attr_t *attr, void (*entrypoint)(void *), void *arg);

/**@ingroup func_qurt_thread_join
   @xreflabel{sec:thread_join}
   Waits for a specified thread to finish; the specified thread is another thread within
   the same process.
   The caller thread is suspended until the specified thread exits. When the unspecified thread
   exits, the caller thread is awakened. \n
   @note1hang If the specified thread has already exited, this function returns immediately
              with the result value #QURT_ENOTHREAD. \n
   @note1cont Two threads cannot call qurt_thread_join to wait for the same thread to finish.
              If this occurs, QuRT generates an exception (see Section @xref{sec:exceptionHandling}).

   @param[in]   tid     Thread identifier.
   @param[out]  status  Destination variable for thread exit status. Returns an application-defined
                        value that indicates the termination status of the specified thread.

   @return
   #QURT_ENOTHREAD -- Thread has already exited. \n
   #QURT_EOK -- Thread successfully joined with valid status value.

   @dependencies
   None.
 */
int qurt_thread_join(unsigned int tid, int *status);

/**@ingroup func_qurt_thread_get_name
  Gets the thread name of current thread.\n
  Returns the thread name of the current thread.
  Thread names are assigned to threads as thread attributes,
  see qurt_thread_attr_set_name().
  Thread names identify a thread during debugging or profiling.

  @param[out] name Pointer to a character string, which specifies the address
              where the returned thread name is stored.
  @param[in] max_len Maximum length of the character string that can be returned.

  @return
  None.

  @dependencies
  None.
*/
void qurt_thread_get_name(char *name, unsigned char max_len);

/**@ingroup func_qurt_thread_get_id
   Gets the identifier of the current thread.\n
   Returns the thread identifier for the current thread.

   @return
   Thread identifier -- Identifier of the current thread.

   @dependencies
   None.
 */
qurt_thread_t qurt_thread_get_id(void);

/**@ingroup func_qurt_sleep
  Suspends the current thread for the specified amount of time.

  @param[in] duration  Duration (in microseconds) for which the thread is suspended.

  @return
  None.

  @dependencies
  None.
 */
void qurt_thread_sleep(TickType_t sleep_time);

void qurt_thread_sleep_ms(uint32_t ms);

/**@ingroup func_qurt_thread_get_priority
   Gets the priority of the specified thread. \n
   Returns the thread priority of the specified thread.\n
   Thread priorities are specified as numeric values in a range as large as 1 through 254, with lower
   values representing higher priorities. 1 represents the highest possible thread priority. \n
   Priority 0 and 255 are internally used by the kernel for special purposes.

   @note1hang QuRT can be configured to have different priority ranges.

   @datatypes
   #qurt_thread_t

   @param[in]  threadid Thread identifier.

   @return
   -1 -- Invalid thread identifier. \n
   1 through 254 -- Thread priority value.

   @dependencies
   None.
 */
int qurt_thread_get_priority(qurt_thread_t threadid);

/**@ingroup func_qurt_thread_set_priority
   Sets the priority of the specified thread.\n
   Thread priorities are specified as numeric values in a range as large as 1 through 254, with lower
   values representing higher priorities. 1 represents the highest possible thread priority.
   Priority 0 and 255  are internally used by the kernel  for special purposes.

   @note1hang QuRT can be configured to have different priority ranges. For more
              information, see Section @xref{sec:AppDev}.

   @datatypes
   #qurt_thread_t

   @param[in] threadid  Thread identifier.
   @param[in] newprio   New thread priority value.

   @return
   0 -- Priority successfully set. \n
   -1 -- Invalid thread identifier. \n

   @dependencies
   None.
 */
int qurt_thread_set_priority(qurt_thread_t threadid, unsigned short newprio);

#define configSTACK_DEPTH_TYPE uint16_t
typedef qurt_thread_t TaskHandle_t;
typedef void (*TaskFunction_t)(void *);

BaseType_t nt_qurt_thread_create(
    TaskFunction_t pxTaskCode,
    const char *const pcName, /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
    const configSTACK_DEPTH_TYPE usStackDepth, void *const pvParameters, UBaseType_t uxPriority,
    TaskHandle_t *const pxCreatedTask);

TaskHandle_t nt_qurt_thread_get_id(void);

#endif /* QURT_THREAD_H */
