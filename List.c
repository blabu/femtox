/*
 * List.c
 *
 *  Created on: 17 жовт. 2017 р.
 *      Author: oleksiy.khanin
 */


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

void deleteList(Node_t* listPtr) {
	if(listPtr == NULL) return;
	Node_t* temp = findHead(listPtr);
	while(temp->next != NULL) {
		freeMem(temp->data);
		temp = temp->next;
		freeMem(temp->prev);
	}
	freeMem(listPtr);
}

// flag 0...6 bits is a size of data, and last 7's bit is a flag for allocate memmory
u08 putToEndList(Node_t* list, void* data, u08 Flagalloc_Datasize) {
	if(list == NULL || data == NULL) return NOT_FAUND_DATA_STRUCT_ERROR;
	Node_t* newNode = (Node_t*)allocMem(sizeof(Node_t));
	if(newNode == NULL) return OTHER_ERROR;
	u08* temp = NULL;
	while(list->next != NULL) list = list->next;
	list->next = newNode;
	newNode->prev = list;
	newNode->next = NULL;
	if(Flagalloc_Datasize>>7) {
		temp = (u08*)allocMem(Flagalloc_Datasize & 0x7F);
		if(temp == NULL) {
			freeMem((byte_ptr)newNode);
			return OTHER_ERROR;
		}
		for(u08 i = 0; i< (Flagalloc_Datasize & 0x7F); i++) temp[i] = ((u08*)data)[i];
		newNode->data = temp;
	}
	else newNode->data = data;
	return EVERYTHING_IS_OK;
}

u08 putToFrontList(Node_t* list, void* data, u08 Flagalloc_Datasize) {
	if(list == NULL || data == NULL) return NOT_FAUND_DATA_STRUCT_ERROR;
	Node_t* newNode = (Node_t*)allocMem(sizeof(Node_t));
	if(newNode == NULL) return OTHER_ERROR;
	u08* temp = NULL;
	while(list->prev != NULL) list = list->prev;
	list->prev = newNode;
	newNode->next = list;
	newNode->prev = NULL;
	if(Flagalloc_Datasize>>7) {
		temp = (u08*)allocMem(Flagalloc_Datasize & 0x7F);
		if(temp == NULL) {
			freeMem(newNode);
			return OTHER_ERROR;
		}
		for(u08 i = 0; i<(Flagalloc_Datasize & 0x7F); i++) temp[i] = ((u08*)data)[i];
		newNode->data = temp;
	}
	else newNode->data = data;
	return EVERYTHING_IS_OK;
}

#endif // _LIST_STRUCT
