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
void emitSignal(const void*const signal, BaseSize_t arg_n, BaseParam_t arg_p) {
	for(u08 i = 0; i<SIGNAL_LIST_LEN; i++) {
		if(signalList[i] == signal) {
			if(taskList[i] != NULL) {
				SetTask(taskList[i],arg_n,arg_p);
			}
		}
	}
}

void connectTaskToSignal(const TaskMng task, const void*const signal) {
	for(u08 i = 0; i<SIGNAL_LIST_LEN; i++) {
		if(signalList[i] == NULL) {
			unlock_t unlock = lock(signalList);
			signalList[i] = (void*)signal;
			taskList[i] = task;
			unlock(signalList);
			return;
		}
	}
}

void disconnectTaskFromSignal(const TaskMng task, const void*const signal){
	for(u08 i = 0; i<SIGNAL_LIST_LEN; i++) {
		if(signalList[i] == signal && taskList[i] == task) {
			unlock_t unlock = lock(signalList);
			signalList[i] = NULL;
			unlock(signalList);
		}
	}
}
#endif //SIGNALS_TASK
