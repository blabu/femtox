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
	for(u08 index = 0;index<CALL_BACK_TASK_LIST_LEN; index++){
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
	unlock_t unlock = lock(callBackList);
	u08 i = findCallBack(NULL);
	if(i < CALL_BACK_TASK_LIST_LEN) {
		callBackList[i].Task = task;
		callBackList[i].arg_n = arg_n;
		callBackList[i].arg_p = arg_p;
		labelPointer[i] = (void*)labelPtr;
		unlock(callBackList);
		return EVERYTHING_IS_OK;
	}
	unlock(callBackList);
#ifndef CHECK_ERRORS_CALLBACK
	MaximizeErrorHandler("Overflow callback tasks");
#endif
	return OVERFLOW_OR_EMPTY_ERROR;
}

void clearAllCallBackList() {
	unlock_t unlock = lock(callBackList);
	initCallBackTask();
	unlock(callBackList);
}

void deleteCallBack(const BaseSize_t arg_n, const void*const labelPtr){
	for(u08 i = 0; i < CALL_BACK_TASK_LIST_LEN; i++){
		if(labelPointer[i] == labelPtr) {
			unlock_t unlock = lock(callBackList);
			labelPointer[i] = NULL;
			unlock(callBackList);
		}
	}
}

void execCallBack(const void*const labelPtr){
	for(u08 i = 0; i < CALL_BACK_TASK_LIST_LEN; i++){
		if(labelPointer[i] == labelPtr){
			if(callBackList[i].Task != NULL) {
				SetTask(callBackList[i].Task,callBackList[i].arg_n,callBackList[i].arg_p);
			}
			unlock_t unlock = lock(callBackList);
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
			unlock_t unlock = lock(callBackList);
			labelPointer[i] = NULL;
			unlock(callBackList);
		}
	}
}

u08 changeCallBackLabel(const void*const oldLabel, const void*const newLabel){
	unlock_t unlock = lock(callBackList);
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
