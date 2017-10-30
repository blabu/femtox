/*
 * List.h
 *
 *  Created on: 17 жовт. 2017 р.
 *      Author: oleksiy.khanin
 */

#ifndef LIST_H_
#define LIST_H_

#include "TaskMngr.h"

#ifdef _LIST_STRUCT

typedef struct node {
	struct node* prev;
	struct node* next;
	void* data;
}Node_t;

Node_t* createNewList(void* data);
void deleteList(Node_t* listPtr);
Node_t* findHead(Node_t* listPtr);

// flag 0...6 bits is a size of data, and last 7's bit is a flag for allocate memmory
Node_t* putToEndList(Node_t* list, void* data, u08 Flagalloc_Datasize);
Node_t* putToFrontList(Node_t* list, void* data, u08 Flagalloc_Datasize);
Node_t* getFromEndList(Node_t* list, void** result);
Node_t* getFromFrontList(Node_t* list, void** result);
BaseSize_t getSizeList(Node_t* list);
void ForEachListNodes(Node_t* list, TaskMng task, bool_t flagToManager, BaseSize_t arg_n);
#endif //_LIST_STRUCT

#endif /* LIST_H_ */
