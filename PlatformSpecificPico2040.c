#include "PlatformSpecific.h"
#include <pico/stdlib.h>
#include <pico/sync.h>
#include <hardware/watchdog.h>
#include <hardware/irq.h>

#ifdef MAXIMIZE_OVERFLOW_ERROR
	void MaximizeErrorHandler(string_t str){
		initWatchDog();
		while(1);
	}
#else
#include <stdio.h>
	void MaximizeErrorHandler(string_t str){
		puts(str);
	}
#endif
/********************************************************************************************************************
*********************************************************************************************************************
                                            ПЛАТФОРМО-ЗАВИСИМЫЕ ФУНКЦИИ														|
*********************************************************************************************************************
*********************************************************************************************************************/
void initWatchDog(){
	watchdog_enable(2000, false);
}

void resetWatchDog(void){
	watchdog_update();
}

void idle() {
	__wfi();
}
extern void TimerISR(void);


static repeating_timer_t MainTimer;
static bool timerHandler(struct repeating_timer *t) {
	TimerISR();
	return true;
}

#define LOCK_SIZE 10
typedef PAIR(void*, recursive_mutex_t) Lock_t;
static Lock_t lc[LOCK_SIZE];
static u32 interruptState;
static critical_section_t lockLock;

void _init_Timer(){
	critical_section_init(&lockLock);
	for(u08 i = 0; i<LOCK_SIZE; i++) {
		lc[i].first = NULL;
		recursive_mutex_init(&lc[i].second);
	}
	add_repeating_timer_ms(-1*(s16)(1000/TICK_PER_SECOND), timerHandler, NULL, &MainTimer);
}

static u08 find(const void*const id) {
	for(u08 i = 0;i<LOCK_SIZE; i++) {
		if(lc[i].first == id) return i;
	}
	return LOCK_SIZE;
}

static inline void empty(const void*const resourceId) {
	u08 i = find(resourceId);
	if(i != LOCK_SIZE) {
		recursive_mutex_exit(&lc[i].second);
	}
}

static inline void unlock(const void*const resourceId) {
	if(interruptState) {
		u32 temp = interruptState;
		interruptState = 0;
		restore_interrupts(temp);
	}
	empty(resourceId);
}


unlock_t lock(const void*const resourceId) {
	critical_section_enter_blocking(&lockLock);
	u08 i = find(resourceId);
	if(i == LOCK_SIZE) {
		i = find(NULL);
		if(i == LOCK_SIZE) {
			return empty;
		}
		lc[i].first = (void*)resourceId;
	}
	critical_section_exit(&lockLock);
	uint32_t caller = 0;
	recursive_mutex_enter_blocking(&lc[i].second);
	
	if(interruptState) return empty;
	interruptState = save_and_disable_interrupts();
	
	return unlock;
}
