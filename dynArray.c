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

#include "dynArray.h"
#include "List.h"
#include "logging.h"
#ifdef _DYNAMIC_ARRAY

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void *arrayLabel;
    ListNode_t *base;
    BaseSize_t sizeBaseElement;
    BaseSize_t deltaDataRice;
} DynamicArray_t;

static DynamicArray_t allArrays[DYNAMIC_ARRAY_SIZE];

void initDynamicArray() {
    for (BaseSize_t i = 0; i < DYNAMIC_ARRAY_SIZE; i++) {
        allArrays[i].arrayLabel = NULL;
    }
}

static DynamicArray_t *findArray(const void *identifier) {
    for (BaseSize_t i = 0; i < DYNAMIC_ARRAY_SIZE; i++) {
        if (allArrays[i].arrayLabel == identifier) return allArrays + i;
    }
    return NULL;
}

u08
CreateArray(const void *const identifier, const BaseSize_t sizeElement, const BaseSize_t sizeAll, BaseSize_t dataRice) {
    writeLogStr("INFO: Create array");
    byte_ptr data = allocMem(sizeAll * sizeElement);
    if (data == NULL) return NO_MEMORY_ERROR;
    u08 res = CreateDataStruct(data, sizeElement, sizeAll);
    if (res != EVERYTHING_IS_OK) {
        freeMem(data);
        return res;
    }
    DynamicArray_t *arr = findArray(NULL);
    if (arr == NULL) {
        return NOT_FOUND_DATA_STRUCT_ERROR;
    }
    arr->base = createNewList((void *) data);
    arr->sizeBaseElement = sizeElement;
    arr->arrayLabel = (void *) identifier;
    arr->deltaDataRice = dataRice;
    return EVERYTHING_IS_OK;
}

static void deleteAllData(BaseSize_t arg_n, BaseParam_t dataStruct) {
    delDataStruct((void *) dataStruct);
}

u08 delArray(const void *identifier) {
    writeLogStr("INFO: Delete array");
    DynamicArray_t *a = findArray(identifier);
    if (a == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
    forEachListNodes(a->base, deleteAllData, FALSE, 0);
    deleteList(a->base);
    a->arrayLabel = NULL;
    return EVERYTHING_IS_OK;
}

u08 PutToFrontArray(const void *Elem, const void *identifier) {
    DynamicArray_t *a = findArray(identifier);
    if (a == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
    void *head = peekFromFrontList(a->base);
    if (head == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
    if (PutToFrontDataStruct(Elem, head) != EVERYTHING_IS_OK) {
        byte_ptr newNode = allocMem(a->deltaDataRice * (a->sizeBaseElement));
        if (newNode == NULL) return NO_MEMORY_ERROR;
        u08 res = CreateDataStruct(newNode, a->sizeBaseElement, a->deltaDataRice);
        if (res != EVERYTHING_IS_OK) {
            freeMem(newNode);
            return res;
        }
        res = PutToFrontDataStruct(Elem, newNode);
        if (res != EVERYTHING_IS_OK) {
            freeMem(newNode);
            return res;
        }
        a->base = putToFrontList(a->base, newNode);
        writeLogStr("TRACE: Add new list node and data struct to front");
    }
    return EVERYTHING_IS_OK;
}

u08 PutToEndArray(const void *Elem, const void *identifier) {
    DynamicArray_t *a = findArray(identifier);
    if (a == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
    void *head = peekFromEndList(a->base);
    if (head == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
    if (PutToEndDataStruct(Elem, head) != EVERYTHING_IS_OK) {
        byte_ptr newNode = allocMem(a->deltaDataRice * (a->sizeBaseElement));
        if (newNode == NULL) return NO_MEMORY_ERROR;
        u08 res = CreateDataStruct(newNode, a->sizeBaseElement, a->deltaDataRice);
        if (res != EVERYTHING_IS_OK) {
            freeMem(newNode);
            return res;
        }
        res = PutToEndDataStruct(Elem, newNode);
        if (res != EVERYTHING_IS_OK) {
            freeMem(newNode);
            return res;
        }
        a->base = putToEndList(a->base, newNode);
        writeLogStr("TRACE: Add new list node and data struct to end");
    }
    return EVERYTHING_IS_OK;
}

u08 GetFromFrontArray(void *returnValue, const void *identifier) {
    DynamicArray_t *a = findArray(identifier);
    if (a == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
    void *head = peekFromFrontList(a->base);
    for (; head != NULL; head = peekFromFrontList(a->base)) {
        if (GetFromFrontDataStruct(returnValue, head) == EVERYTHING_IS_OK) return EVERYTHING_IS_OK;
        writeLogStr("TRACE: Delete old list node and data struct from front");
        delDataStruct(head);
        freeMem(head);
        a->base = getFromFrontList(a->base, &head);
        if (a->base == NULL) return OVERFLOW_OR_EMPTY_ERROR;
    }
    if (head == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
    return UNDEFINED_BEHAVIOR;
}

u08 GetFromEndArray(void *returnValue, const void *identifier) {
    DynamicArray_t *a = findArray(identifier);
    if (a == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
    void *head = peekFromEndList(a->base);
    for (; head != NULL; head = peekFromEndList(a->base)) {
        if (GetFromEndDataStruct(returnValue, head) == EVERYTHING_IS_OK) return EVERYTHING_IS_OK;
        writeLogStr("TRACE: Delete old list node and data struct from end");
        delDataStruct(head);
        freeMem(head);
        a->base = getFromEndList(a->base, &head);
        if (a->base == NULL) return OVERFLOW_OR_EMPTY_ERROR;
    }
    if (head == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
    return UNDEFINED_BEHAVIOR;
}

u08 peekFromFrontArray(void *returnValue, const void *identifier) {
    DynamicArray_t *a = findArray(identifier);
    if (a == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
    void *head = peekFromFrontList(a->base);
    for (; head != NULL; head = peekFromFrontList(a->base)) {
        if (peekFromFrontData(returnValue, head) == EVERYTHING_IS_OK) return EVERYTHING_IS_OK;
        writeLogStr("TRACE: Delete old list node and data struct from front");
        delDataStruct(head);
        freeMem(head);
        a->base = getFromFrontList(a->base, &head);
        if (a->base == NULL) return OVERFLOW_OR_EMPTY_ERROR;
    }
    if (head == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
    return UNDEFINED_BEHAVIOR;
}

u08 peekFromEndArray(void *returnValue, const void *identifier) {
    DynamicArray_t *a = findArray(identifier);
    if (a == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
    void *head = peekFromEndList(a->base);
    for (; head != NULL; head = peekFromEndList(a->base)) {
        if (peekFromEndData(returnValue, head) == EVERYTHING_IS_OK) return EVERYTHING_IS_OK;
        writeLogStr("TRACE: Delete old list node and data struct from end");
        delDataStruct(head);
        freeMem(head);
        a->base = getFromEndList(a->base, &head);
        if (a->base == NULL) return OVERFLOW_OR_EMPTY_ERROR;
    }
    if (head == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
    return UNDEFINED_BEHAVIOR;
}

u08 delFromFrontArray(const void *const identifier) {
    DynamicArray_t *a = findArray(identifier);
    if (a == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
    void *head = peekFromFrontList(a->base);
    for (; head != NULL; head = peekFromFrontList(a->base)) {
        if (delFromFrontDataStruct(head) == EVERYTHING_IS_OK) return EVERYTHING_IS_OK;
        writeLogStr("TRACE: Delete old list node and data struct from front");
        delDataStruct(head);
        freeMem(head);
        a->base = getFromFrontList(a->base, &head);
        if (a->base == NULL) return OVERFLOW_OR_EMPTY_ERROR;
    }
    if (head == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
    return UNDEFINED_BEHAVIOR;
}

u08 delFromEndArray(const void *const identifier) {
    DynamicArray_t *a = findArray(identifier);
    if (a == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
    void *head = peekFromEndList(a->base);
    for (; head != NULL; head = peekFromEndList(a->base)) {
        if (delFromEndDataStruct(head) == EVERYTHING_IS_OK) return EVERYTHING_IS_OK;
        writeLogStr("TRACE: Delete old list node and data struct from end");
        delDataStruct(head);
        freeMem(head);
        a->base = getFromEndList(a->base, &head);
        if (a->base == NULL) return OVERFLOW_OR_EMPTY_ERROR;
    }
    if (head == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
    return UNDEFINED_BEHAVIOR;
}

BaseSize_t getCurrentSizeArray(const void *const identifier) {
    DynamicArray_t *a = findArray(identifier);
    if (a == NULL) return 0;
    BaseSize_t sz = 0;
    ListNode_t *l = a->base = findHead(a->base);
    while (l != NULL) {
        sz += getCurrentSizeDataStruct(l->data);
        l = l->next;
    }
    return sz;
}

bool_t isEmptyArray(const void *const identifier) {
    DynamicArray_t *a = findArray(identifier);
    if (a == NULL) return TRUE;
    ListNode_t *l = a->base = findHead(a->base);
    while (l != NULL) {
        if (!isEmptyDataStruct(l->data)) return TRUE;
        l = l->next;
    }
    return FALSE;
}

void clearArray(const void *const identifier) {
    writeLogStr("TRACE: Clear dynamic array");
    DynamicArray_t *a = findArray(identifier);
    if (a == NULL) return;
    ListNode_t *l = a->base = findHead(a->base);
    while (l != NULL) {
        clearDataStruct(l->data);
        l = l->next;
        if (l != NULL) {
            a->base = l;
            delDataStruct(l->prev->data);
            deleteListNode(l->prev);
        }
    }
}

void forEachArray(const void *const identifier, TaskMng tsk) {
    writeLogStr("TRACE: For each dynamic array");
    DynamicArray_t *a = findArray(identifier);
    if (a == NULL) return;
    ListNode_t *l = a->base = findHead(a->base);
    while (l != NULL) {
        forEachDataStruct(l->data, tsk);
        l = l->next;
    }
}

#ifdef __cplusplus
}
#endif
#endif //_DYNAMIC_ARRAY
