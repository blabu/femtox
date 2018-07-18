/*
 * Signal.c
 *
 *  Created on: 5 мая 2018 г.
 *      Author: blabu
 */
#include "TaskMngr.h"
#include "PlatformSpecific.h"
#ifdef SIGNALS_TASK

#if SIGNAL_LIST_LEN>0xFF
#error "SIGNAL MUST BE LESS 0xFF"
#endif

static TaskMng taskList[SIGNAL_LIST_LEN];
static void* signalList[SIGNAL_LIST_LEN];

// Прочесываем очередь, находим задачи подписанные на этот сигнал и вызываем их. При этом задачи из списка не удаляются
void emitSignal(void* signal, BaseSize_t arg_n, BaseParam_t arg_p) {
	for(u08 i = 0; i<SIGNAL_LIST_LEN; i++) {
		if(signalList[i] == signal) {
			if(taskList[i] != NULL) {
				SetTask(taskList[i],arg_n,arg_p);
			}
		}
	}
}

void connectTaskToSignal(TaskMng task, void* signal) {
	bool_t flagISR = FALSE;
	for(u08 i = 0; i<SIGNAL_LIST_LEN; i++) {
		if(signalList[i] == NULL) {
			if(INTERRUPT_STATUS) {
				flagISR = TRUE;
				INTERRUPT_DISABLE;
			}
			signalList[i] = signal;
			taskList[i] = task;
			if(flagISR) INTERRUPT_ENABLE;
			return;
		}
	}
}

void disconnectTaskFromSignal(TaskMng task, void* signal){
	bool_t flagISR = FALSE;
	for(u08 i = 0; i<SIGNAL_LIST_LEN; i++) {
		if(signalList[i] == signal && taskList[i] == task) {
			if(INTERRUPT_STATUS) {
				flagISR = TRUE;
				INTERRUPT_DISABLE;
			}
			signalList[i] = NULL;
			if(flagISR) INTERRUPT_ENABLE;
		}
	}
}
#endif //SIGNALS_TASK
