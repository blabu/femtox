#ifndef Task_Manager
#define Task_Manager

#include "FemtoxTypes.h"
#include "FemtoxConf.h"

extern const char* const _osVersion;

#define ABS(XX) (((XX) > 0)?(XX):(-(XX)))

void initFemtOS(void);    /* Инициализация менеджера задач. Здесь весь список задач (масив TaskLine) иницмализируется функцией Idle*/
void ResetFemtOS(void);  // Програмный сброс микроконтроллера
void runFemtOS( void ); // Запуск операционной системы
void SetTask (TaskMng New_Task, BaseSize_t n, BaseParam_t data); /* Функция помещает в конец очереди задачу New_Task с количеством параметров n. И параметрами data[n]
Прочесываем всю очередь задач в поисках пустой функции (Idle). Когда нашли засовываем вместо нее новую задачу
и выходим из функции. Если не нашли пустой функции и засовывать задачу некуда просто выходим из функции. Можно также возвращать
код ошибки если не удалось записать задачу и нормального завершения. Тогда функция будет иметь тип не void, а uint8_t.
 */

bool_t isEmptyTaskList( void );
u08 getFreePositionForTask(void);
u08 getFreePositionForTimerTask(void);

#ifdef SET_FRONT_TASK_ENABLE
void SetFrontTask (TaskMng New_Task, BaseSize_t n, BaseParam_t data); // Поставить задачу в начало очереди
#endif
//Сами задачи следует делать небольшими.

void delAllTask(void);

//********************************СИСТЕМНЫЙ ТАЙМЕР*********************************************
void TimerISR(void); //Обработчик прерывания по совпадению теущего значения таймера и счетчика.

void SetTimerTask(TaskMng TPTR, BaseSize_t n, BaseParam_t data, Time_t New_Time); //Постановщик задач c количеством параметров n в таймер.
/*Функция устанавливающая задачу в очередь по таймеру. На входе адрес перехода (имя задачи) и время в тиках службы таймера.
Время двухбайтное, т.е. от 1 до 65535 измеряеться в переплнениях таймера0. Не имеет значение
какой таймер использовать. Если в очереди таймеров уже есть таймер с такой задачей, то происходит апдейт времени. Две одинаковых задачи в
очереди таймеров не возможны. Это можно было бы реализовать, но на практике удобней апдейтить. Число таймеров выбирается
исходя из одновременно устанавливаемых в очередь задач. Так как работа с глобальной очередью таймеров, то
надо соблюдать атомарность добавления в очередь. Причем не тупо запрещать/разрешать прерывания, а
восстанавливать состояние прерываний.
 */
bool_t updateTimer(TaskMng TPTR, BaseSize_t n, BaseParam_t data, Time_t New_Time);
void delTimerTask(TaskMng TPTR, BaseSize_t n, BaseParam_t data);
void delAllTimerTask();
void SetIdleTask(IdleTask_t Task);
// Можно задать IDLE задачку, которая выполняется когда есть свободное время у процессора
//Задача должна иметь сигнатуру void Task(void)


u32 getTick(void);

void MaximizeErrorHandler(string_t str);

void memCpy(void * destination, const void * source, const BaseSize_t num);
void memSet(void* destination, const BaseSize_t size, const u08 value);

#ifdef EVENT_LOOP_TASKS
bool_t CreateEvent(Predicat_t condition, CycleFuncPtr_t effect); // Регистрирует новое событие в списке событий
void delEvent(Predicat_t condition); //Удаляем обработку события  condition
#endif

#ifdef  DATA_STRUCT_MANAGER
#define NOT_FOUND_DATA_STRUCT_ERROR 1
#define OVERFLOW_OR_EMPTY_ERROR     2
#define OTHER_ERROR                 3
#define NULL_PTR_ERROR              4
#define NO_MEMORY_ERROR             5
#define EVERYTHING_IS_OK            0
u08 CreateDataStruct(const void * const D, const BaseSize_t sizeElement, const BaseSize_t sizeAll);
u08 delDataStruct(const void * const Data);                                    // Удаляем структуру из списка структур
BaseSize_t getCurrentSizeDataStruct(const void* const Data);
u08 PutToCycleDataStruct(const void* Elem, const void* Array);
u08 GetFromCycleDataStruct(void* returnValue, const void* Array);
u08 PutToFrontDataStruct(const void * const Elem, const void * const Array);   // Кладем элемент в начало
u08 PutToEndDataStruct(const void * const Elem, const void * const Array);     // Кладем элемент в конец
u08 GetFromFrontDataStruct(void * const returnValue, const void * const Array);// Достаем элемент с начала структуры
u08 GetFromEndDataStruct(void * const returnValue, const void * const Array); // Достаем элемент с конца структуры данных
u08 delFromFrontDataStruct(const void* const Data); // Удаляет один элемент из структуры данных Data с начала
u08 delFromEndDataStruct(const void* const Data); // Удаляет один элемент из структуры данных Data с конца
u08 peekFromFrontData(void* returnValue, const void* Array); // Посмотреть первый элемент очереди не удаляя его
u08 peekFromEndData(void* returnValue, const void* Array);  // Посмотреть последний элемент очереди не удаляя его
bool_t isEmptyDataStruct(const void * const Data); // Проверяет пустая ли структура данных
void for_each(const void * const Array, TaskMng tsk);
void clearDataStruct(const void * const Data); // Очистить структуру данных с указателем Data
void showAllDataStruct(void); // передает в ЮАРТ данные о всех структурах данных
/*---------------ОЧЕРЕДЬ-------------------*/
// Создание очереди вернет ноль если очередь успешно создана
#define CreateQ(Q, sizeElement, sizeAll)    CreateDataStruct((void*)(Q), (BaseSize_t)(sizeElement), (BaseSize_t)(sizeAll))
// Положить элемент по указателю Elem в очередь Queue
#define PutToBackQ(Elem, Queue) PutToEndDataStruct((void*)(Elem), (void*)(Queue))
// Достать єлемент из очереди и записать его по указателю returnValue
#define GetFromQ(returnValue, Queue)   GetFromFrontDataStruct((void*)(returnValue), (void*)(Queue))
// Удаляем из массива очередей очередь с заданным идентивикатором
#define DeleteQ(Queue)  delDataStruct((void*)(Queue))

/*----------------СТЕК--------------------*/
#define CreateStack(Stack,sizeElement,sizeAll)  CreateDataStruct((void*)(Stack), (BaseSize_t)(sizeElement), (BaseSize_t)(sizeAll)) /*Создание стека в масиве структур данных*/
#define PushToStack(Elem, Stack)    PutToEndDataStruct((void*)(Elem), (void*)(Stack)) /*Вставляем элемент в стек*/
#define PopFromStack(returnValue, Stack)    GetFromEndDataStruct((void*)(returnValue), (void*)(Stack)) /*Достаем элемент из стека*/
#define DelStack(Stack)   delDataStruct((void*)(Stack)) /*Удаляем стек*/
#endif //DATA_STRUCT_MANAGER

#ifdef MUTEX_ENABLE
// TRUE - Если мьютекс захватить НЕ УДАЛОСЬ
bool_t tryGetMutex(const mutexType mutexNumb);
// TRUE - Если мьютекс захватить НЕ УДАЛОСЬ
bool_t getMutex(const mutexType mutexNumb, TaskMng TPTR, BaseSize_t n, BaseParam_t data); // Пытается захватить мьютекс Вернет TRUE если захватить не удалось
void freeMutex(const mutexType mutexNumb);     // Освобождает мьютекс
#define GET_MUTEX(mutexNumb, TaskPTR, arg_n, arg_p) if(getMutex((mutexType)mutexNumb, (TaskMng)TaskPTR,(BaseSize_t)arg_n, (BaseParam_t)arg_p)) return
#define FREE_MUTEX(mutexNumb) freeMutex((mutexType)mutexNumb)
#endif //MUTEX_ENABLE

#ifdef CYCLE_FUNC
void SetCycleTask(Time_t time, CycleFuncPtr_t CallBack, bool_t flagToQueue); // toManager == 0(false) выполняется прям в прерывании
void delCycleTask(BaseSize_t arg_n, CycleFuncPtr_t CallBack);
#endif //CYCLE_FUNC

#ifdef GLOBAL_FLAGS
typedef u08 globalFlags_t;
void setFlags(globalFlags_t flagMask);
void clearFlags(globalFlags_t flagMask);
bool_t getFlags(globalFlags_t flagMask);
globalFlags_t getGlobalFlags(void);
#endif

#ifdef ALLOC_MEM
byte_ptr allocMem(u08 size);  //size - до 127 размер блока выделяемой памяти
#define GET_MEMORY(size,pointer) if(!pointer){pointer = allocMem((u08)size);}
void freeMem(byte_ptr data);  // Освобождение памяти
void defragmentation(void);         // Дефрагментация памяти
u16 getFreeMemmorySize(void);
u16 getAllocateMemmorySize(byte_ptr data);
void clearAllMemmory(void); // Аварийное освобождение памяти
#endif //ALLOC_MEM

#ifdef CLOCK_SERVICE
Time_t getAllSeconds(void);
u08 getMinutes(void);
u08 getHour(void);
u16 getDayInYear(void);
u16 getDayAndMonth(void);
u08 getDaysInMonth(u08 month);
Date_t getDateFromSeconds(Time_t sec, bool_t toLocalTimeZone);
Time_t getSecondsFromDate(const Date_t*const date);
void setSeconds(u32 sec);
void setDate(string_t date); //YY.MM.DD hh:mm:ss
void dateToString(string_t out, Date_t* date);
s08 compareDates(const Date_t*const date1, const Date_t*const date2); /* * return >0 if date1 > date2  * return 0 if date = date2  * return <0 if date1 < date2  */
void addOneSecondToDate(Date_t* date);
void addOneMinutesToDate(Date_t* date);
void addOneHourToDate(Date_t* date);
void addOneDayToDate(Date_t* date);
void subOneDayFromDate(Date_t * date);
#define TIME_INDEX 2
#define SUMMER_TIME
#endif

#ifdef CALL_BACK_TASK
#ifndef OVERFLOW_OR_EMPTY_ERROR
#define OVERFLOW_OR_EMPTY_ERROR 2
#endif
#ifndef EVERYTHING_IS_OK
#define EVERYTHING_IS_OK 0
#endif
u08 registerCallBack(TaskMng task, BaseSize_t arg_n, BaseParam_t arg_p, void* labelPtr);
void execCallBack(void* labelPtr);
void execErrorCallBack(BaseSize_t errorCode, void* labelPtr);
void deleteCallBack(BaseSize_t arg_n, void* labelPtr);
u08 changeCallBackLabel(void* oldLabel, void* newLabel);
void clearAllCallBackList();
#endif

#ifdef SIGNALS_TASK
void connectTaskToSignal(TaskMng task, void* signal);
void disconnectTaskFromSignal(TaskMng task, void* signal);
void emitSignal(void* signal, BaseSize_t arg_n, BaseParam_t arg_p);
#endif

#ifdef USE_SOFT_UART
/*
 *  Настраиваем прерывания по достижению события совпадения каждые 26 мкс
 *  Для организации основных скоростей UART таймер работает с постоянной частотой прервыний
 *  Например 9600 бод/с = 1/(BAUD_9600*26мкс)
*/
void initSoftUART();
void enableSoftUART(bool_t txEnable, bool_t rxEnable);
void disableSoftUART();
void delSoftUART(const u08 numbUART);
void CreateSoftUART(const BaseSize_t buffTXsize, const BaseSize_t buffRXsize, const s08 BAUD,
                    const u08 numbUART, const u08 TXpinNumber, const u08 RXpinNumber);
void sendUART_byte(const u08 numbUART, const u08 U_data);
u08 readUART_byte(u08 numbUART);
BaseSize_t readUART_array(const u08 numbUART, const BaseSize_t size, const unsigned char* U_data); // Вернет количество прочитанного
void clearSoftUartRxBuffer(const u08 numbUART);
void sendUART_str(const u08 numbUART, const string_t U_data);
void sendUART_array(const u08 numbUART, const BaseSize_t size, const unsigned char* U_data);
void UARTTimerISR(); // Само прерывание
#endif //USE_SOFT_UART

//---------------------------------------------------------	СИНОНИМЫ API функций ядра ------------------------------------------------------------
#define Scheduler()	runFemtOS()		/*Функция диспетчера*/
#define Manager()       runFemtOS()
#define init_Mng()	initFemtOS()
#define CreateTask(New_Task, n, data)  SetTask((TaskMng)New_Task, (BaseSize_t)n, (BaseParam_t)data)
#define CreatePriorityTask(New_Task, n, data) SetFrontTask((TaskMng)New_Task, (BaseSize_t)n, (BaseParam_t)data)

#endif/*Task_Manager*/
