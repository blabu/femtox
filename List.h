/*
 * List.h
 *
 *  Created on: 17 жовт. 2017 р.
 *      Author: oleksiy.khanin
 */

#ifndef LIST_H_
#define LIST_H_

#include "TaskMngr.h"
#include "FemtoxTypes.h"

#ifdef _LIST_STRUCT


////TODO DELETE IT FROM HERE (DEFINED IN FemtoxTypes)
//#ifdef _LIST_STRUCT
//typedef struct node {
//	struct node* prev;
//	struct node* next;
//	void* data;
//}ListNode_t;
//#endif

ListNode_t* createNewList(void* data);
void deleteList(ListNode_t* listPtr);
void deleteListNode(ListNode_t* listPtr);
ListNode_t* findHead(const ListNode_t* listPtr);
ListNode_t* findTail(const ListNode_t* listPtr);
// flag 0...6 bits is a size of data, and last 7's bit is a flag for allocate memmory
ListNode_t* putToEndList(ListNode_t* list, void* data);
ListNode_t* putToFrontList(ListNode_t* list, void* data);
ListNode_t* getFromEndList(ListNode_t* list, void** result);
ListNode_t* getFromFrontList(ListNode_t* list, void** result);
void* peekFromFrontList(ListNode_t* list);
void* peekFromEndList(ListNode_t* list);
BaseSize_t getSizeList(ListNode_t* list);
void forEachListNodes(ListNode_t* list, TaskMng task, bool_t flagToManager, BaseSize_t arg_n);
#endif //_LIST_STRUCT

#endif /* LIST_H_ */
