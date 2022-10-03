//
// Created by blabu on 29.10.2019.
//
/**/
#include <String.h>
#include "TaskMngr.h"
#include "PlatformSpecific.h"

#ifdef COMMAND_TASK

#if COMMAND_TASK_LIST_SIZE > 0xFF
#warning "ERROR COMAND_TASK_LIST_SIZE must be less 0xFF"
#endif

static TaskMng tasks[COMMAND_TASK_LIST_SIZE];
string_t commandList[COMMAND_TASK_LIST_SIZE];
string_t commandDescription[COMMAND_TASK_LIST_SIZE];

void (*commandHandler)(TaskMng task, string_t command, string_t description);

static u08 findTaskByCommand(string_t cmd) {
    u08 i = 0;
    for(;i<COMMAND_TASK_LIST_SIZE; i++) {
        if(str1_str2(commandList[i], cmd)) break;
    }
    return i;
}

static u08 findFree() {
    u08 i = 0;
    for(;i<COMMAND_TASK_LIST_SIZE; i++) {
        if(commandList[i] == NULL) break;
    }
    return i;
}

void initCommandList() {
    for(u08 i = 0; i<COMMAND_TASK_LIST_SIZE; i++) commandList[i] = NULL;
}

u08 delCommand(string_t command) {
    const unlock_t lck = lock((void*)commandList);
    const u08 pos = findTaskByCommand(command);
    if(pos == COMMAND_TASK_LIST_SIZE) {lck((void*)commandList); return NOT_FOUND_DATA_STRUCT_ERROR;}
    commandList[pos] = NULL;
    lck((void*)commandList);
    return EVERYTHING_IS_OK;
}

u08 addTaskCommand(TaskMng tsk, string_t command, string_t description) {
    const unlock_t lck = lock((void*)commandList);
    const u08 pos = findFree();
    if(pos == COMMAND_TASK_LIST_SIZE) {
        lck((void*)commandList);
        return OVERFLOW_OR_EMPTY_ERROR;
    }
    commandList[pos] = command;
    commandDescription[pos] = description;
    tasks[pos] = tsk;
    lck((void*)commandList);
    return EVERYTHING_IS_OK;
}

u08 execWithSubCommand(string_t command, BaseSize_t subCmdCount, string_t subCommands) {
    const unlock_t lck = lock((void*)commandList);
    const u08 pos = findTaskByCommand(command);
    if(pos == COMMAND_TASK_LIST_SIZE) {
        lck((void*)commandList);
        execCallBack(execWithSubCommand);
        return NOT_FOUND_DATA_STRUCT_ERROR;
    }
    if(tasks[pos] != NULL) {
        lck((void*)commandList);
        SetTask(tasks[pos], subCmdCount, (BaseParam_t)subCommands);
        changeCallBackLabel(execWithSubCommand, tasks[pos]);
        return EVERYTHING_IS_OK;
    }
    lck((void*)commandList);
    execCallBack(execWithSubCommand);
    return NULL_PTR_ERROR;
}

u08 execCommand(string_t input) {
	BaseSize_t n = strSplit(' ', input);
	if(!n) {
		execCallBack(execCommand);
		return NULL_PTR_ERROR;
	}
	changeCallBackLabel(execCommand, execWithSubCommand);
	return execWithSubCommand(input, n-1, input+strSize(input)+1);
}

void forEachCommand(cmdHandler_t handler) {
    if(handler == NULL) return;
    const unlock_t lck = lock((void*)commandList);
    for(u08 i = 0; i<COMMAND_TASK_LIST_SIZE; i++) {
        if(commandList[i] != NULL) {
        	handler(tasks[i], commandList[i], commandDescription[i]);
        }
    }
    lck((void*)commandList);
}

u08 getFreeCommandSize() {
	u08 n = 0;
	for(u08 i=0; i<COMMAND_TASK_LIST_SIZE; i++) {
		if(commandList[i] == NULL) n++;
	}
	return n;
}

#endif
