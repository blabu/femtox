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

#ifdef _LIST_STRUCT

Node_t* createNewList(void* data){
	Node_t* res = (Node_t*)allocMem(sizeof(Node_t));
	if(res == NULL) return res; // Memmory allocate error
	res->data = data;
	res->prev = NULL;
	res->next = NULL;
	return res;
}

Node_t* findHead(Node_t* listPtr) {
	while(listPtr->prev != NULL) listPtr = listPtr->prev;
	return listPtr;
}

Node_t* findTail(Node_t* listPtr) {
	while(listPtr->next != NULL) listPtr = listPtr->next;
	return listPtr;
}

void deleteList(Node_t* listPtr) {
	if(listPtr == NULL) return;
	Node_t* temp = findHead(listPtr);
	while(temp->next != NULL) {
		freeMem(temp->data);
		temp = temp->next;
		freeMem(temp->prev);
	}
	freeMem(temp->data);
	freeMem(temp);
	freeMem(listPtr);
}

// flag 0...6 bits is a size of data, and last 7's bit is a flag for allocate memmory
Node_t* putToEndList(Node_t* list, void* data, u08 Flagalloc_Datasize) {
	if(list == NULL || data == NULL) return NULL;
	Node_t* newNode = (Node_t*)allocMem(sizeof(Node_t));
	if(newNode == NULL) return NULL;
	u08* temp = NULL;
	while(list->next != NULL) list = list->next;
	list->next = newNode;
	newNode->prev = list;
	newNode->next = NULL;
	if(Flagalloc_Datasize>>7) {
		temp = (u08*)allocMem(Flagalloc_Datasize & 0x7F);
		if(temp == NULL) {
			freeMem((byte_ptr)newNode);
			return NULL;
		}
		memCpy(temp, data, (Flagalloc_Datasize & 0x7F));
		newNode->data = temp;
	}
	else newNode->data = data;
	return newNode;
}

Node_t* putToFrontList(Node_t* list, void* data, u08 Flagalloc_Datasize) {
	if(list == NULL || data == NULL) return NULL;
	Node_t* newNode = (Node_t*)allocMem(sizeof(Node_t));
	if(newNode == NULL) return NULL;
	u08* temp = NULL;
	while(list->prev != NULL) list = list->prev;
	list->prev = newNode;
	newNode->next = list;
	newNode->prev = NULL;
	if(Flagalloc_Datasize>>7) {
		temp = (u08*)allocMem(Flagalloc_Datasize & 0x7F);
		if(temp == NULL) {
			freeMem(newNode);
			return NULL;
		}
		memCpy(temp, data, (Flagalloc_Datasize & 0x7F));
		newNode->data = temp;
	}
	else newNode->data = data;
	return newNode;
}

Node_t* getFromEndList(Node_t* list, void** result){
    if(list == NULL) return NULL;
    Node_t* tail = findTail(list); // Ищем последний элемент
    *result = tail->data;
    tail->prev->next = NULL;
    freeMem(tail);
    return tail->prev;
}

Node_t* getFromFrontList(Node_t* list, void** result){
    if(list == NULL) return NULL;
    Node_t* head = findHead(list); // Ищем последний элемент
    *result = head->data;
    head->next->prev = NULL;
    freeMem(head);
    return head->next;
}

BaseSize_t getSizeList(Node_t* list){
    if(list == NULL) return 0;
    BaseSize_t size = 0;
	Node_t * head = findHead(list);
    while(head->next != NULL) size++;
    return size;
}

#ifdef __cplusplus
}
#endif


#endif // _LIST_STRUCT
