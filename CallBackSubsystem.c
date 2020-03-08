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
 * CallBackSubsystem.c
 *
 *  Created on: 28 квіт. 2017 р.
 *      Author: oleksiy.khanin
 */

#include "TaskMngr.h"
#include "PlatformSpecific.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef CALL_BACK_TASK
#if CALL_BACK_TASK_LIST_LEN > 0xFE
#error "incompatible size"
#endif

static void* labelPointer[CALL_BACK_TASK_LIST_LEN]; // Массив указателей на функции по завершении которых следует вызвать callBack (по сути метка колбэка)
static TaskList_t callBackList[CALL_BACK_TASK_LIST_LEN];	// Указатель на функцию которая будет вызвана

void initCallBackTask(void){
	for(u08 index=0;index<CALL_BACK_TASK_LIST_LEN; index++){
		labelPointer[index] = NULL;
	}
}

static u08 findCallBack(const void*const labelPtr){
	u08 index = 0;
	for(;index < CALL_BACK_TASK_LIST_LEN; index++){
		if(labelPointer[index] == labelPtr) break;
	}
	return index;
}

u08 registerCallBack(const TaskMng task, const BaseSize_t arg_n, const BaseParam_t arg_p, const void*const labelPtr){
	const unlock_t unlock = lock(callBackList);
	const u08 i = findCallBack(NULL);
	if(i < CALL_BACK_TASK_LIST_LEN) {
		labelPointer[i] = (void*)labelPtr;
		callBackList[i].Task = task;
		callBackList[i].arg_n = arg_n;
		callBackList[i].arg_p = arg_p;
		unlock(callBackList);
		return EVERYTHING_IS_OK;
	}
	unlock(callBackList);
#ifndef CHECK_ERRORS_CALLBACK
	MaximizeErrorHandler("Overflow callback tasks");
#endif
	return OVERFLOW_OR_EMPTY_ERROR;
}

void clearAllCallBackList(void) {
	const unlock_t unlock = lock(callBackList);
	initCallBackTask();
	unlock(callBackList);
}

void deleteCallBack(const BaseSize_t arg_n, const void*const labelPtr){
	for(u08 i = 0; i < CALL_BACK_TASK_LIST_LEN; i++){
		if(labelPointer[i] == labelPtr) {
			const unlock_t unlock = lock(callBackList);
			labelPointer[i] = NULL;
			unlock(callBackList);
		}
	}
}

void deleteCallBackByTask(TaskMng task) {
	for(u08 i = 0; i < CALL_BACK_TASK_LIST_LEN; i++){
		if(labelPointer[i] != NULL && callBackList[i].Task == task) {
			const unlock_t unlock = lock(callBackList);
			labelPointer[i] = NULL;
			unlock(callBackList);
		}
	}
}

void execCallBack(const void*const labelPtr){
	for(u08 i = 0; i < CALL_BACK_TASK_LIST_LEN; i++){
		if(labelPointer[i] == labelPtr) {
			if(callBackList[i].Task != NULL) {
			#ifdef SET_FRONT_TASK_ENABLE
				SetFrontTask(callBackList[i].Task,callBackList[i].arg_n,callBackList[i].arg_p);
			#else
				SetTask(callBackList[i].Task,callBackList[i].arg_n,callBackList[i].arg_p);
			#endif
			}
			const unlock_t unlock = lock(callBackList);
			labelPointer[i] = NULL;
			unlock(callBackList);
		}
	}
}

void execErrorCallBack(const BaseSize_t errorCode, const void*const labelPtr){
	for(u08 i = 0; i < CALL_BACK_TASK_LIST_LEN; i++){
		if(labelPointer[i] == labelPtr){
			if(callBackList[i].Task != NULL) {
				SetTask(callBackList[i].Task,errorCode,callBackList[i].arg_p);
			}
			const unlock_t unlock = lock(callBackList);
			labelPointer[i] = NULL;
			unlock(callBackList);
		}
	}
}

u08 changeCallBackLabel(const void*const oldLabel, const void*const newLabel){
	const unlock_t unlock = lock(callBackList);
	for(u08 i = 0; i<CALL_BACK_TASK_LIST_LEN; i++) {
		if(labelPointer[i] == oldLabel) labelPointer[i] = (void*)newLabel;
	}
	unlock(callBackList);
	return EVERYTHING_IS_OK;
}

#endif // CALL_BACK_TASK


#ifdef __cplusplus
}
#endif
