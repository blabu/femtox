#ifndef Task_Manager
#define Task_Manager

#ifndef NULL
#define NULL ((void*)0)
#endif

extern const char* osVersion;

#define ABS(XX) (((XX) > 0)?(XX):(-(XX)))
#define BASE_DALAY(x)  for(register volatile unsigned int ccii=0; ccii<(x); ccii++) /*Задержка*/
#define PAIR(T,V) struct{T first; V second;}

//#define SET_FRONT_TASK_ENABLE  /*разрешаем добавлеие в голову очереди задач (высокоприоритетная задача)*/
#define DATA_STRUCT_MANAGER   /*Включаем работу с очередями средствами деспетчера*/
#define CYCLE_FUNC  /*Разрешение работы циклически выполняемых программ в прерывании системного таймера*/
#define MUTEX_ENABLE /*Включаем поддержку мьютексов*/
//#define MAXIMIZE_OVERFLOW_ERROR  /*При переполнении очереди задач и или таймеров система заглохнет (максимизация оибки)*/
#define ALLOC_MEM   /*Включение динамического выделения памяти*/
#define EVENT_LOOP_TASKS
//#define USE_SOFT_UART
#define CLOCK_SERVICE
#define GLOBAL_FLAGS
#define CALL_BACK_TASK
#define SIGNALS_TASK
#define _LIST_STRUCT
//#define _DYNAMIC_ARRAY
#define _TRY_IT_TASK  /*Експериментальная функция*/

#define TASK_LIST_LEN 20U /*Длина очереди задач*/
#define TIME_LINE_LEN 30U /*Максимальне количество системных таймеров*/
#define TIME_DELAY_IF_BUSY 2U /*Задержка на повторную попытку поставить задачу в очередь или захватить мьютекс*/

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

typedef unsigned short  BaseSize_t; // Первый аргумент для задачи в диспетчере
typedef void* BaseParam_t;  // Второй аргумент для задачи в диспетчере

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


//Эта операционка сделана для 8-ми битных микроконтроллеров, поэтому чаще всего будут передаватся функциям восьмибиные параметры. Поэтому взят указатель на char

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

#ifdef _TRY_IT_TASK
#ifdef CALL_BACK_TASK
#ifdef ALLOC_MEM
/*
 * Экспериментальная функция
 * Аргументы:
 * attempt - колличество попыток выполнить задачу
 * delayTime - задержка между выполнениями задачи
 * task - указатель на саму задачу, которую надо выполнить.
 * arg_n, arg_p - аргументы задачи, которую надо выполнить.
 * isOK - указатель на функцию предикат, которая возвращает истину если задача выполнилась успешно
*/
	void tryItTask(BaseSize_t attempt, Time_t delayTime, TaskMng task, BaseSize_t arg_n, BaseParam_t arg_p, Predicat_t isOK);
#endif
#endif
#endif

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

void SetIdleTask(IdleTask_t Task);
// Можно задать IDLE задачку, которая выполняется когда есть свободное время у процессора
//Задача должна иметь сигнатуру void Task(void)


u32 getTick(void);

void MaximizeErrorHandler(void);

void memCpy(void * destination, const void * source, const BaseSize_t num);
void memSet(void* destination, const BaseSize_t size, const u08 value);
void shiftLeftArray(BaseParam_t source, BaseSize_t sourceSize, BaseSize_t shiftSize);

#ifdef EVENT_LOOP_TASKS
#define EVENT_LIST_SIZE 10
   bool_t CreateEvent(Predicat_t condition, CycleFuncPtr_t effect); // Регистрирует новое событие в списке событий
   void delEvent(Predicat_t condition); //Удаляем обработку события  condition
#endif

#ifdef  DATA_STRUCT_MANAGER
#define ArraySize   12 /*Общее количество всех структур данных*/
#define NOT_FAUND_DATA_STRUCT_ERROR 1
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
#define deleteQ(Queue)  delDataStruct((void*)(Queue))

/*----------------СТЕК--------------------*/
// Создание стека в масиве структур данных
#define CreateStack(Stack,sizeElement,sizeAll)  CreateDataStruct((void*)(Stack), (BaseSize_t)(sizeElement), (BaseSize_t)(sizeAll))
// Вставляем элемент в стек
#define PushToStack(Elem, Stack)    PutToEndDataStruct((void*)(Elem), (void*)(Stack))
// Достаем элемент из стека
#define PopFromStack(returnValue, Stack)    GetFromEndDataStruct((void*)(returnValue), (void*)(Stack))
// Удаляем стек
#define delStack(Stack)   delDataStruct((void*)(Stack))
#endif //DATA_STRUCT_MANAGER

#ifdef MUTEX_ENABLE
#define MUTEX_SIZE 8
#if MUTEX_SIZE <= 8
typedef u08 mutexType;
#elif MUTEX_SIZE <= 16
typedef u16 mutexType;
#elif MUTEX_SIZE <= 32
typedef u32 mutexType;
#else
#error "Too long size for mutex"
#endif
	  // TRUE - Если мьютекс захватить НЕ УДАЛОСЬ
	  bool_t tryGetMutex(const mutexType mutexNumb);
	  // TRUE - Если мьютекс захватить НЕ УДАЛОСЬ
	  bool_t getMutex(const mutexType mutexNumb, TaskMng TPTR, BaseSize_t n, BaseParam_t data); // Пытается захватить мьютекс Вернет TRUE если захватить не удалось
      void freeMutex(const mutexType mutexNumb);     // Освобождает мьютекс
#define GET_MUTEX(mutexNumb, TaskPTR, arg_n, arg_p) if(getMutex((u08)mutexNumb, (TaskMng)TaskPTR,(BaseSize_t)arg_n, (BaseParam_t)arg_p)) return
#define FREE_MUTEX(mutexNumb) freeMutex((u08)mutexNumb)
#endif //MUTEX_ENABLE

#ifdef CYCLE_FUNC
     #define TIMERS_ARRAY_SIZE 15
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
#define HEAP_SIZE 600UL /*6500*/
    byte_ptr allocMem(u08 size);  //size - до 127 размер блока выделяемой памяти
#define GET_MEMORY(size,pointer) if(!pointer){pointer = allocMem((u08)size);}
    void freeMem(byte_ptr data);  // Освобождение памяти
    void defragmentation(void);         // Дефрагментация памяти
    u16 getFreeMemmorySize(void);
    u16 getAllocateMemmorySize(byte_ptr data);
    void clearAllMemmory(void); // Аварийное освобождение памяти
#endif //ALLOC_MEM

#ifdef CLOCK_SERVICE
typedef struct {
	u08 sec;  //Начинаются с 0
  	u08 min;  //Начинаются с 0
   	u08 hour;  //Начинаются с 0
   	u08 day;  //Начинаются с 1
   	u08 mon;  //Начинаются с 1
   	u16 year;
} Date_t;
	Time_t getAllSeconds(void);
    u08 getMinutes(void);
    u08 getHour(void);
    u16 getDayInYear(void);
    u16 getDayAndMonth(void);
    u08 getDaysInMonth(u08 month);
    Date_t getDateFromSeconds(Time_t sec);
    Time_t getSecondsFromDate(const Date_t*const date);
    void setSeconds(u32 sec);
    void setDate(string_t date); //YY.MM.DD hh:mm:ss
    void dateToString(string_t out, Date_t* date);
    s08 compareDates(const Date_t*const date1, const Date_t*const date2); /* * return >0 if date1 > date2  * return 0 if date = date2  * return <0 if date1 < date2  */
    void addOneSecondToDate(Date_t* date);
    void addOneMinutesToDate(Date_t* date);
    void addOneHourToDate(Date_t* date);
    void addOneDayToDate(Date_t* date);
	#define TIME_INDEX 2
	#define SUMMER_TIME
#endif

#ifdef CALL_BACK_TASK
#define CALL_BACK_TASK_LIST_LEN 35
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
#endif
#ifdef SIGNALS_TASK
#define SIGNAL_LIST_LEN 10
    void connectTaskToSignal(TaskMng task, void* signal);
    void disconnectTaskFromSignal(TaskMng task, void* signal);
    void emitSignal(void* signal, BaseSize_t arg_n, BaseParam_t arg_p);
#endif
//---------------------------------------------------------	СИНОНИМЫ API функций ядра ------------------------------------------------------------
#define Scheduler()	runFemtOS()		/*Функция диспетчера*/
#define Manager()       runFemtOS()
#define init_Mng()	initFemtOS()
#define CreateTask(New_Task, n, data)  SetTask((TaskMng)New_Task, (BaseSize_t)n, (BaseParam_t)data)
#define CreatePriorityTask(New_Task, n, data) SetFrontTask((TaskMng)New_Task, (BaseSize_t)n, (BaseParam_t)data)

#endif/*Task_Manager*/
