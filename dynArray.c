/*
 * dynString.c
 *
 *  Created on: 17 жовт. 2017 р.
 *      Author: oleksiy.khanin
 */

#include "dynArray.h"

#ifdef _DYNAMIC_ARRAY
#ifdef __cplusplus
extern "C" {
#endif

DynamicArray_t* createNewDynamicArray(u08 capasity) {
	if(capasity > 0x7F) return NULL; // Size is to big
	DynamicArray_t* res = (DynamicArray_t*)allocMem(sizeof(DynamicArray_t));
	if(res == NULL) return res; // Memmory allocate error
	res->data = (u08*)allocMem(capasity);
	if(res->data == NULL) {
		freeMem((byte_ptr)res);
		return NULL;
	}
	res->capasity = capasity;
	res->size = 0;
	return res;
}

void deleteDynamicArray(DynamicArray_t* array) {
	if(array == NULL) return;
	freeMem(array->data);
	freeMem((byte_ptr)array);
}

u08 append(DynamicArray_t* array, u08* data, u08 DataSize) {
	if(array == NULL || data == NULL || DataSize > 0x7F) return OTHER_ERROR;
	if((array->size + DataSize)>0x7F) return OVERFLOW_OR_EMPTY_ERROR; // Can't allocate memmory for this data
	if((array->capasity - array->size) < DataSize) { // need reallocate memmory
		u08* tempData = (u08*)allocMem((array->size + DataSize));
		if(tempData == NULL) return OTHER_ERROR;
		for(u08 i = 0; i<(array->size); i++) tempData[i] = array->data[i];
		freeMem(array->data);
		array->data = tempData;
	}
	for(u08 i = array->size; i<(array->size + DataSize); i++) array->data[i] = data[i];
	return EVERYTHING_IS_OK;
}

#ifdef __cplusplus
}
#endif


#endif //_DYNAMIC_ARRAY
