#include "mytimer.h"


void MYTIMER_init(int timerNo)
{
    // we don't have to do anything.
}

void MYTIMER_enable(int timerNo)
{
	if (timerNo == 2) {
		MYTIMER2->control |= MYTIMER_ENABLE_MASK;
	} else {
		MYTIMER->control |= MYTIMER_ENABLE_MASK;
	}
}

void MYTIMER_disable(int timerNo)
{
	if (timerNo == 2) {
		MYTIMER2->control &= 0xFFFFFFFE;
	} else {
		MYTIMER->control &= 0xFFFFFFFE;
	}
}

void MYTIMER_setOverflowVal(int timerNo, uint32_t value)
{
	if (timerNo == 2) {
		uint32_t * timerAddr = (uint32_t*)(MYTIMER2);
		*timerAddr = value; // overflowReg is at offset 0x0
	} else {
		uint32_t * timerAddr = (uint32_t*)(MYTIMER);
		*timerAddr = value; // overflowReg is at offset 0x0
	}

}

uint32_t MYTIMER_getCounterVal(int timerNo)
{
	if (timerNo == 2) {
		uint32_t * timerAddr = (uint32_t*)(MYTIMER2);
		return *(timerAddr+1); // counterReg is at offset 0x4
	} else {
		uint32_t * timerAddr = (uint32_t*)(MYTIMER);
		return *(timerAddr+1); // counterReg is at offset 0x4
	}
}



/**
 * Enable all interrupts
 */
void MYTIMER_enable_allInterrupts(int timerNo)
{
	if (timerNo == 2) {
		MYTIMER2->control |= 0x00000002;
	} else {
		MYTIMER->control |= 0x00000002;
	}
}
/**
 * Disable all interrupts
 */

void MYTIMER_disable_allInterrupts(int timerNo)
{
	if (timerNo == 2) {
		MYTIMER2->control &= 0xFFFFFFFD;
	} else {
		MYTIMER->control &= 0xFFFFFFFD;
	}
}
/**
 *
 * Enable compare interrupt
 */

void MYTIMER_enable_compareInt(int timerNo)
{
	if (timerNo == 2) {
		MYTIMER2->control |= 0x00000004;
	} else {
		MYTIMER->control |= 0x00000004;
	}
}

/**
 * Disable compare interrupt
 */

void MYTIMER_disable_compareInt(int timerNo){
	if (timerNo == 2) {
		MYTIMER->control &= 0xFFFFFFFB;
	} else {
		MYTIMER->control &= 0xFFFFFFFB;
	}
}
/**
 * Set Compare value
 */

void MYTIMER_setCompareVal(int timerNo, uint32_t compare)
{
	if (timerNo == 2) {
		uint32_t * compRegAddr = MYTIMER2->compare;
		*compRegAddr = compare;
	} else {
		uint32_t * compRegAddr = MYTIMER->compare;
		*compRegAddr = compare;
	}
}
/**
 * Enable overflow interrupt
 */

void MYTIMER_enable_overflowInt(int timerNo)
{
	if (timerNo == 2) {
		MYTIMER2->control |= 0x00000008;
	} else {
		MYTIMER->control |= 0x00000008;
	}
}

/**
 * Disable overflow interrupt
 */

void MYTIMER_disable_overflowInt(int timerNo)
{
	if (timerNo == 2) {
		MYTIMER2->control |= 0xFFFFFFF7;
	} else {
		MYTIMER->control |= 0xFFFFFFF7;
	}
}

/**
  * Interrupt status
*/

uint32_t MYTIMER_getInterrupt_status(int timerNo){
	if (timerNo == 2) {
		uint32_t * compRegAddr = MYTIMER2->interrupt_status;
		return *compRegAddr;
	} else {
		uint32_t * compRegAddr = MYTIMER->interrupt_status;
		return *compRegAddr;
	}
}

/**
 * Enable Capture
 */
void MYTIMER_enable_capture(int timerNo){
	if (timerNo == 2) {
		MYTIMER2->control |= 0x0000020;
	} else {
		MYTIMER->control |= 0x0000020;
	}
}
/**
 * Disable Capture
 */
void MYTIMER_disable_capture(int timerNo){
	if (timerNo == 2) {
		MYTIMER->control &= 0xFFFFFFDF;
	} else {
		MYTIMER->control &= 0xFFFFFFDF;
	}
}

/**
 * Read the synchronous capture value
*/

uint32_t MYTIMER_get_sync_capture(int timerNo){
	if (timerNo == 2) {
		uint32_t * compRegAddr = MYTIMER2->sync_capture;
		return *compRegAddr;
	} else {
		uint32_t * compRegAddr = MYTIMER->sync_capture;
		return *compRegAddr;
	}
}
/**
 * Read the asynchronous capture value
*/

uint32_t MYTIMER_get_async_capture(int timerNo){
	if (timerNo == 2) {
		uint32_t * compRegAddr = MYTIMER2->async_capture;
		return *compRegAddr;
	} else {
		uint32_t * compRegAddr = MYTIMER->async_capture;
		return *compRegAddr;
	}
}
