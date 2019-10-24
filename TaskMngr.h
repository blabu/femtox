#ifndef Task_Manager
#define Task_Manager

#include "FemtoxConf.h"
#include "FemtoxTypes.h"
#include "compilerSpecific.h"

extern const BaseSize_t _MAX_BASE_SIZE;
extern const char* const _osVersion;

#define ABS(XX) (((XX) > 0)?(XX):(-(XX)))
#define BitsSet(Register, Bits)                    (Register) |= (Bits)
#define BitsClear(Register, Bits)                  (Register) &= ~(Bits)
#define BitSet(Register, Bit)                      (Register) |= (1 << (Bit))
#define BitClear(Register, Bit)                    (Register) &= (~(1<< (Bit)))
#define BitRead(Register,Bit)                      ((qFalse == ((Register)& (1<<(Bit))))? qFalse : qTrue)
#define BitToggle(Register,Bit)                    ((Register)^= (1<<(Bit)))
#define BitWrite(Register, Bit, Value)             ((Value) ? qBitSet(Register,Bit) : qBitClear(Register,Bit))
#define BitMakeByte(b7,b6,b5,b4,b3,b2,b1,b0)       (uint8_t)( ((b7)<<7) + ((b6)<<6) + ((b5)<<5) + ((b4)<<4) + ((b3)<<3) + ((b2)<<2) + ((b1)<<1) + ((b0)<<0) )
#define ByteMakeFromBits(b7,b6,b5,b4,b3,b2,b1,b0)  qBitMakeByte(b7,b6,b5,b4,b3,b2,b1,b0)
#define ByteHighNibble(Register)                   ((uint8_t)((Register)>>4))
#define ByteLowNibble(Register)                    ((uint8_t)((Register)&0x0F))
#define ByteMergeNibbles(H,L)                      ((uint8_t)(((H)<<4)|(0x0F&(L))))
#define WordHighByte(Register)                     ((uint8_t)((Register)>>8))
#define WordLowByte(Register)                      ((uint8_t)((Register)&0x00FF))
#define WordMergeBytes(H,L)                        ((uint16_t)(((H)<<8)|(L)))
#define DWordHighWord(Register)                    ((uint16_t)((Register) >> 16))
#define DWordLowWord(Register)                     ((uint16_t)((Register) & 0xFFFF))
#define DWordMergeWords(H,L)                       ((uint32_t)(((uint32_t)(H) << 16 ) | (L) ) )

#define CLIP(X, Max, Min)                          (((X) < (Min)) ? (Min) : (((X) > (Max)) ? (Max) : (X)))
#define CLIPUpper(X, Max)                          (((X) > (Max)) ? (Max) : (X))
#define CLIPLower(X, Min)                          (((X) < (Min)) ? (Min) : (X))
#define IsBetween(X, Low, High)                    ((qBool_t)((X) >= (Low) && (X) <= (High)))
#define MIN(a,b)                                   (((a)<(b))?(a):(b))
#define MAX(a,b) 								   (((a)>(b))?(a):(b))

void initFemtOS(void);    /* Инициализация менеджера задач. Здесь весь список задач (масив TaskLine) иницмализируется функцией Idle*/
CC_NO_RETURN void ResetFemtOS(void);  // Програмный сброс микроконтроллера
CC_NO_RETURN void runFemtOS( void ); // Запуск операционной системы
void SetTask (const TaskMng New_Task, const BaseSize_t n, const BaseParam_t data); /* Функция помещает в конец очереди задачу New_Task с количеством параметров n. И параметрами data[n]
Прочесываем всю очередь задач в поисках пустой функции (Idle). Когда нашли засовываем вместо нее новую задачу
и выходим из функции. Если не нашли пустой функции и засовывать задачу некуда просто выходим из функции. Можно также возвращать
код ошибки если не удалось записать задачу и нормального завершения. Тогда функция будет иметь тип не void, а uint8_t.
 */

bool_t isEmptyTaskList(void);
u08 getFreePositionForTask(void);
u08 getFreePositionForTimerTask(void);

#ifdef SET_FRONT_TASK_ENABLE
void SetFrontTask (const TaskMng New_Task, const BaseSize_t n, const BaseParam_t data); // Поставить задачу в начало очереди
#endif
//Сами задачи следует делать небольшими.

void delAllTask(void);

//********************************СИСТЕМНЫЙ ТАЙМЕР*********************************************
void TimerISR(void); //Обработчик прерывания по совпадению теущего значения таймера и счетчика.

void SetTimerTask(const TaskMng TPTR, const BaseSize_t n, const BaseParam_t data, const Time_t New_Time); //Постановщик задач c количеством параметров n в таймер.
/*Функция устанавливающая задачу в очередь по таймеру. На входе адрес перехода (имя задачи) и время в тиках службы таймера.
Время двухбайтное, т.е. от 1 до 65535 измеряеться в переплнениях таймера0. Не имеет значение
какой таймер использовать. Если в очереди таймеров уже есть таймер с такой задачей, то происходит апдейт времени. Две одинаковых задачи в
очереди таймеров не возможны. Это можно было бы реализовать, но на практике удобней апдейтить. Число таймеров выбирается
исходя из одновременно устанавливаемых в очередь задач. Так как работа с глобальной очередью таймеров, то
надо соблюдать атомарность добавления в очередь. Причем не тупо запрещать/разрешать прерывания, а
восстанавливать состояние прерываний.
 */
bool_t updateTimer(const TaskMng TPTR, const BaseSize_t n, const BaseParam_t data, const Time_t New_Time);
void delTimerTask(const TaskMng TPTR, const BaseSize_t n, const BaseParam_t data);
void delAllTimerTask(void);
void SetIdleTask(const IdleTask_t Task);
// Можно задать IDLE задачку, которая выполняется когда есть свободное время у процессора
//Задача должна иметь сигнатуру void Task(void)

u32 getTick(void);

void MaximizeErrorHandler(const string_t str);
bool_t compare(const void* block1, const void* block2, const BaseSize_t size);
void memCpy(void * destination, const void * source, const BaseSize_t num);
void memSet(void* destination, const BaseSize_t size, const u08 value);
void shiftLeftArray(BaseParam_t source, BaseSize_t sourceSize, BaseSize_t shiftSize);

#ifdef LOAD_STATISTIC
u32 getLoadAvarage();
#endif

#ifdef EVENT_LOOP_TASKS
bool_t CreateEvent(Predicat_t condition, CycleFuncPtr_t effect); // Регистрирует новое событие в списке событий
void delEvent(Predicat_t condition); //Удаляем обработку события  condition
#endif

enum {
 NOT_FOUND_DATA_STRUCT_ERROR  = 1,
 OVERFLOW_OR_EMPTY_ERROR	  = 2,
 OTHER_ERROR                  = 3,
 NULL_PTR_ERROR               = 4,
 NO_MEMORY_ERROR              = 5,
 UNDEFINED_BEHAVIOR			  = 6,
 NOT_IMPLEMENTED_ERROR		  = 7,
 EVERYTHING_IS_OK             = 0,
};

#ifdef  DATA_STRUCT_MANAGER
u08 CreateDataStruct(const void* D, const BaseSize_t sizeElement, const BaseSize_t sizeAll);
u08 delDataStruct(const void* Data);               // Удаляем структуру из списка структур
void clearDataStruct(const void* const Data); // Очистить структуру данных с указателем Data
BaseSize_t getCurrentSizeDataStruct(const void* const Data);
u08 PutToCycleDataStruct(const void* Elem, const void* Array);
u08 GetFromCycleDataStruct(void* returnValue, const void* Array);
u08 PutToFrontDataStruct(const void * Elem, const void* Array);   // Кладем элемент в начало
u08 PutToEndDataStruct(const void* Elem, const void* Array);     // Кладем элемент в конец
u08 GetFromFrontDataStruct(void* returnValue, const void* Array);// Достаем элемент с начала структуры
u08 GetFromEndDataStruct(void* returnValue, const void* Array); // Достаем элемент с конца структуры данных
u08 delFromFrontDataStruct(const void* const Data); // Удаляет один элемент из структуры данных Data с начала
u08 delFromEndDataStruct(const void* const Data); // Удаляет один элемент из структуры данных Data с конца
u08 peekFromFrontData(void* returnValue, const void* Array); // Посмотреть первый элемент очереди не удаляя его
u08 peekFromEndData(void* returnValue, const void* Array);  // Посмотреть последний элемент очереди не удаляя его
bool_t isEmptyDataStruct(const void* const Data); // Проверяет пустая ли структура данных
void forEach(const void* const Array, TaskMng tsk);
/*---------------ОЧЕРЕДЬ-------------------*/
// Создание очереди вернет ноль если очередь успешно создана
#define CreateQ(Q, sizeElement, sizeAll)    CreateDataStruct((void*)(Q), (BaseSize_t)(sizeElement), (BaseSize_t)(sizeAll))
// Положить элемент по указателю Elem в очередь Queue
#define PutToBackQ(Elem, Queue) PutToEndDataStruct((void*)(Elem), (void*)(Queue))
// Достать єлемент из очереди и записать его по указателю returnValue
#define GetFromQ(returnValue, Queue)   GetFromFrontDataStruct((void*)(returnValue), (const void*)(Queue))
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
bool_t getMutex(const mutexType mutexNumb, const TaskMng TPTR, const BaseSize_t n, const BaseParam_t data); // Пытается захватить мьютекс Вернет TRUE если захватить не удалось
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

#ifdef ALLOC_MEM_LARGE
byte_ptr allocMem(const BaseSize_t size);  //size - до 127 размер блока выделяемой памяти
#define GET_MEMORY(size,pointer) if(!pointer){pointer = allocMem((u08)size);}
void freeMem(const byte_ptr data);  // Освобождение памяти
void defragmentation(void);         // Дефрагментация памяти
void freeMemTask(BaseSize_t count, BaseParam_t pointer);
BaseSize_t getFreeMemmorySize(void);
BaseSize_t getAllocateMemmorySize(const byte_ptr data);
void clearAllMemmory(void); // Аварийное освобождение памяти
#endif //ALLOC_MEM_LARGE
#ifdef ALLOC_MEM
byte_ptr allocMem(const u08 size);  //size - до 127 размер блока выделяемой памяти
#define GET_MEMORY(size,pointer) if(!pointer){pointer = allocMem((u08)size);}
void freeMem(const byte_ptr data);  // Освобождение памяти
void freeMemTask(BaseSize_t count, BaseParam_t pointer);
void defragmentation(void);         // Дефрагментация памяти
u16 getFreeMemmorySize(void);
u16 getAllocateMemmorySize(const byte_ptr data);
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
void setSeconds(const u32 sec);
void setDate(string_t date); //YY.MM.DD hh:mm:ss
//Form YY.MM.DD hh:mm:ss
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
u08 registerCallBack(const TaskMng task, const BaseSize_t arg_n, const BaseParam_t arg_p, const void*const labelPtr);
void execCallBack(const void*const labelPtr);
void execErrorCallBack(const BaseSize_t errorCode, const void*const labelPtr);
void deleteCallBack(const BaseSize_t arg_n, const void*const labelPtr);
void deleteCallBackByTask(TaskMng task);
u08 changeCallBackLabel(const void* oldLabel, const void*const newLabel);
void clearAllCallBackList(void);
#endif

#ifdef SIGNALS_TASK
void connectTaskToSignal(const TaskMng task, const void*const signal);
void disconnectTaskFromSignal(const TaskMng task, const void*const signal);
void emitSignal(const void*const signal, BaseSize_t arg_n, BaseParam_t arg_p);
#endif

#ifdef USE_SOFT_UART
/*
 *  Настраиваем прерывания по достижению события совпадения каждые 26 мкс
 *  Для организации основных скоростей UART таймер работает с постоянной частотой прервыний
 *  Например 9600 бод/с = 1/(BAUD_9600*26мкс)
*/
void initSoftUART(void);
void enableSoftUART(bool_t txEnable, bool_t rxEnable);
void disableSoftUART(void);
void delSoftUART(const u08 numbUART);
void CreateSoftUART(const BaseSize_t buffTXsize, const BaseSize_t buffRXsize, const s08 BAUD,
                    const u08 numbUART, const u08 TXpinNumber, const u08 RXpinNumber);
void sendUART_byte(const u08 numbUART, const u08 U_data);
u08 readUART_byte(u08 numbUART);
BaseSize_t readUART_array(const u08 numbUART, const BaseSize_t size, const unsigned char* U_data); // Вернет количество прочитанного
void clearSoftUartRxBuffer(const u08 numbUART);
void sendUART_str(const u08 numbUART, const string_t U_data);
void sendUART_array(const u08 numbUART, const BaseSize_t size, const unsigned char* U_data);
void UARTTimerISR(void); // Само прерывание
#endif //USE_SOFT_UART

//---------------------------------------------------------	СИНОНИМЫ API функций ядра ------------------------------------------------------------
#define Scheduler()	    runFemtOS()		/*Функция диспетчера*/
#define Manager()       runFemtOS()
#define init_Mng()	initFemtOS()
#define CreateTask(New_Task, n, data)  SetTask((TaskMng)New_Task, (BaseSize_t)n, (BaseParam_t)data)
#define CreatePriorityTask(New_Task, n, data) SetFrontTask((TaskMng)New_Task, (BaseSize_t)n, (BaseParam_t)data)

#define everyTimeRunInCoroutine if(TRUE)
#define startCoroutine(_variable)   switch(_variable){ case 0:
#define yieldCoroutine(_constParam) do{break; case (_constParam):;}while(0)
#define finishCoroutine(_variable)  do{_variable = 0xFF; break; default:;}while(0)
#define stopCoroutine() }

#endif/*Task_Manager*/
