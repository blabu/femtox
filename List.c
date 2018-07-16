/*
 * List.c
 *
 *  Created on: 17 жовт. 2017 р.
 *      Author: oleksiy.khanin
 */
#ifdef __cplusplus
extern "C" {
#endif

#include "List.h"
#include "logging.h"

#ifdef _LIST_STRUCT

ListNode_t* createNewList(void* data){
	ListNode_t* res = (ListNode_t*)allocMem(sizeof(ListNode_t));
	if(res == NULL) return res; // Memmory allocate error
	res->data = data;
	res->prev = NULL;
	res->next = NULL;
	return res;
}

ListNode_t* findHead(ListNode_t* listPtr) {
	if(listPtr == NULL) return NULL;
	while(listPtr->prev != NULL) {
        listPtr = listPtr->prev;
	}
	return listPtr;
}

ListNode_t* findTail(ListNode_t* listPtr) {
	if(listPtr == NULL) return NULL;
	while(listPtr->next != NULL) {
            listPtr = listPtr->next;
	}
	return listPtr;
}

void deleteList(ListNode_t* listPtr) {
	if(listPtr == NULL) return;
	ListNode_t* temp = findHead(listPtr);
	while(temp->next != NULL) {
		freeMem(temp->data);
		temp = temp->next;
		freeMem((byte_ptr)temp->prev);
	}
	freeMem((byte_ptr)temp->data);
	freeMem((byte_ptr)temp);
	freeMem((byte_ptr)listPtr);
}

// flag 0...6 bits is a size of data, and last 7's bit is a flag for allocate memmory
ListNode_t* putToEndList(ListNode_t* list, void* data, u08 Flagalloc_Datasize) {
	if(list == NULL || data == NULL) {
		writeLogStr("putToEndList Incorrect input value");
		return NULL;
	}
	ListNode_t* newNode = (ListNode_t*)allocMem(sizeof(ListNode_t));
	if(newNode == NULL) {
		writeLogStr("putToEndList Memory error");
		return NULL;
	}
	byte_ptr temp;
	list = findTail(list);
	if(Flagalloc_Datasize>>7) {
		u08 size = Flagalloc_Datasize & 0x7F;
		temp = (byte_ptr)allocMem(size);
		if(temp == NULL) {
			writeLogStr("Error when try alloc mem for data");
			freeMem((byte_ptr)newNode);
			return NULL;
		}
		memCpy(temp, data, size);
		newNode->data = temp;
	}
	else newNode->data = data;
    list->next = newNode;
	newNode->prev = list;
	newNode->next = NULL;
	return newNode;
}

ListNode_t* putToFrontList(ListNode_t* list, void* data, u08 Flagalloc_Datasize) {
	if(list == NULL || data == NULL){
		writeLogStr("putToFrontList Incorrect input value");
		return NULL;
	}
	ListNode_t* newNode = (ListNode_t*)allocMem(sizeof(ListNode_t));
	if(newNode == NULL) {
		writeLogStr("putToFrontList Memory error");
		return NULL;
	}
	byte_ptr temp = NULL;
	list = findHead(list);
	if(Flagalloc_Datasize>>7) {
		temp = (byte_ptr)allocMem(Flagalloc_Datasize & 0x7F);
		if(temp == NULL) {
			freeMem((byte_ptr)newNode);
			return NULL;
		}
		memCpy(temp, data, (Flagalloc_Datasize & 0x7F));
		newNode->data = temp;
	}
	else newNode->data = data;
	list->prev = newNode;
    newNode->next = list;
    newNode->prev = NULL;
	return newNode;
}

ListNode_t* getFromEndList(ListNode_t* list, void** result){
    if(list == NULL) {
    	writeLogStr("getFromEndList Incorrect input value");
    	return NULL;
    }
    ListNode_t* tail = findTail(list); // Ищем последний элемент
    ListNode_t* prev = NULL;
    *result = tail->data; // Сохраняем указатель на данные
    if(tail->prev != NULL) {
    		prev = tail->prev;
            prev->next = NULL;
    }
    freeMem((byte_ptr)tail);
    return prev;
}

ListNode_t* getFromFrontList(ListNode_t* list, void** result){
    if(list == NULL) {
    	writeLogStr("getFromFrontList Incorrect input value");
    	return NULL;
    }
    ListNode_t* head = findHead(list); // Ищем последний элемент
    ListNode_t* next = NULL;
    *result = head->data;
    if(head->next != NULL) {
        head->next->prev = NULL;
        next = head->next;
    }
    freeMem((byte_ptr)head);
    return next;
}

BaseSize_t getSizeList(ListNode_t* list){
    if(list == NULL) return 0;
    BaseSize_t size = 0;
	ListNode_t * head = findHead(list);
	if(head != NULL) {
        while(head->next != NULL) size++;
	}
    return size;
}

void ForEachListNodes(ListNode_t* list, TaskMng task, bool_t flagToManager, BaseSize_t arg_n) {
    if(list == NULL) return;
	ListNode_t* head = findHead(list);
	if(head != NULL) {
		do {
			if(flagToManager)SetTask(task, arg_n, (BaseParam_t)head->data);
			else task(arg_n, (BaseParam_t)head->data);
			head = head->next;
		}while(head != NULL);
	}
}

#ifdef __cplusplus
}
#endif


#endif // _LIST_STRUCT
