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

u08 CreateArray(const void*const identifier, const BaseSize_t sizeElement, const BaseSize_t sizeAll, BaseSize_t dataRice);
u08 delArray(const void* identifier);                                    // Удаляем структуру из списка структур
BaseSize_t getCurrentSizeArray(const void* const identifier);
u08 PutToFrontArray(const void * Elem, const void* identifier);   // Кладем элемент в начало
u08 PutToEndArray(const void* Elem, const void* identifier);     // Кладем элемент в конец
u08 GetFromFrontArray(void* returnValue, const void* identifier);// Достаем элемент с начала структуры
u08 GetFromEndArray(void* returnValue, const void* identifier); // Достаем элемент с конца структуры данных
u08 delFromFrontArray(const void* const identifier); // Удаляет один элемент из структуры данных Data с начала
u08 delFromEndArray(const void* const identifier); // Удаляет один элемент из структуры данных Data с конца
u08 peekFromFrontArray(void* returnValue, const void* identifier); // Посмотреть первый элемент очереди не удаляя его
u08 peekFromEndArray(void* returnValue, const void* identifier);  // Посмотреть последний элемент очереди не удаляя его
bool_t isEmptyArray(const void* const identifier); // Проверяет пустая ли структура данных
void forEachArray(const void* const identifier, TaskMng tsk);
void clearArray(const void* const identifier); // Очистить структуру данных с указателем Data

#endif // _DYNAMIC_ARRAY
#endif /* DYNARRAY_H_ */
