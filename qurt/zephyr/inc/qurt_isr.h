/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef QAL_ISR_H
#define QAL_ISR_H

#include "zephyr/kernel.h"
#include "qurt_thread.h"

#define QURT_INT_NON_DELAYED_ACK           0
#define QURT_INT_DELAYED_ACK               1
#define QURT_INT_ACK_DEFAULT               QURT_INT_NON_DELAYED_ACK
#define QURT_INT_TRIGGER_USE_DEFAULT       0xff

/**@ingroup func_qurt_isr_create
  Creates an ISR thread with the specified attributes, and makes it executable.

  @datatypes
  #qurt_thread_t \n
  #qurt_thread_attr_t

  @param[out]  thread_id  Returns a pointer to the thread identifier
                          if the thread was successfully created.
  @param[in]   attr       Pointer to the initialized thread attribute structure
                          that specifies the attributes of the created thread.

  @return
  #QURT_EVAL    -- Invalid arguments
  #QURT_EOK -- Thread created. \n
  #QURT_EFAILED -- Thread not created.

  @dependencies
  None.
 */
int qurt_isr_create (qurt_thread_t *thread_id, qurt_thread_attr_t *pAttr);

/**@ingroup func_qurt_isr_register2
  Registers an Interrupt Service Routine to an ISR thread. ISR callback with
  the specified attributes.
  The interrupt is enabled when this function returns success.

  @datatypes
   qurt_thread_t

  @param[in]   isr_thread_id ISR thread ID, returned from qurt_isr_create()
  @param[in]   int_num  Encoded interrupt number, for RISCV arch check
               RISCV_ISR_NUM_CLINT_LOCAL & RISCV_ISR_NUM_PLIC_GLOBAL for more info
  @param[in]   prio     Priority of the ISR
  @param[in]   flags    Defines ACK type. Values : \n
                             QURT_INT_NON_DELAYED_ACK - ISR is acknowledged by
			                       the interrupt handle routine in the Kernel.
                             QURT_INT_DELAYED_ACK     - Client chooses to acknowledge.
  @param[in]   int_type Notifies it to registered function. Values: \n
                             - QURT_INT_TRIGGER_USE_DEFAULT
                             - QURT_INT_TRIGGER_LEVEL_HIGH
                             - QURT_INT_TRIGGER_LEVEL_LOW
                             - QURT_INT_TRIGGER_RISING_EDGE
                             - QURT_INT_TRIGGER_FALLING_EDGE
                             - QURT_INT_TRIGGER_DUAL_EDGE
  @param[in]   isr  Interrupt Service Routine with proto type
                    void isr(void *arg, int int_num)
  @param[in]   arg  1st argument of the ISR when it is called to service the
                    interrupt

  @return
   QURT_EOK          -- Successfully registered the ISR for the interrupt
   QURT_EINT         -- Interrupt not configured
   QURT_EINVALID     -- Invalid Thread ID
   QURT_EDISABLED    -- The feature is disabled
   QURT_EDUPLICATE   -- Interrupt is already registered

  @dependencies
   Thread ID should be created using qurt_isr_create()
 */
int qurt_isr_register2 (qurt_thread_t isr_thread_id, int int_num,
                        unsigned short prio, unsigned short flags,
                        unsigned int int_type,
                        void (*isr) (void *, int), void *arg);

/**@ingroup func_qurt_isr_deregister2
  De-registers the ISR for the specified interrupt.
  The interrupt is disabled when this function returns success.

  @param[in] int_num Encoded interrupt number, for RISCV arch check
             RISCV_ISR_NUM_CLINT_LOCAL & RISCV_ISR_NUM_PLIC_GLOBAL for more info

  @return
   QURT_EOK            -- ISR deregistered successfully
   QURT_ENOREGISTERED  -- Interrupt with int_num is not registered

  @dependencies
  None.
 */
int qurt_isr_deregister2 (int int_num);

/**@ingroup func_qurt_isr_delete
   ISR thread will exit and releases Kernel resources

   @note1hang   The ISR thread shouldn't be actively processing interrupts,
                otherwise the call will fail and return an error.

   @param[in]   thread-id of the ISR thread that needs to be deleted.

   @return
    QURT_ENOTALLOWED   -- ISR thread is processing an interrupt
    QURT_EINVALID      -- Invalid ISR thread ID
    QURT_EOK           -- Success

   @dependencies
   Thread ID should be created using qurt_isr_create()
 */
int qurt_isr_delete (qurt_thread_t isr_tid);

/**@ingroup func_qurt_interrupt_disable
  Disables an interrupt with its interrupt number.

  The interrupt must be registered prior to calling this function.
  After qurt_interrupt_disable() returns, interrupt cannot be send to the
  core, until qurt_interrupt_enable() is called for the same interrupt.

  @param[in] int_num Encoded interrupt number, for RISCV arch check
             RISCV_ISR_NUM_CLINT_LOCAL & RISCV_ISR_NUM_PLIC_GLOBAL for more info

  @return
  #QURT_EOK  -- Interrupt successfully disabled.\n
  #QURT_EINT -- Invalid interrupt number.\n
  #QURT_ENOTALLOWED -- Interrupt is locked.\n
  #QURT_EVAL -- Interrupt is not registered.

  @dependencies
  None.
*/
unsigned int qurt_interrupt_disable(int int_num);

/**@ingroup func_qurt_interrupt_enable
  Enables an interrupt with its interrupt number.

  The interrupt must be registered prior to calling this function.

  @param[in] int_num Encoded interrupt number, for RISCV arch check
             RISCV_ISR_NUM_CLINT_LOCAL & RISCV_ISR_NUM_PLIC_GLOBAL for more info

  @return
  #QURT_EOK -- Interrupt successfully enabled.\n
  #QURT_EINT -- Invalid interrupt number.\n
  #QURT_ENOTALLOWED -- Interrupt is locked.\n
  #QURT_EVAL -- Interrupt is not registered.

  @dependencies
  None.

*/
unsigned int qurt_interrupt_enable(int int_num);


#define qurt_isr_register_3(irq_p, isr_p)  IRQ_CONNECT(irq_p, 1, isr_p, NULL, 0)

#endif