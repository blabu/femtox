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
ListNode_t* peekFromFrontList(ListNode_t* list);
ListNode_t* peekFromEndList(ListNode_t* list);
BaseSize_t getSizeList(ListNode_t* list);
void forEachListNodes(ListNode_t* list, TaskMng task, bool_t flagToManager, BaseSize_t arg_n);
#endif //_LIST_STRUCT

#endif /* LIST_H_ */
