#ifndef MYTIMER_H_
#define MYTIMER_H_

#include "CMSIS/a2fxxxm3.h"

#define MYTIMER_BASE 0x40050000
#define MYTIMER_BASE2 0X40050200

// The technique of using a structure declaration
// to describe the device register layout and names is
// very common practice. Notice that there aren't actually
// any objects of that type defined, so the declaration
// simply indicates the structure without using up any store.

typedef struct
{
    uint32_t overflow; // Offset 0x00
    uint32_t counter; // Offset 0x04
    uint32_t control; // Offset 0x08
    uint32_t compare; // Offset 0x0C
    uint32_t interrupt_status; // Offset 0x10
    uint32_t sync_capture; // Offset 0x14
    uint32_t async_capture; // Offset 0x18
} mytimer_t;

#define MYTIMER_ENABLE_MASK 0x00000001UL

// Using the mytimer_t structure we can make the
// compiler do the offset mapping for us.
// To access the device registers, an appropriately
// cast constant is used as if it were pointing to
// such a structure, but of course it points to memory addresses instead.
// Look at at mytimer.c
// Look at the the functions's disassembly
// in .lst file under the Debug folder

#define MYTIMER ((mytimer_t *) MYTIMER_BASE)
#define MYTIMER2 ((mytimer_t *) MYTIMER_BASE2)

/**
 * Initialize the MYTIMER
*/
void MYTIMER_init(int timerNo);

/**
 * Start MYTIMER
*/
void MYTIMER_enable(int timerNo);

/**
 * Stop MYTIMER
*/
void MYTIMER_disable(int timerNo);

/**
 * Set the limit to which the timer counts.
*/
void MYTIMER_setOverflowVal(int timerNo, uint32_t value);

/**
 * Read the counter value of the timer.
*/
uint32_t MYTIMER_getCounterVal(int timerNo);



/**
 * Enable all interrupts
 */
void MYTIMER_enable_allInterrupts(int timerNo);

/**
 * Disable all interrupts
 */
void MYTIMER_disable_allInterrupts(int timerNo);

/**
 * Enable compare interrupt
 */
void MYTIMER_enable_compareInt(int timerNo);

/**
 * Disable compare interrupt
 */
void MYTIMER_disable_compareInt(int timerNo);

/**
 * Set Compare value
 */
void MYTIMER_setCompareVal(int timerNo, uint32_t compare);

/**
 * Enable overflow interrupt
 */
void MYTIMER_enable_overflowInt(int timerNo);

/**
 * Disable overflow interrupt
 */
void MYTIMER_disable_overflowInt(int timerNo);

/**
  * Interrupt status
*/
uint32_t MYTIMER_getInterrupt_status(int timerNo);

/**
 * Enable Capture
 */
void MYTIMER_enable_capture(int timerNo);

/**
 * Disable Capture
 */
void MYTIMER_disable_capture(int timerNo);

/**
 * Read the synchronous capture value
*/
uint32_t MYTIMER_get_sync_capture(int timerNo);

/**
 * Read the asynchronous capture value
*/
uint32_t MYTIMER_get_async_capture(int timerNo);

#endif /* MYTIMER_H_ */
