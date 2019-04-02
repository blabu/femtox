/*
 * FemtoxConf_ESP32.h
 *
 *  Created on: 26 мар. 2019 г.
 *      Author: blabu
 */

#ifndef FEMTOX_FEMTOXCONF_ESP32_H_
#define FEMTOX_FEMTOXCONF_ESP32_H_

#define SET_FRONT_TASK_ENABLE  /*разрешаем добавлеие в голову очереди задач (высокоприоритетная задача)*/
#define DATA_STRUCT_MANAGER   /*Включаем работу с очередями средствами деспетчера*/
#define CYCLE_FUNC  /*Разрешение работы циклически выполняемых программ в прерывании системного таймера*/
#define MUTEX_ENABLE /*Включаем поддержку мьютексов*/
#define MAXIMIZE_OVERFLOW_ERROR  /*При переполнении очереди задач и или таймеров система заглохнет (максимизация оибки)*/
//#define ALLOC_MEM   /*Включение динамического выделения памяти*/
#define ALLOC_MEM_LARGE  /*Включение динамического выделения памяти без ограничения размера*/
#define EVENT_LOOP_TASKS
//#define USE_SOFT_UART
#define CLOCK_SERVICE
#define GLOBAL_FLAGS
#define CALL_BACK_TASK
#define SIGNALS_TASK
#define _LIST_STRUCT
#define _DYNAMIC_ARRAY
#define _PWR_SAVE
#define NEED_CRYPT
#define NEED_BASE64
#define NEED_RANDOM
#define NEED_CRC16
//#define ENABLE_LOGGING
#define NEED_MATRIX
//#define STANDART_MEMCPY_MEMSET
#define LOAD_STATISTIC /*Сколько времени мы находимся в IDLE процессе в сравнении с полным рабочим временем в секундах*/

#define TASK_LIST_LEN 10U /*Длина очереди задач*/
#define TIME_LINE_LEN 30U /*Максимальне количество системных таймеров*/
#define TIME_DELAY_IF_BUSY 5U /*Задержка на повторную попытку поставить задачу в очередь или захватить мьютекс*/

#ifdef EVENT_LOOP_TASKS
#define EVENT_LIST_SIZE 5
#endif

#ifdef  DATA_STRUCT_MANAGER
#define ArraySize   12 /*Общее количество всех структур данных*/
#endif

#ifdef MUTEX_ENABLE
#define MUTEX_SIZE 8 /*Может быть 8,16,32 бита, и соответсвенно столько же мьютексов*/
#endif

#ifdef CYCLE_FUNC
#define TIMERS_ARRAY_SIZE 10
#endif

#ifdef ALLOC_MEM
#define HEAP_SIZE 7500UL /*6500*/
#endif
#ifdef ALLOC_MEM_LARGE
#define HEAP_SIZE 10000UL /*6500*/
#endif

#ifdef CALL_BACK_TASK
//#define CHECK_ERRORS_CALLBACK
#define CALL_BACK_TASK_LIST_LEN 30
#endif

#ifdef SIGNALS_TASK
#define SIGNAL_LIST_LEN 2
#endif

#ifdef _PWR_SAVE
//   #define NATIVE_TIMER_PWR_SAVE /*Реализация динамического изменения частоты таймера нативным способом*/
#endif

#ifdef USE_SOFT_UART
  #define SOFT_UART_WORK_FLAG 1<<2
  #define UART_NUMB 1    /*Колличество программных ЮАРТов*/
  #define BAUD_300   127
  #define BAUD_600   64
  #define BAUD_1200  32
  #define BAUD_2400  16
  #define BAUD_4800  8
  #define BAUD_9600  4
  #define DATA_BITS  8   /*Количество бит данных в посылке*/
  #define STOP_BITS  1   /*Колличество СТОП битов*/
#endif

#endif /* FEMTOX_FEMTOXCONF_ESP32_H_ */
