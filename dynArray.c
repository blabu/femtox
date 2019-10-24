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

const u08 deltaBaseDataRice = 10;
typedef struct {
	void* arrayLabel;
	ListNode_t* base;
	BaseSize_t sizeBaseElement;
} DynamicArray_t;

DynamicArray_t allArrays;

static DynamicArray_t* findArray(const void* identifier) {
	if(allArrays.arrayLabel == identifier) return &allArrays;
	return NULL;
}

u08 CreateArray(const void* identifier, const BaseSize_t sizeElement, const BaseSize_t sizeAll) {
	byte_ptr data = allocMem(sizeAll*sizeElement);
	if(data == NULL) return NO_MEMORY_ERROR;
	u08 res = CreateDataStruct(data,sizeElement,sizeAll);
	if(res != EVERYTHING_IS_OK) {
		freeMem(data);
		return res;
	}
	DynamicArray_t* arr = findArray(NULL);
	if(arr == NULL) {
		return NOT_FOUND_DATA_STRUCT_ERROR;
	}
	arr->base = createNewList((void*)data);
	arr->sizeBaseElement = sizeElement;
	arr->arrayLabel = identifier;
	return EVERYTHING_IS_OK;
}

static void deleteAllData(BaseSize_t arg_n, BaseParam_t arg_p) {
	delDataStruct((void*)arg_p);
	freeMem(arg_p);
}

u08 delArray(const void* identifier) {
	DynamicArray_t* a = findArray(identifier);
	if(a == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
	forEachListNodes(a->base, deleteAllData, TRUE, 0);
	deleteList(a->base);
	return EVERYTHING_IS_OK;
}

u08 PutToFrontArray(const void * Elem, const void* identifier) {
	DynamicArray_t* a = findArray(identifier);
	if(a == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
	void* head = peekFromFrontList(a->base);
	if(head == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
	if(PutToFrontDataStruct(Elem, head) != EVERYTHING_IS_OK) {
		byte_ptr newNode = allocMem(deltaBaseDataRice*(a->sizeBaseElement));
		if(newNode == NULL) return NO_MEMORY_ERROR;
		u08 res = CreateDataStruct(newNode,a->sizeBaseElement,deltaBaseDataRice);
		if(res != EVERYTHING_IS_OK) {
			freeMem(newNode);
			return res;
		}
		res = PutToFrontDataStruct(Elem, newNode);
		if(res != EVERYTHING_IS_OK) {
			freeMem(newNode);
			return res;
		}
		a->base = putToFrontList(a->base, newNode);
		writeLogStr("Add new list node and data struct");
	}
	return EVERYTHING_IS_OK;
}

u08 PutToEndArray(const void* Elem, const void* identifier) {
	DynamicArray_t* a = findArray(identifier);
	if(a == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
	void* head = peekFromEndList(a->base);
	if(head == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
	if(PutToEndDataStruct(Elem, head) != EVERYTHING_IS_OK) {
		byte_ptr newNode = allocMem(deltaBaseDataRice*(a->sizeBaseElement));
		if(newNode == NULL) return NO_MEMORY_ERROR;
		u08 res = CreateDataStruct(newNode,a->sizeBaseElement, deltaBaseDataRice);
		if(res != EVERYTHING_IS_OK) {
			freeMem(newNode);
			return res;
		}
		res = PutToEndDataStruct(Elem, newNode);
		if(res != EVERYTHING_IS_OK) {
			freeMem(newNode);
			return res;
		}
		a->base = putToEndList(a->base, newNode);
		writeLogStr("Add new list node and data struct");
	}
	return EVERYTHING_IS_OK;
}

u08 GetFromFrontArray(void* returnValue, const void* identifier) {
	DynamicArray_t* a = findArray(identifier);
	if(a == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
	void* head = peekFromFrontList(a->base);
	for(;head != NULL; head=peekFromFrontList(a->base)) {
		if(GetFromFrontDataStruct(returnValue, head) == EVERYTHING_IS_OK) return EVERYTHING_IS_OK;
		writeLogStr("Delete old list node and data struct");
		delDataStruct(head);
		freeMem(head);
		a->base = getFromFrontList(a->base, &head);
		if(a->base == NULL) return OVERFLOW_OR_EMPTY_ERROR;
	}
	if(head == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
	return UNDEFINED_BEHAVIOR;
}

u08 GetFromEndArray(void* returnValue, const void* identifier) {
	DynamicArray_t* a = findArray(identifier);
	if(a == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
	void* head = peekFromEndList(a->base);
	for(;head != NULL; head=peekFromEndList(a->base)) {
		if(GetFromEndDataStruct(returnValue, head) == EVERYTHING_IS_OK) return EVERYTHING_IS_OK;
		writeLogStr("Delete old list node and data struct");
		delDataStruct(head);
		freeMem(head);
		a->base = getFromEndList(a->base, &head);
		if(a->base == NULL) return OVERFLOW_OR_EMPTY_ERROR;
	}
	if(head == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
	return UNDEFINED_BEHAVIOR;
}

u08 peekFromFrontArray(void* returnValue, const void* identifier) {
	DynamicArray_t* a = findArray(identifier);
	if(a == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
	void* head = peekFromFrontList(a->base);
	for(;head != NULL; head=peekFromFrontList(a->base)) {
		if(peekFromFrontData(returnValue, head) == EVERYTHING_IS_OK) return EVERYTHING_IS_OK;
		writeLogStr("Delete old list node and data struct");
		delDataStruct(head);
		freeMem(head);
		a->base = getFromFrontList(a->base, &head);
		if(a->base == NULL) return OVERFLOW_OR_EMPTY_ERROR;
	}
	if(head == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
	return UNDEFINED_BEHAVIOR;
}

u08 peekFromEndArray(void* returnValue, const void* identifier) {
	DynamicArray_t* a = findArray(identifier);
	if(a == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
	void* head = peekFromEndList(a->base);
	for(;head != NULL; head=peekFromEndList(a->base)) {
		if(peekFromEndData(returnValue, head) == EVERYTHING_IS_OK) return EVERYTHING_IS_OK;
		writeLogStr("Delete old list node and data struct");
		delDataStruct(head);
		freeMem(head);
		a->base = getFromEndList(a->base, &head);
		if(a->base == NULL) return OVERFLOW_OR_EMPTY_ERROR;
	}
	if(head == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
	return UNDEFINED_BEHAVIOR;
}

u08 delFromFrontArray(const void* const identifier) {
	DynamicArray_t* a = findArray(identifier);
	if(a == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
	void* head = peekFromFrontList(a->base);
	for(;head != NULL; head=peekFromFrontList(a->base)) {
		if(delFromFrontDataStruct(head) == EVERYTHING_IS_OK) return EVERYTHING_IS_OK;
		writeLogStr("Delete old list node and data struct");
		delDataStruct(head);
		freeMem(head);
		a->base = getFromFrontList(a->base, &head);
		if(a->base == NULL) return OVERFLOW_OR_EMPTY_ERROR;
	}
	if(head == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
	return UNDEFINED_BEHAVIOR;
}

u08 delFromEndArray(const void* const identifier) {
	DynamicArray_t* a = findArray(identifier);
	if(a == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
	void* head = peekFromEndList(a->base);
	for(;head != NULL; head=peekFromEndList(a->base)) {
		if(delFromEndDataStruct(head) == EVERYTHING_IS_OK) return EVERYTHING_IS_OK;
		writeLogStr("Delete old list node and data struct");
		delDataStruct(head);
		freeMem(head);
		a->base = getFromEndList(a->base, &head);
		if(a->base == NULL) return OVERFLOW_OR_EMPTY_ERROR;
	}
	if(head == NULL) return NOT_FOUND_DATA_STRUCT_ERROR;
	return UNDEFINED_BEHAVIOR;
}

BaseSize_t getCurrentSizeArray(const void* const identifier) {
	DynamicArray_t* a = findArray(identifier);
	if(a == NULL) return 0;
	BaseSize_t sz = 0;
	ListNode_t* l = findHead(a->base);
	if(l == NULL) return sz;
	do {
		sz += getCurrentSizeDataStruct(l->data);
		l = l->next;
	} while(l != NULL);
	return sz;
}

void clearArray(const void* const identifier) {
	DynamicArray_t* a = findArray(identifier);
	if(a == NULL) return;
	ListNode_t* l = findHead(a->base);
	if(l == NULL) return;
	do {
		clearDataStruct(l->data);
		l = l->next;
	} while(l != NULL);
}

bool_t isEmptyArray(const void* const identifier) {
	return FALSE;
}

void forEachArray(const void* const identifier, TaskMng tsk) {
	DynamicArray_t* a = findArray(identifier);
	if(a == NULL) return;
	BaseSize_t sz = 0;
	ListNode_t* l = findHead(a->base);
	if(l == NULL) return;
	do {
		forEach(l->data,tsk);
		l = l->next;
	} while(l != NULL);
}
