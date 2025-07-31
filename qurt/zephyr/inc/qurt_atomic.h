/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QAL_ATOMIC_H
#define QAL_ATOMIC_H

/*=============================================================================
                        CONSTANTS AND MACROS
=============================================================================*/

/*=============================================================================
                        FUNCTIONS
=============================================================================*/
/**@ingroup func_qurt_atomic_set
  Sets the atomic variable with the specified value.

  @note1hang The function retries until load lock and store conditional
             is successful.

  @param[in,out]  target Pointer to the atomic variable.
  @param[in]      value  Value to set.

  @return
  Value successfuly set.

  @dependencies
  None.
*/
unsigned int qurt_atomic_set(unsigned int *target, unsigned int value);

/**@ingroup func_qurt_atomic_and
  Bitwise AND operation of the atomic variable with mask.

  @note1hang The function retries until load lock and store conditional
             is successful.

  @param[in,out]  target Pointer to the atomic variable.
  @param[in]      mask   Mask for bitwise AND.

  @return
  None

  @dependencies
  None.
*/
void qurt_atomic_and(unsigned int *target, unsigned int mask);

/**@ingroup func_qurt_atomic_or
  Bitwise OR operation of the atomic variable with mask.

  @note1hang The function retries until load lock and store conditional
             is successful.

  @param[in,out]  target Pointer to the atomic variable.
  @param[in]      mask   Mask for bitwise OR.

  @return
  None.

  @dependencies
  None.
*/
void qurt_atomic_or(unsigned int *target, unsigned int mask);

/**@ingroup func_qurt_atomic_xor
  Bitwise XOR operation of the atomic variable with mask.

  @note1hang The function retries until load lock and store conditional
             is successful.

  @param[in,out]  target Pointer to the atomic variable.
  @param[in]      mask   Mask for bitwise XOR.

  @return
  None.

  @dependencies
  None.
*/
void qurt_atomic_xor(unsigned int *target, unsigned int mask);

/**@ingroup func_qurt_atomic_set_bit
  Sets a bit in the atomic variable at a specified position.

  @note1hang The function retries until load lock and store conditional
             is successful.

  @param[in,out]  target Pointer to the atomic variable.
  @param[in]      bit    Bit position to set.

  @return
  None.

  @dependencies
  None.
*/
void qurt_atomic_set_bit(unsigned int *target, unsigned int bit);

/**@ingroup func_qurt_atomic_clear_bit
  Clears a bit in the atomic variable at a specified position.

  @note1hang The function retries until load lock and store conditional
             is successful.

  @param[in,out]  target Pointer to the atomic variable.
  @param[in]      bit    Bit position to clear.

  @return
  None.

  @dependencies
  None.
*/
void qurt_atomic_clear_bit(unsigned int *target, unsigned int bit);

/**@ingroup func_qurt_atomic_add
  Adds an integer to atomic variable.

  @note1hang The function retries until load lock and store conditional
             is successful.

  @param[in,out]  target Pointer to the atomic variable.
  @param[in]      v      An integer value to add.

  @return
  None.

  @dependencies
  None.
*/
void qurt_atomic_add(unsigned int *target, unsigned int v);

/**@ingroup func_qurt_atomic_sub
  Subtracts an integer from an atomic variable.

  @note1hang The function retries until load lock and store conditional
             is successful.

  @param[in,out]  target Pointer to the atomic variable.
  @param[in]      v      An integer value to subtract.

  @return
  None.

  @dependencies
  None.
*/
void qurt_atomic_sub(unsigned int *target, unsigned int v);

/**@ingroup func_qurt_atomic_inc
  Increments an atomic variable by one.

  @note1hang The function retries until load lock and store conditional
             is successful.

  @param[in,out]  target Pointer to the atomic variable.

  @return
  None.

  @dependencies
  None.
*/
void qurt_atomic_inc(unsigned int *target);

/**@ingroup func_qurt_atomic_inc_return
  Increments an atomic variable by one.

  @note1hang The function retries until load lock and store conditional
             is successful.

  @param[in,out]  target Pointer to the atomic variable.

  @return
  Incremented value.

  @dependencies
  None.
*/
unsigned int qurt_atomic_inc_return(unsigned int *target);

/**@ingroup func_qurt_atomic_dec
  Decrements an atomic variable by one.

  @note1hang The function retries until load lock and store conditional
             is successful.

  @param[in,out]  target Pointer to the atomic variable.

  @return
  None.

  @dependencies
  None.
*/
void qurt_atomic_dec(unsigned int *target);

/**@ingroup func_qurt_atomic_dec_return
  Decrements an atomic variable by one.

  @note1hang The function retries until load lock and store conditional
             is successful.

  @param[in,out]  target Pointer to the atomic variable.

  @return
  Decremented value.

  @dependencies
  None.
*/
unsigned int qurt_atomic_dec_return(unsigned int *target);

/**@ingroup func_qurt_atomic_compare_and_set
  Compares the current value of the atomic variable with the
  specified value and set to a new value when compare is successful.

  @note1hang The function retries until load lock and store conditional
             is successful.

  @param[in,out]  target  Pointer to the atomic variable.
  @param[in]      old_val Old value to compare.
  @param[in]      new_val New value to set.

  @return
  FALSE -- Specified value is not equal to the current value. \n
  TRUE --Specified value is equal to the current value.

  @dependencies
  None.
*/
unsigned int qurt_atomic_compare_and_set(unsigned int *target, unsigned int old_val, unsigned int new_val);

/** @endcond */

#endif