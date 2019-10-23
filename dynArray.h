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

u08 CreateArray(const void* D, const BaseSize_t sizeElement, const BaseSize_t sizeAll);
u08 delArray(const void* Data);                                    // Удаляем структуру из списка структур
BaseSize_t getCurrentSizeArray(const void* const Data);
u08 PutToFrontArray(const void * Elem, const void* Array);   // Кладем элемент в начало
u08 PutToEndArray(const void* Elem, const void* Array);     // Кладем элемент в конец
u08 GetFromFrontArray(void* returnValue, const void* Array);// Достаем элемент с начала структуры
u08 GetFromEndArray(void* returnValue, const void* Array); // Достаем элемент с конца структуры данных
u08 delFromFrontArray(const void* const Data); // Удаляет один элемент из структуры данных Data с начала
u08 delFromEndArray(const void* const Data); // Удаляет один элемент из структуры данных Data с конца
u08 peekFromFrontArray(void* returnValue, const void* Array); // Посмотреть первый элемент очереди не удаляя его
u08 peekFromEndArray(void* returnValue, const void* Array);  // Посмотреть последний элемент очереди не удаляя его
bool_t isEmptyArray(const void* const Data); // Проверяет пустая ли структура данных
void for_eachArray(const void* const Array, TaskMng tsk);
void clearArray(const void* const Data); // Очистить структуру данных с указателем Data

#endif // _DYNAMIC_ARRAY
#endif /* DYNARRAY_H_ */
