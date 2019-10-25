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

ListNode_t* findHead(const ListNode_t* listPtr) {
	if(listPtr == NULL) return NULL;
	while(listPtr->prev != NULL) {
        listPtr = listPtr->prev;
	}
	return (ListNode_t*)listPtr;
}

ListNode_t* findTail(const ListNode_t* listPtr) {
	if(listPtr == NULL) return NULL;
	while(listPtr->next != NULL) {
       listPtr = listPtr->next;
	}
	return (ListNode_t*)listPtr;
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

void deleteListNode(ListNode_t* listPtr) {
	if(listPtr == NULL) return;
	ListNode_t* prev = listPtr->prev;
	ListNode_t* next = listPtr->next;
	next->prev = prev;
	prev->next = next;
	freeMem(listPtr->data);
	freeMem(listPtr);
}

ListNode_t* putToEndList(ListNode_t* list, void* data) {
	if(list == NULL || data == NULL) {
		writeLogStr("ERROR putToEndList Incorrect input value");
		return NULL;
	}
	ListNode_t* newNode = (ListNode_t*)allocMem(sizeof(ListNode_t));
	if(newNode == NULL) {
		writeLogStr("ERROR putToEndList Memory error");
		return NULL;
	}
	list = findTail(list);
	newNode->data = data;
    list->next = newNode;
	newNode->prev = list;
	newNode->next = NULL;
	return newNode;
}

ListNode_t* putToFrontList(ListNode_t* list, void* data) {
	if(list == NULL || data == NULL){
		writeLogStr("ERROR putToFrontList Incorrect input value");
		return NULL;
	}
	ListNode_t* newNode = (ListNode_t*)allocMem(sizeof(ListNode_t));
	if(newNode == NULL) {
		writeLogStr("ERROR putToFrontList Memory error");
		return NULL;
	}
	list = findHead(list);
	newNode->data = data;
	list->prev = newNode;
    newNode->next = list;
    newNode->prev = NULL;
	return newNode;
}

ListNode_t* getFromEndList(ListNode_t* list, void** result){
	if(list != NULL) {
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
	writeLogStr("ERROR getFromEndList Incorrect input value");
	return NULL;
}

ListNode_t* getFromFrontList(ListNode_t* list, void** result){
	if(list != NULL) {
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
   	writeLogStr("ERROR getFromFrontList Incorrect input value");
   	return NULL;
}

void* peekFromFrontList(ListNode_t* list) {
	if(list != NULL) {
		ListNode_t *head = findHead(list);
		if(head != NULL) {
			return head->data;
		}
		return NULL;
	}
	return NULL;
}

void* peekFromEndList(ListNode_t* list) {
	if(list != NULL) {
		ListNode_t *tail = findTail(list);
		if(tail != NULL) {
			return tail->data;
		}
		return NULL;
	}
	return NULL;
}


BaseSize_t getSizeList(ListNode_t* list){
	if(list != NULL) {
		ListNode_t * head = findHead(list);
		if(head != NULL) {
			BaseSize_t size = 1;
			while(head->next != NULL) {
				head = head->next;
				size++;
			}
			return size;
		}
	}
    return 0;
}

void forEachListNodes(ListNode_t* list, TaskMng task, bool_t flagToManager, BaseSize_t arg_n) {
	if(list != NULL) {
		ListNode_t* head = findHead(list);
		while(head != NULL) {
			if(flagToManager) SetTask(task, arg_n, (BaseParam_t)head->data);
			else task(arg_n, (BaseParam_t)head->data);
			head = head->next;
		}
	}
}

#ifdef __cplusplus
}
#endif


#endif // _LIST_STRUCT
