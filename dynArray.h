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

DynamicArray_t* createNewDynamicArray(u08 capasity);
void deleteDynamicArray(DynamicArray_t* array); // delete all data and array pointer
void freeDynamicArray(DynamicArray_t* array);   // free memory only for data. Array pointer still valid
u08 arrayAppend(DynamicArray_t* array, u08* data, u08 DataSize);

#endif // _DYNAMIC_ARRAY
#endif /* DYNARRAY_H_ */
