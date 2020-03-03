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
#define DEBUG_DYNAMIC_ARRAY

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

u08 CreateArray(const void *const identifier, const BaseSize_t sizeElement, const BaseSize_t sizeAll, BaseSize_t dataRice) {
#ifdef DEBUG_DYNAMIC_ARRAY
    writeLogStr("INFO: Create array");
#endif
    byte_ptr data = allocMem(sizeAll * sizeElement);
    if (data == NULL) return NO_MEMORY_ERROR;
    u08 res = CreateDataStruct(data, sizeElement, sizeAll);
    if (res != EVERYTHING_IS_OK) {
        freeMem(data);
        return res;
    }
    DynamicArray_t *arr = findArray(NULL);
    if (arr == NULL) {
    	delDataStruct(data);
    	freeMem(data);
        return NOT_FOUND_DATA_STRUCT_ERROR;
    }
    arr->base = createNewList((void *)data);
    arr->sizeBaseElement = sizeElement;
    arr->arrayLabel = (void *) identifier;
    arr->deltaDataRice = dataRice;
    return EVERYTHING_IS_OK;
}

static void deleteAllData(BaseSize_t arg_n, BaseParam_t dataStruct) {
#ifdef DEBUG_DYNAMIC_ARRAY
	writeLogStr("TRACE: Delete data struct in list dynamic arrays");
#endif
    delDataStruct((void *)dataStruct);
}

u08 delArray(const void *identifier) {
#ifdef DEBUG_DYNAMIC_ARRAY
    writeLogStr("INFO: Delete array");
#endif
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
    ListNode_t* head = peekFromFrontList(a->base);
    if (head == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
    if(head->data == NULL) {
		#ifdef DEBUG_DYNAMIC_ARRAY
    	writeLogStr("ERROR: Undefine behavior in PutToFrontArray");
		#endif
    	return UNDEFINED_BEHAVIOR;
    }
    if (PutToFrontDataStruct(Elem, head->data) != EVERYTHING_IS_OK) {
        byte_ptr newNode = allocMem(a->deltaDataRice * (a->sizeBaseElement));
        if (newNode == NULL) return NO_MEMORY_ERROR;
        u08 res = CreateDataStruct(newNode, a->sizeBaseElement, a->deltaDataRice);
        if (res != EVERYTHING_IS_OK) {
            freeMem(newNode);
			#ifdef DEBUG_DYNAMIC_ARRAY
            writeLogStr("ERROR: Can not create new struct in PutToFrontArray");
			#endif
            return res;
        }
        res = PutToFrontDataStruct(Elem, newNode);
        if (res != EVERYTHING_IS_OK) {
            freeMem(newNode);
            return res;
        }
        a->base = putToFrontList(a->base, newNode);
#ifdef DEBUG_DYNAMIC_ARRAY
        writeLogStr("TRACE: Add new list node and data struct to front");
#endif
    }
    return EVERYTHING_IS_OK;
}

u08 PutToEndArray(const void *Elem, const void *identifier) {
    DynamicArray_t *a = findArray(identifier);
    if (a == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
    ListNode_t* head = peekFromEndList(a->base);
    if (head == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
    if(head->data == NULL) {
		#ifdef DEBUG_DYNAMIC_ARRAY
    	writeLogStr("ERROR: Undefine behavior in PutToEndArray");
		#endif
    	return UNDEFINED_BEHAVIOR;
    }
    if (PutToEndDataStruct(Elem, head->data) != EVERYTHING_IS_OK) {
        byte_ptr newNode = allocMem(a->deltaDataRice * (a->sizeBaseElement));
        if (newNode == NULL) return NO_MEMORY_ERROR;
        u08 res = CreateDataStruct(newNode, a->sizeBaseElement, a->deltaDataRice);
        if (res != EVERYTHING_IS_OK) {
            freeMem(newNode);
			#ifdef DEBUG_DYNAMIC_ARRAY
            writeLogStr("ERROR: Can not create new struct in PutToEndArray");
			#endif
            return res;
        }
        res = PutToEndDataStruct(Elem, newNode);
        if (res != EVERYTHING_IS_OK) {
            freeMem(newNode);
            return res;
        }
        a->base = putToEndList(a->base, newNode);
#ifdef DEBUG_DYNAMIC_ARRAY
        writeLogStr("TRACE: Add new list node and data struct to end");
#endif
    }
    return EVERYTHING_IS_OK;
}

u08 GetFromFrontArray(void *returnValue, const void *identifier) {
    DynamicArray_t *a = findArray(identifier);
    if (a == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
    for (ListNode_t* head = peekFromFrontList(a->base); head != NULL && head->data != NULL; head = peekFromFrontList(a->base)) {
        if(GetFromFrontDataStruct(returnValue, head->data) == EVERYTHING_IS_OK) return EVERYTHING_IS_OK;
        if(head->next != NULL) {
			#ifdef DEBUG_DYNAMIC_ARRAY
        	writeLogStr("TRACE: Delete old list node and data struct from front");
			#endif
        	a->base = head->next;
        	delDataStruct(head->data);
        	deleteListNode(head);
        } else {
			#ifdef DEBUG_DYNAMIC_ARRAY
        	writeLogStr("TRACE: Struct is empty");
			#endif
        	return OVERFLOW_OR_EMPTY_ERROR;
        }
    }
	#ifdef DEBUG_DYNAMIC_ARRAY
    writeLogStr("ERROR: Undefine behavior in GetFromFrontArray");
	#endif
    return UNDEFINED_BEHAVIOR;
}

u08 GetFromEndArray(void *returnValue, const void *identifier) {
    DynamicArray_t *a = findArray(identifier);
    if (a == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
    for (ListNode_t* head = peekFromEndList(a->base); head != NULL && head->data != NULL; head = peekFromEndList(a->base)) {
        if (GetFromEndDataStruct(returnValue, head) == EVERYTHING_IS_OK) return EVERYTHING_IS_OK;
        if(head->prev != NULL) {
			#ifdef DEBUG_DYNAMIC_ARRAY
        	writeLogStr("TRACE: Delete old list node and data struct from end");
			#endif
        	a->base = head->prev;
        	delDataStruct(head->data);
        	deleteListNode(head);
        } else {
			#ifdef DEBUG_DYNAMIC_ARRAY
        	writeLogStr("TRACE: Struct is empty");
			#endif
        	 return OVERFLOW_OR_EMPTY_ERROR;
        }
    }
	#ifdef DEBUG_DYNAMIC_ARRAY
    writeLogStr("ERROR: Undefine behavior in GetFromEndArray");
	#endif
    return UNDEFINED_BEHAVIOR;
}

u08 peekFromFrontArray(void *returnValue, const void *identifier) {
    DynamicArray_t *a = findArray(identifier);
    if (a == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
    ListNode_t *head = peekFromFrontList(a->base);
    if(head == NULL) return OVERFLOW_OR_EMPTY_ERROR;
    return peekFromFrontData(returnValue, head->data);
}

u08 peekFromEndArray(void *returnValue, const void *identifier) {
    DynamicArray_t *a = findArray(identifier);
    if (a == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
    ListNode_t* head = peekFromEndList(a->base);
    if(head == NULL) return OVERFLOW_OR_EMPTY_ERROR;
    return peekFromEndData(returnValue, head);
}

u08 delFromFrontArray(const void *const identifier) {
    DynamicArray_t *a = findArray(identifier);
    if (a == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
    for (ListNode_t *head = peekFromFrontList(a->base); head != NULL && head->data != NULL; head = peekFromFrontList(a->base)) {
        if (delFromFrontDataStruct(head) == EVERYTHING_IS_OK) return EVERYTHING_IS_OK;
        if(head->next != NULL) {
#ifdef DEBUG_DYNAMIC_ARRAY
        	writeLogStr("TRACE: Delete old list node and data struct from front");
#endif
        	a->base = head->next;
        	delDataStruct(head->data);
        	deleteListNode(head);
        } else {
        	return OVERFLOW_OR_EMPTY_ERROR;
        }
    }
    return UNDEFINED_BEHAVIOR;
}

u08 delFromEndArray(const void *const identifier) {
    DynamicArray_t *a = findArray(identifier);
    if (a == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
    for(ListNode_t *head = peekFromEndList(a->base); head != NULL; head = peekFromEndList(a->base)) {
        if (delFromEndDataStruct(head) == EVERYTHING_IS_OK) return EVERYTHING_IS_OK;
        if(head->prev != NULL) {
#ifdef DEBUG_DYNAMIC_ARRAY
        	writeLogStr("TRACE: Delete old list node and data struct from end");
#endif
        	a->base = head->prev;
        	delDataStruct(head->data);
        	deleteListNode(head);
        } else {
        	return OVERFLOW_OR_EMPTY_ERROR;
        }
    }
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
    if (a == NULL) {
#ifdef DEBUG_DYNAMIC_ARRAY
    	writeLogStr("TRACE: Can not find array");
#endif
    	return TRUE;
    }
    ListNode_t *l = a->base = findHead(a->base);
    while (l != NULL) {
        if(!isEmptyDataStruct(l->data)) return FALSE;
        l = l->next;
    }
    return TRUE;
}

void clearArray(const void *const identifier) {
#ifdef DEBUG_DYNAMIC_ARRAY
    writeLogStr("TRACE: Clear dynamic array");
#endif
    DynamicArray_t *a = findArray(identifier);
    if (a == NULL) return;
    ListNode_t *l = a->base = findHead(a->base);
    while (l != NULL) {
        l = l->next;
        if (l != NULL) {
            a->base = l;
            delDataStruct(l->prev->data);
            deleteListNode(l->prev);
        } else {
        	clearDataStruct(a->base->data);
        }
    }
}

void forEachArray(const void *const identifier, TaskMng tsk) {
#ifdef DEBUG_DYNAMIC_ARRAY
    writeLogStr("TRACE: For each dynamic array");
#endif
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
