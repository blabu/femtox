#include "PlatformSpecific.h"
#include "TaskMngr.h"

#ifdef SCELETON

#ifdef MAXIMIZE_OVERFLOW_ERROR
#include "logging.h"
	void MaximizeErrorHandler(string_t str){
		writeLogStr(str);
		for(u16 i = 0; i<0xFFFF; i++);
		initWatchDog();
		while(1);
	}
#else
#include "logging.h"
	void MaximizeErrorHandler(string_t str){
		writeLogStr(str);
	}
#endif
/********************************************************************************************************************
*********************************************************************************************************************
                                            ПЛАТФОРМО-ЗАВИСИМЫЕ ФУНКЦИИ														|
*********************************************************************************************************************
*********************************************************************************************************************/


//#define INTERRUPT_ENABLE  __enable_irq()   //{asm("nop"); __asm__ __volatile__("eint");}
//#define INTERRUPT_DISABLE __disable_irq()  //{__asm__ __volatile__("dint nop"); asm("nop");}
//#define INTERRUPT_STATUS  (__get_CONTROL() & (uint32_t)(1<<7))

static void unlock(const void*const resourceId){}

static void empty(const void*const resourceId) {}

unlock_t lock(const void*const resourceId){
	if(resourceId != NULL) return unlock;
	else return empty;
}

void initWatchDog(void){}

void resetWatchDog(void){}

void _init_Timer(void){}

void MainTimerISRHandler(void){TimerISR();}

#ifndef USE_SOFT_UART
void _initTimerSoftUart() {}
void initProgramUartGPIO(unsigned short TX_MASK, unsigned short RX_MASK){}
void _deInitTimerSoftUart(){}
void deInitProgramUartGPIO(unsigned short TX_MASK, unsigned short RX_MASK){}
#else
#error "Not implemented yet soft UART"
#endif

#ifdef NATIVE_TIMER_PWR_SAVE
#error "Can not compile whith it. Not implemented yet"
#endif

#endif
