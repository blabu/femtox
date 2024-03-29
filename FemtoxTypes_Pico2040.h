/*
 * types.h
 *
 *  Created on: 18 лип. 2018 р.
 *      Author: Admin
 */

#ifndef FEMTOXTYPES_ARM_H_
#define FEMTOXTYPES_ARM_H_

#include "FemtoxConf.h"

#ifdef PICO2040

#ifndef NULL
#define NULL ((void*)0)
#endif

#define PAIR(T,V) struct{T first; V second;}
typedef char* string_t;
typedef unsigned long long u64;
typedef long long s64;
typedef unsigned int   u32;
typedef unsigned short u16;
typedef unsigned char  u08;
typedef signed int     s32;
typedef signed short   s16;
typedef signed char    s08;
typedef unsigned int   Time_t;
typedef enum {FALSE=0, TRUE = !FALSE} bool_t;

typedef unsigned char* byte_ptr;
typedef unsigned int  BaseSize_t; // Первый аргумент для задачи в диспетчере
typedef void* BaseParam_t;  // Второй аргумент для задачи в диспетчере
extern const BaseSize_t _MAX_BASE_SIZE;

typedef void(*unlock_t)(const void* const resourceId);
typedef void (*IdleTask_t)(void);      // Указатель на функцию обработки холостого хода void funcIDLE(void)
typedef void (*CycleFuncPtr_t)(void);  // Указатель на функцию void func(void). Для циклического выполнения в прерывании таймера
typedef bool_t (*Predicat_t)(void);    // Указатель на функцию предикат (bool_t func1(void))
typedef void (*TaskMng)(BaseSize_t arg_n, BaseParam_t arg_p);  // Объявляем пользовательский тип данных - указатель на функцию.
// Каждая задача имеет два параметра, которые определяют количество параметров для функции, и адрес в памяти начиная с которого их считать
// Если arg_n == 1, то arg_p стоит считать не указателем, а значением параметра (передача по значению одного элемента)
// Отметим, что имя функции является ее указателем.
// Вызвать любую функцию можно двояко:
//  1. Стандартным способом через ее имя и список параметров Например, shov1();
//  2. Через указатель на функцию. К примеру, (*show1)() - операция разыменовывания указателя на функцию;

typedef struct {
	TaskMng Task;         //Указателей на функции, которая является задачей и принимает два параметра (количество аргументов и адрес первого из них)
	BaseSize_t   arg_n; // первый аргумент (Количество принимаемых аргументов)
	BaseParam_t arg_p; // второй аргумент (Указатель на начало массива аргументов)
} TaskList_t;

typedef struct {
	u08 sec;  //Начинаются с 0
	u08 min;  //Начинаются с 0
	u08 hour;  //Начинаются с 0
	u08 day;  //Начинаются с 1
	u08 mon;  //Начинаются с 1
	u16 year;
} Date_t;

#if MUTEX_SIZE <= 8
typedef u08 mutexType;
#elif MUTEX_SIZE <= 16
typedef u16 mutexType;
#elif MUTEX_SIZE <= 32
typedef u32 mutexType;
#else
#error "Too long size for mutex"
#endif

#ifdef _LIST_STRUCT
typedef struct node {
	struct node* prev;
	struct node* next;
	void* data;
}ListNode_t;
#endif

#ifdef _DYNAMIC_ARRAY

#endif

#endif //ARM_STM32

#endif /* FEMTOXTYPES_ARM_H_ */
