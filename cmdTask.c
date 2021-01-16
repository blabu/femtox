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

static TaskList_t tasks[COMMAND_TASK_LIST_SIZE];
string_t commandList[COMMAND_TASK_LIST_SIZE];

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

u08 addTaskCommand(TaskMng tsk, BaseSize_t arg_n, BaseParam_t arg_p, string_t command) {
    const unlock_t lck = lock((void*)commandList);
    const u08 pos = findFree();
    if(pos == COMMAND_TASK_LIST_SIZE) {
        lck((void*)commandList);
        return OVERFLOW_OR_EMPTY_ERROR;
    }
    commandList[pos] = command;
    tasks[pos].Task = tsk;
    tasks[pos].arg_n = arg_n;
    tasks[pos].arg_p = arg_p;
    lck((void*)commandList);
    return EVERYTHING_IS_OK;
}

u08 execCommand(string_t command) {
    const unlock_t lck = lock((void*)commandList);
    const u08 pos = findTaskByCommand(command);
    if(pos == COMMAND_TASK_LIST_SIZE) {
        lck((void*)commandList);
        return NOT_FOUND_DATA_STRUCT_ERROR;
    }
    if(tasks[pos].Task != NULL) {
        lck((void*)commandList);
        SetTask(tasks[pos].Task,tasks[pos].arg_n, tasks[pos].arg_p);
        return EVERYTHING_IS_OK;
    }
    lck((void*)commandList);
    return NULL_PTR_ERROR;
}

void execCommandTask(BaseSize_t  arg_n, string_t command) {
    const unlock_t lck = lock((void*)commandList);
    const u08 pos = findTaskByCommand(command);
    if(pos == COMMAND_TASK_LIST_SIZE) {
        lck((void*)commandList);
        return;
    }
    if(tasks[pos].Task != NULL) {
        lck((void*)commandList);
        SetTask(tasks[pos].Task,tasks[pos].arg_n, tasks[pos].arg_p);
    }
    lck((void*)commandList);
}

void forEachCommand(TaskMng tsk, BaseSize_t arg_n, bool_t toManager) {
    if(tsk == NULL) return;
    const unlock_t lck = lock((void*)commandList);
    for(u08 i = 0; i<COMMAND_TASK_LIST_SIZE; i++) {
        if(commandList[i] != NULL) {
            if(toManager) SetTask(tsk,arg_n,(BaseParam_t)commandList[i]);
            else tsk(arg_n,(BaseParam_t)commandList[i]);
        }
    }
    lck((void*)commandList);
}

#endif
