/*
MIT License

Copyright (c) 2017 Oleksiy Khanin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 * */

/*
 * Signal.c
 *
 *  Created on: 5 мая 2018 г.
 *      Author: blabu
 */
#include "TaskMngr.h"
#include "PlatformSpecific.h"
#ifdef SIGNALS_TASK

#ifdef __cplusplus
extern "C" {
#endif

#if SIGNAL_LIST_LEN > 0xFF
#error "SIGNAL MUST BE LESS 0xFF"
#endif

volatile static TaskMng taskList[SIGNAL_LIST_LEN];
volatile static void *signalList[SIGNAL_LIST_LEN];

// Прочесываем очередь, находим задачи подписанные на этот сигнал и вызываем их. При этом задачи из списка не удаляются
void emitSignal(const void *const signal, BaseSize_t arg_n, BaseParam_t arg_p) {
    for (u08 i = 0; i < SIGNAL_LIST_LEN; i++) {
        if (signalList[i] == signal) {
            if (taskList[i] != NULL) {
                SetTask(taskList[i], arg_n, arg_p);
            }
        }
    }
}

u08 connectTaskToSignal(const TaskMng task, const void *const signal) {
    for (u08 i = 0; i < SIGNAL_LIST_LEN; i++) {
        if (signalList[i] == NULL) {
            unlock_t unlock = lock(signalList);
            signalList[i] = (void *) signal;
            taskList[i] = task;
            unlock(signalList);
            return EVERYTHING_IS_OK;
        }
    }
    return OVERFLOW_OR_EMPTY_ERROR;
}

void disconnectTaskFromSignal(const TaskMng task, const void *const signal) {
    for (u08 i = 0; i < SIGNAL_LIST_LEN; i++) {
        if (signalList[i] == signal && taskList[i] == task) {
            unlock_t unlock = lock(signalList);
            signalList[i] = NULL;
            unlock(signalList);
        }
    }
}

u08 getFreeSignalSize() {
	u08 n = 0;
	for(u08 i=0; i<SIGNAL_LIST_LEN; i++) {
		if(signalList[i] == NULL) n++;
	}
	return n;
}

#ifdef __cplusplus
}
#endif
#endif //SIGNALS_TASK
