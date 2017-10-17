/*
 * dynArray.h
 *
 *  Created on: 17 жовт. 2017 р.
 *      Author: oleksiy.khanin
 */

#ifndef DYNARRAY_H_
#define DYNARRAY_H_

#include "TaskMngr.h"

#ifdef _DYNAMIC_ARRAY

typedef struct {
	u08 size;		// Текущее кол-во элементов
	u08 capasity; 	// Текуший размер выделенной области
	u08* data;		// Указатель на начало области
}DynamicArray_t;

DynamicArray_t* createNewDynamicArray(u08 capasity);
void deleteDynamicArray(DynamicArray_t* array);
u08 append(DynamicArray_t* array, u08* data, u08 DataSize);

#endif // _DYNAMIC_ARRAY
#endif /* DYNARRAY_H_ */
