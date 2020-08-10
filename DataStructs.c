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

/* *
 * dataStructs.c
 *
 *  Created on: 28 квіт. 2017 р.
 *      Author: oleksiy.khanin
 */


#include "TaskMngr.h"
#include "PlatformSpecific.h"

#ifdef __cplusplus
extern "C" {
#endif
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//                                  РЕАЛИЗАЦИЯ СТРУКТУР ДАННЫХ
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
#ifdef DATA_STRUCT_MANAGER
#if DATA_STRUCT_ArraySize > 0xFFFF
#error "incompatible size"
#endif
typedef struct
{
	const void* Data;               // Указатель на начало очереди
	BaseSize_t firstCount;    // Указатель на первый свободный элемент абстрактной структуры данных
	BaseSize_t lastCount;     // Указатель на последний фактический элемент в абстрактной структуре данных
	BaseSize_t sizeElement;   // Размер одного элемента абстрактной структуры данных в байтах
	BaseSize_t sizeAllElements;// Общий размер в количествах элементов в абстрактной структуре данных
} AbstractDataType;
volatile static AbstractDataType Data_Array[DATA_STRUCT_ArraySize];   // Собственно сам массив абстрактных структур данных

static inline BaseSize_t findNumberDataStruct(const void* const Data) {
	register BaseSize_t i = 0;
	for(; i<DATA_STRUCT_ArraySize; i++) { // находим абстрактную структуру данных
		if(Data_Array[i].Data == Data) break;
	}
	return i;
}

void initDataStruct(void) {  // Инициализация абстрактной структуры данных
	for(register BaseSize_t i = 0; i<DATA_STRUCT_ArraySize; i++) {
		Data_Array[i].Data = NULL;
		Data_Array[i].firstCount = 0;
		Data_Array[i].lastCount = 0;
		Data_Array[i].sizeElement = 0;
		Data_Array[i].sizeAllElements = 0;
	}
}

// Функция создает абстрактную структуру данных (резервирует место под нее в глобальном массиве)
// sizeElement - размер одного элемента в БАЙТАХ, sizeAll - размер очереди в ЭЛЕМЕНТАХ
u08 CreateDataStruct(const void* D, const BaseSize_t sizeElement, const BaseSize_t sizeAll) {
	register BaseSize_t i = 0;
	for(; i<DATA_STRUCT_ArraySize; i++) // Ищем пустое место в списке для новой структуры данных
	{
		if(Data_Array[i].Data == D) return OTHER_ERROR; // Если такая структура уже есть
		if(Data_Array[i].Data == NULL) break;
	}
	if(i == DATA_STRUCT_ArraySize) return NOT_FOUND_DATA_STRUCT_ERROR;
	const unlock_t unlock = lock((const void* const)(&Data_Array[i].Data));
	Data_Array[i].Data = (void*)D; // Адрес начала
	Data_Array[i].sizeElement = sizeElement; // размер одного элемента в байтах
	Data_Array[i].sizeAllElements = sizeAll; // Размер всей очереди в элементах
	Data_Array[i].firstCount= 0;
	Data_Array[i].lastCount = 0;
	unlock((const void* const)(&Data_Array[i].Data));
	return EVERYTHING_IS_OK;
}

// Удаляем абстрактную структуру данных
u08 delDataStruct(const void* Data) { // Удаляем из массива абстрактную структуру данных с заданным идентификатором
	BaseSize_t i = findNumberDataStruct(Data);
	if(i == DATA_STRUCT_ArraySize) return NOT_FOUND_DATA_STRUCT_ERROR;  // Если такой не существует в массиве, выдаем ошибку
	const unlock_t unlock = lock((const void* const)(&Data_Array[i].Data));
	Data_Array[i].Data = NULL;    // Если абстрактная структура данных есть удаляем ее
	unlock((const void* const)(&Data_Array[i].Data));
	return EVERYTHING_IS_OK;
}

static BaseSize_t incLast(volatile AbstractDataType* d) {
	BaseSize_t last = 0;
	while(last != d->lastCount) last=d->lastCount;
	if(!last) last = d->sizeAllElements;
	return last-1;
}

static BaseSize_t incFirst(volatile AbstractDataType* d) {
	BaseSize_t first = 0;
	while(first != d->firstCount) first = d->firstCount;
	if(first >= d->sizeAllElements-1) return 0;
	return first+1;
}

static BaseSize_t decLast(volatile AbstractDataType* d) {
	BaseSize_t last = 0;
	while(last != d->lastCount) last=d->lastCount;
	if(last >= d->sizeAllElements-1) return 0;
	return last+1;
}

static BaseSize_t decFirst(volatile AbstractDataType* d) {
	BaseSize_t first = 0;
	while(first != d->firstCount) first = d->firstCount;
	if(!first) first = d->sizeAllElements;
	return first-1;
}

u08 PutToCycleDataStruct(const void* Elem, const void* Array) {
	const BaseSize_t i = findNumberDataStruct(Array);
	if(i == DATA_STRUCT_ArraySize) return NOT_FOUND_DATA_STRUCT_ERROR;    // Если мы не нашли абстрактную структуру данных с указанным идентификтором выходим
	const unlock_t unlock = lock((const void* const)(&Data_Array[i].Data));
	const unsigned int offset = Data_Array[i].firstCount * Data_Array[i].sizeElement; //вычисляем смещение в байтах
	void* dst = (void*)((byte_ptr)Data_Array[i].Data + offset);     // Определяем адресс куда копировать
	memCpy(dst, Elem, Data_Array[i].sizeElement); // Вставляем наш элемент
	Data_Array[i].firstCount = incFirst(&Data_Array[i]);
	Data_Array[i].lastCount  = decLast(&Data_Array[i]);
	unlock((const void* const)(&Data_Array[i].Data));
	return EVERYTHING_IS_OK;
}

u08 GetFromCycleDataStruct(void* returnValue, const void* Array){
	const BaseSize_t i = findNumberDataStruct(Array);
	if(i == DATA_STRUCT_ArraySize) return NOT_FOUND_DATA_STRUCT_ERROR;    // Если в массиве нет искомой абстрактной структуры данных с заданным идентификатором
	const unlock_t unlock = lock((const void* const)(&Data_Array[i].Data));
	if(Data_Array[i].lastCount > 0) { // Если есть какие либо данные
		Data_Array[i].firstCount = decFirst(&Data_Array[i]);
		Data_Array[i].lastCount = incLast(&Data_Array[i]);
		const unsigned int offset = Data_Array[i].firstCount * Data_Array[i].sizeElement;  // Определяем смещение на элемент, который надо достать
		const void* dst = (void*)((byte_ptr)Data_Array[i].Data + offset);     // Записываем адрес памяти свободной ячейки
		memCpy(returnValue, dst, Data_Array[i].sizeElement);   // Если структура данных найдена, читаем от туда первый (самый старый) элемент
		unlock((const void* const)(&Data_Array[i].Data));  // Если все происходило не в прерывании восстанавливаем разрешение прерываний
		return EVERYTHING_IS_OK;   // Если все впорядке возвращаем ноль
	}
	*((byte_ptr)returnValue) = 0;
	unlock((const void* const)(&Data_Array[i].Data));  // Если все происходило не в прерывании восстанавливаем разрешение прерываний
	return OVERFLOW_OR_EMPTY_ERROR;
}

//Положить элемент Elem в начало структуры данных Array
u08 PutToFrontDataStruct(const void* Elem, const void* Array){
	const BaseSize_t i = findNumberDataStruct(Array);
	if(i == DATA_STRUCT_ArraySize) return NOT_FOUND_DATA_STRUCT_ERROR;    // Если мы не нашли абстрактную структуру данных с указанным идентификтором выходим
	const unlock_t unlock = lock((const void* const)(&Data_Array[i].Data));
	BaseSize_t frontCount = incFirst(&Data_Array[i]); 		  // Будущий указатель на СВОБОДНЫЙ элемент
	if(frontCount != Data_Array[i].lastCount) {
		const unsigned int offset = Data_Array[i].firstCount * Data_Array[i].sizeElement; //вычисляем смещение в байтах
		void* dst = (void*)((byte_ptr)Data_Array[i].Data + offset);     // Определяем адресс куда копировать
		memCpy(dst, Elem, Data_Array[i].sizeElement); // Вставляем наш элемент
		Data_Array[i].firstCount = frontCount;
		unlock((const void* const)(&Data_Array[i].Data));
		return EVERYTHING_IS_OK;
	}
	unlock((const void* const)(&Data_Array[i].Data));
	return OVERFLOW_OR_EMPTY_ERROR;  // Если после добавления мы догоним lastCount, значит структура заполнена
}

// Положить элемент Elem в конец абстрактной структуры данных Array
u08 PutToEndDataStruct(const void* Elem, const void* Array) {
	const BaseSize_t i = findNumberDataStruct(Array);
	if(i == DATA_STRUCT_ArraySize) return NOT_FOUND_DATA_STRUCT_ERROR;    // Если мы не нашли абстрактную структуру данных с указанным идентификтором выходим
	const unlock_t unlock = lock((const void* const)(&Data_Array[i].Data));
	BaseSize_t endCount = incLast(&Data_Array[i]);
	if(endCount != Data_Array[i].firstCount){
		const unsigned int offset = endCount * Data_Array[i].sizeElement;  // Определяем смещение на свободную позицию (количество байт)
		void* dst = (void*)((byte_ptr)Data_Array[i].Data + offset); // Записываем адрес памяти начала свободной ячейки
		memCpy(dst, Elem, Data_Array[i].sizeElement);  // Копируем все байты Elem в массив Array с заданным смещением
		Data_Array[i].lastCount = endCount;           // После копирования инкрементируем текущую позицию
		unlock((const void* const)(&Data_Array[i].Data));
		return EVERYTHING_IS_OK;
	}
	unlock((const void* const)(&Data_Array[i].Data));
	return OVERFLOW_OR_EMPTY_ERROR;  //Если после добавления струтктура переполнится не добавляем
}

u08 GetFromFrontDataStruct(void* returnValue, const void* Array){ // Достаем элемент с начала структуры данных
	const BaseSize_t i = findNumberDataStruct(Array);
	if(i == DATA_STRUCT_ArraySize) return NOT_FOUND_DATA_STRUCT_ERROR;    // Если в массиве нет искомой абстрактной структуры данных с заданным идентификатором
	const unlock_t unlock = lock((const void* const)(&Data_Array[i].Data));
	if(Data_Array[i].firstCount != Data_Array[i].lastCount) {
		Data_Array[i].firstCount = decFirst(&Data_Array[i]);
		const unsigned int offset = Data_Array[i].firstCount * Data_Array[i].sizeElement;  // Определяем смещение на элемент, который надо достать
		const void* dst = (void*)((byte_ptr)Data_Array[i].Data + offset);     // Записываем адрес памяти свободной ячейки
		memCpy(returnValue, dst, Data_Array[i].sizeElement);   // Если структура данных найдена, читаем от туда первый (самый старый) элемент
		unlock((const void* const)(&Data_Array[i].Data));// Если все происходило не в прерывании восстанавливаем разрешение прерываний
		return EVERYTHING_IS_OK;   // Если все впорядке возвращаем ноль
	}
	unlock((const void* const)(&Data_Array[i].Data));// Если все происходило не в прерывании восстанавливаем разрешение прерываний
	return OVERFLOW_OR_EMPTY_ERROR; // Если она пустая читать нечего
}

u08 GetFromEndDataStruct(void* returnValue, const void* Array) { // Достаем элемент с конца структуры данных
	const BaseSize_t i = findNumberDataStruct(Array);
	if(i == DATA_STRUCT_ArraySize) return NOT_FOUND_DATA_STRUCT_ERROR;    // Если в массиве нет искомой абстрактной структуры данных с заданным идентификатором
	const unlock_t unlock = lock((const void* const)(&Data_Array[i].Data));
	if(Data_Array[i].lastCount != Data_Array[i].firstCount) {
		const unsigned int offset = Data_Array[i].lastCount*Data_Array[i].sizeElement;
		const void* src = (void*)((byte_ptr)Data_Array[i].Data+offset);
		memCpy(returnValue, src, Data_Array[i].sizeElement);   // Если структура данных найдена, читаем от туда первый (самый старый) элемент
		Data_Array[i].lastCount = decLast(&Data_Array[i]);
		unlock((const void* const)(&Data_Array[i].Data));// Если все происходило не в прерывании восстанавливаем разрешение прерываний
		return EVERYTHING_IS_OK;   // Если все впорядке возвращаем ноль
	}
	unlock((const void* const)(&Data_Array[i].Data));// Если все происходило не в прерывании восстанавливаем разрешение прерываний
	return OVERFLOW_OR_EMPTY_ERROR; //Проверка пустая ли структура данных
}

u08 delFromFrontDataStruct(const void* const Data){
	const BaseSize_t i = findNumberDataStruct(Data);
	if(i == DATA_STRUCT_ArraySize) return NOT_FOUND_DATA_STRUCT_ERROR;    // Если в массиве нет искомой абстрактной структуры данных с заданным идентификатором
	const unlock_t unlock = lock((const void* const)(&Data_Array[i].Data));
	if(Data_Array[i].firstCount != Data_Array[i].lastCount) {
		Data_Array[i].firstCount = decFirst(&Data_Array[i]);
		unlock((const void* const)(&Data_Array[i].Data)); // Если все происходило не в прерывании восстанавливаем разрешение прерываний
		return EVERYTHING_IS_OK;   // Если все впорядке возвращаем ноль
	}
	unlock((const void* const)(&Data_Array[i].Data));// Если все происходило не в прерывании восстанавливаем разрешение прерываний
	return OVERFLOW_OR_EMPTY_ERROR; // Если она пустая читать нечего
}

u08 delFromEndDataStruct(const void* const Data) {
	const BaseSize_t i = findNumberDataStruct(Data);
	if(i == DATA_STRUCT_ArraySize) return NOT_FOUND_DATA_STRUCT_ERROR;    // Если в массиве нет искомой абстрактной структуры данных с заданным идентификатором
	const unlock_t unlock = lock((const void* const)(&Data_Array[i].Data));
	if(Data_Array[i].lastCount != Data_Array[i].firstCount) {
		Data_Array[i].lastCount = decLast(&Data_Array[i]);
		unlock((const void* const)(&Data_Array[i].Data));// Если все происходило не в прерывании восстанавливаем разрешение прерываний
		return EVERYTHING_IS_OK;   // Если все впорядке возвращаем ноль
	}
	unlock((const void* const)(&Data_Array[i].Data));// Если все происходило не в прерывании восстанавливаем разрешение прерываний
	return OVERFLOW_OR_EMPTY_ERROR; //Проверка пустая ли структура данных
}

u08 peekFromFrontData(void* returnValue, const void* Array) {
	const BaseSize_t i = findNumberDataStruct(Array);
	if(i == DATA_STRUCT_ArraySize) return NOT_FOUND_DATA_STRUCT_ERROR;    // Если в массиве нет искомой абстрактной структуры данных с заданным идентификатором
	const unlock_t unlock = lock((const void* const)(&Data_Array[i].Data));
	if(Data_Array[i].firstCount != Data_Array[i].lastCount) {
		u08 count = decFirst(&Data_Array[i]);
		const unsigned int offset = count * Data_Array[i].sizeElement;  // Определяем смещение на элемент, который надо достать
		const void* dst = (void*)((byte_ptr)Data_Array[i].Data + offset); // Записываем адрес памяти свободной ячейки
		memCpy(returnValue, dst, Data_Array[i].sizeElement);   // Если структура данных найдена, читаем от туда первый (самый старый) элемент
		unlock((const void* const)(&Data_Array[i].Data));// Если все происходило не в прерывании восстанавливаем разрешение прерываний
		return EVERYTHING_IS_OK;   // Если все впорядке возвращаем ноль
	}
	unlock((const void* const)(&Data_Array[i].Data));// Если все происходило не в прерывании восстанавливаем разрешение прерываний
	return OVERFLOW_OR_EMPTY_ERROR;// Если она пустая читать нечего
}

u08 peekFromEndData(void* returnValue, const void* Array) {
	const BaseSize_t i = findNumberDataStruct(Array);
	if(i == DATA_STRUCT_ArraySize) return NOT_FOUND_DATA_STRUCT_ERROR;    // Если в массиве нет искомой абстрактной структуры данных с заданным идентификатором
	const unlock_t unlock = lock((const void* const)(&Data_Array[i].Data));
	if(Data_Array[i].lastCount != Data_Array[i].firstCount) {
		const unsigned int offset = Data_Array[i].lastCount * Data_Array[i].sizeElement;
		const void* src = (void*)((byte_ptr)Data_Array[i].Data+offset);
		memCpy(returnValue, src, Data_Array[i].sizeElement);   // Если структура данных найдена, читаем от туда первый (самый старый) элемент
		unlock((const void* const)(&Data_Array[i].Data));// Если все происходило не в прерывании восстанавливаем разрешение прерываний
		return EVERYTHING_IS_OK;   // Если все впорядке возвращаем ноль
	}
	unlock((const void* const)(&Data_Array[i].Data));// Если все происходило не в прерывании восстанавливаем разрешение прерываний
	return OVERFLOW_OR_EMPTY_ERROR; //Проверка пустая ли структура данных
}

void clearDataStruct(const void * const Data){
	const BaseSize_t i = findNumberDataStruct(Data);
	if(i == DATA_STRUCT_ArraySize) return;
	const unlock_t unlock = lock((const void* const)(&Data_Array[i].Data));
	Data_Array[i].firstCount = 0; // Очищаем от данных наш массив
	Data_Array[i].lastCount = 0;
	unlock((const void* const)(&Data_Array[i].Data));
}

bool_t isEmptyDataStruct(const void* const Data){
	const BaseSize_t i = findNumberDataStruct(Data);
	if(i == DATA_STRUCT_ArraySize) return TRUE; // Если такой структуры нет она точно пустая
	return (bool_t)(Data_Array[i].firstCount == Data_Array[i].lastCount); // Если они равны друг другу значит пустая
}

BaseSize_t getCurrentSizeDataStruct(const void* const Data) {
	const BaseSize_t i = findNumberDataStruct(Data);
	if(i == DATA_STRUCT_ArraySize) return 0;
	BaseSize_t first = 0;
	BaseSize_t last = 0;
	while(first != Data_Array[i].firstCount) first = Data_Array[i].firstCount;
	while(last != Data_Array[i].lastCount) last = Data_Array[i].lastCount;
	if(last > first) return (first + (Data_Array[i].sizeAllElements-last));
	if(first > last) return (first - last);
	return 0;
}

void forEachDataStruct(const void* const Array, TaskMng tsk) {
	const BaseSize_t i = findNumberDataStruct(Array);
	if(i == DATA_STRUCT_ArraySize) return;
	BaseSize_t first = 0;
	BaseSize_t last = 0;
	while(first != Data_Array[i].firstCount) first = Data_Array[i].firstCount; // Первый свободный элемент структуры
	while(last != Data_Array[i].lastCount) last = Data_Array[i].lastCount; // Последний фактический элемент структуры
	if(first == last) return;
	if(first > last) {
		do{
			if(first) first--;
			else first=Data_Array[i].sizeAllElements-1;
			BaseSize_t offset = first*Data_Array[i].sizeElement;
			BaseParam_t ptr = (BaseParam_t)((byte_ptr)Data_Array[i].Data + offset);
			if(tsk != NULL) tsk(0,ptr);
		}while(first > last);
	} else {
		do {
			BaseParam_t ptr = (BaseParam_t)((byte_ptr)Data_Array[i].Data + last*Data_Array[i].sizeElement);
			if(tsk != NULL) tsk(0,ptr);
			last++;
			if(last >= Data_Array[i].sizeAllElements) last = 0;
		}while(first < last);
	}
}

#endif //DATA_STRUCT_MANAGER


#ifdef __cplusplus
}
#endif
