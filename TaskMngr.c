#include "TaskMngr.h"
#include "PlatformSpecific.h"

#ifdef __cplusplus
extern "C" {
#endif
/*
Создание простого диспетчера задач. Пример построения программы
Кроме диспетчера задач здесь также реализован системный таймер на базе Т/С0
инициализируется таймер счетчик, и включает прерывание по переполнению Т/С0
*/

const char* osVersion = "V1.0.2";

static void TaskManager(void);
static void TimerService(void);
#ifdef ALLOC_MEM
extern void initHeap(void);
#endif

#ifdef DATA_STRUCT_MANAGER
extern void initDataStruct( void );
#endif

#ifdef CYCLE_FUNC
extern void initCycleTask( void );
extern void CycleService( void );
#endif

#ifdef EVENT_LOOP_TASKS
extern void EventManager( void );
extern void initEventList( void );
#endif

#ifdef CLOCK_SERVICE
extern u32 __systemSeconds;
#endif

static void ClockService( void );

#ifdef CALL_BACK_TASK
extern void initCallBackTask();
#endif

#ifdef EMPTY
#warning "EMPTY eror"
#endif
#define EMPTY 0xFF

#ifndef TASK_LIST_LEN
#error "TASK_LIST_LEN error (Не определена длина списка задач)"
#endif
// длина списка задач (максимальное число задач)
#if TASK_LIST_LEN > 0xFE
#error "Invalid size"
#endif

#ifndef TIME_LINE_LEN
#error "TIME_LINE_LEN error (Не определена длина списка таймеров)"
#endif

#if TIME_LINE_LEN > 0xFE
#error "Invalid size"
#endif

volatile static IdleTask_t IdleTask=NULL;

static TaskList_t TaskList[TASK_LIST_LEN];   // Очередь задач - это глобальный масив переменных. Каждый элемент которой состоит из трех переменных.

// Очередь системных таймеров
//В очередь записывается задача (указатель на функцию) и выдержка времени необходимая перед постановкой задачи в очередь
static unsigned int MainTime[TIME_LINE_LEN];// Выдержка времени для конкретной задачи в мс.
static TaskList_t MainTimer[TIME_LINE_LEN]; // Указатель задачи, которая состоит из указателя на функцию задачи, и двух аргументов


volatile static Time_t GlobalTick;
u32 getTick(void) {
	u32 time_res = 0;
    while(time_res != GlobalTick) time_res = (u32)GlobalTick;      // Так как переменная у нас двухбайтная
#ifdef CLOCK_SERVICE
    time_res += __systemSeconds*TICK_PER_SECOND;
#endif
    return time_res;
}


static void ClockService(void){
    GlobalTick++;
#ifdef CLOCK_SERVICE
    if(GlobalTick >= TICK_PER_SECOND) {
    	__systemSeconds++;
    	GlobalTick = 0;
    }
#endif
}

void SetIdleTask(IdleTask_t Task)
{
    bool_t flag_ISR = FALSE;
    if (INTERRUPT_STATUS) //Если прерывания разрешены, то запрещаем их
    {
        INTERRUPT_DISABLE;
        flag_ISR = TRUE;                     // И устанавливаем флаг, что мы не в прерывании
    }
    IdleTask = Task;
    if(flag_ISR) INTERRUPT_ENABLE;
}

static void Idle(void) // Функция включает режим пониженного электропотребления микроконтроллера. При этом перестает работать ядро.
{
    if(IdleTask != NULL) IdleTask();
}
/********************************************************************************************************************
*********************************************************************************************************************
|					МЕНЕДЖЕР ЗАДАЧ	 														|
*********************************************************************************************************************
*********************************************************************************************************************
---------------------------------------------------------------------------------------------------------------------*/
void initFemtOS (void)   // Инициализация менеджера задач
{
    u08 i;
    // Отметим, что имя функции является ее указателем.
    // Вызвать любую функцию можно двояко:
    //  1. Стандартным способом через ее имя и список параметров Например, shov1();
    //  2. Через указатель на функцию. К примеру, (*show1)() - операция разыменовывания указателя на функцию;
    //INTERRUPT_DISABLE;
    for(i=0;i<TASK_LIST_LEN;i++)  //Набираем очередь задач // Это масив указателей на функции
    {
        TaskList[i].Task = NULL;
    }
    for(i=0; i<TIME_LINE_LEN;i++)
    {
        MainTimer[i].Task = NULL; // Вся очередь таймеров состоит из пустышек
    }
    _init_Timer();
    IdleTask = NULL;
    GlobalTick = 0;
#ifdef ALLOC_MEM
    initHeap();
#endif
#ifdef DATA_STRUCT_MANAGER
    initDataStruct();
#endif //DATA_STRUCT_MANAGER

#ifdef CYCLE_FUNC
    initCycleTask();
#endif  //CYCLE_FUNC
#ifdef EVENT_LOOP_TASKS
    initEventList();
#endif
#ifdef CALL_BACK_TASK
    initCallBackTask();
#endif
    //INTERRUPT_ENABLE;
}

void runFemtOS( void )
{
    while(1)
    {
#ifdef EVENT_LOOP_TASKS
        EventManager();
#endif
        TaskManager();
    }
}

void ResetFemtOS(void){
    WATCH_DOG_ON;
    while(1);
}

void TimerISR(void)
{
#ifdef CYCLE_FUNC
	CycleService();
#endif
      TimerService();	// Пересчет всех системных таймеров из очереди
      ClockService();
} 	//Отработка прерывания по переполнению TCNT0

static u08 countBegin = 0;    // Указатель на НАЧАЛО очереди (нужен для быстрого диспетчера)
static u08 countEnd = 0;      // Указатель на КОНЕЦ очереди (нужен для быстрого диспетчера)

// Функция Планировщика (Менеджера) задач. Она запускает ту функцию, которая должна сейчас выполнятся.
/*	Берется первая функция из очереди задач (TaskLine[0]) и проверяется не пустая ли она. Если не пустая, то смещаем всю чередь
на один элемент вверх, а в последний элемент очереди ставим Idle (пустую функцию, включающую режим ожидания МК).
Берем количество параметров из глобального стека и передаем взятой функции, которая берет свои параметры из глобального стека.
*/
static void TaskManager(void)
{
    BaseSize_t   n;       // Первый аргумент следующей функции (количество параметров)
    BaseParam_t  a;       // Второй аргумент для следующей фунции (адрес первой переменной)
    TaskMng Func_point;       // Определяем временную переменную типа указатель на функцию
    INTERRUPT_DISABLE;
    if(countBegin != countEnd) // Если очередь не пустая
    {// Необходимо помнить про конвеерный способ выборки команд в микроконтроллере (if - как можно чаще должен быть истиной)
        Func_point = TaskList[countBegin].Task; // countBegin - указывает на начало очереди на рабочую задачу
        a = TaskList[countBegin].arg_p;
        n = TaskList[countBegin].arg_n;
        countBegin = (countBegin < TASK_LIST_LEN-1)? countBegin+1:0;
        INTERRUPT_ENABLE;
        Func_point(n,a);
        return;
    }
    INTERRUPT_ENABLE; // Если очередь пустая включаем прерывания
    Idle();         // И выполняем функция простоя
}

void SetTask(TaskMng New_Task, BaseSize_t n, BaseParam_t data)
{
    bool_t flag_inter = FALSE;
    if(INTERRUPT_STATUS) //Если прерывания разрешены, то запрещаем их
    {
        INTERRUPT_DISABLE;
        flag_inter = TRUE;                     // И устанавливаем флаг, что мы не в прерывании
    }
    register u08 count = (countEnd < TASK_LIST_LEN-1)? countEnd+1:0; //Кольцевой буфер
    if(count != countBegin) // Если после добавления задачи countEnd не догонит countBegin значит очередь не переполнена
    {// Необходимо помнить про конвеерный способ выборки команд в микроконтроллере (if - как можно чаще должен быть истиной)
        TaskList[countEnd].Task = New_Task; // Если очередь не переполнится добавляем элемент в очередь
        TaskList[countEnd].arg_n = n;       // countEnd
        TaskList[countEnd].arg_p = data;
        countEnd = count;
        if(flag_inter) INTERRUPT_ENABLE;
        return;
    }// Здесь мы окажемся в редких случаях когда oчередь переполнена
    SetTimerTask(New_Task, n, data, TIME_DELAY_IF_BUSY);  //Ставим задачу в очередь(попытаемся записать ее позже)
    if (flag_inter) INTERRUPT_ENABLE;  //предварительно восстановив прерывания, если надо.
}

bool_t isEmptyTaskList( void ){
	if(countBegin == countEnd) return TRUE; // Если очередь пустая (чаще всего так и есть)
	return FALSE;
}

u08 getFreePositionForTask(void){
	u08 end = 0;
	u08 begin = 0;
	while(begin != countBegin) begin = countBegin;
	while(end != countEnd) end = countEnd;
	if(end >= begin) return (TASK_LIST_LEN - (end-begin));
	return begin - end;
}

#ifdef SET_FRONT_TASK_ENABLE
void SetFrontTask (TaskMng New_Task, BaseSize_t n, BaseParam_t data) // Функция помещает в НАЧАЛО очереди задачу New_Task
{
    bool_t flag_inter = FALSE;
    if(INTERRUPT_STATUS)
    {
        flag_inter = TRUE;
        INTERRUPT_DISABLE;
    }
    register u08 count = (countBegin)? countBegin-1:TASK_LIST_LEN-1; // Определяем указатель начала очереди куда должны вставить новую задачку
    if(count != countEnd)   // Если очередь еще не переполнена
    {// Необходимо помнить про конвеерный способ выборки команд в микроконтроллере (if - как можно чаще должен быть истиной)
        countBegin = count;
        TaskList[countBegin].Task = New_Task;
        TaskList[countBegin].arg_n = n;
        TaskList[countBegin].arg_p = data;
        return;
    }
    // Здесь мы окажемся если все таки очередь переполнена (мало вероятный случай)
    SetTimerTask(New_Task, n, data, TIME_DELAY_IF_BUSY);
    if (flag_inter) INTERRUPT_ENABLE;  //предварительно восстановив прерывания, если надо.
}
#endif  //SET_FRONT_TASK_ENABLE

void delAllTask(void) {
	while(countBegin != countEnd)  {
		countBegin = countEnd = 0;
	}
}
/********************************************************************************************************************
*********************************************************************************************************************
|						 СИСТЕМНЫЙ ТАЙМЕР														|
*********************************************************************************************************************
*********************************************************************************************************************
---------------------------------------------------------------------------------------------------------------------*/
static u08 lastTimerIndex = 0; // Указывает на индекс следующего СВОБОДНОГО таймера

static void TimerService (void)
{
    u08 index = 0;
    while(index < lastTimerIndex)  // Перебираем всю очередь таймеров
    {
        if(MainTime[index] != 1)    // Если таймер еще не дотикал (наиболее вероятно)
        {
            MainTime[index]--;      // Тикаем им
            index++;                // И переходим на следующую итерацию цикла
            continue;
        }
        SetTask (MainTimer[index].Task,  // Ставим нашу задачу в конец очередь
                 MainTimer[index].arg_n,
                 MainTimer[index].arg_p);

        lastTimerIndex--;
        MainTimer[index].Task  = MainTimer[lastTimerIndex].Task;    // На место этого таймера перемещаем последний
        MainTimer[index].arg_n = MainTimer[lastTimerIndex].arg_n;
        MainTimer[index].arg_p = MainTimer[lastTimerIndex].arg_p;
        MainTime[index] = MainTime[lastTimerIndex];
    }
}

void SetTimerTask(TaskMng TPTR, BaseSize_t n, BaseParam_t data, Time_t New_Time){
    bool_t flag_inter = FALSE;  // флаг состояния прерывания
    if (INTERRUPT_STATUS) //Если прерывания разрешены, то запрещаем их
    {
        INTERRUPT_DISABLE;
        flag_inter = TRUE;                     // И устанавливаем флаг, что мы не в прерывании
    }
    if(lastTimerIndex < TIME_LINE_LEN) // Если очередь не переполнена
    {
        MainTimer[lastTimerIndex].Task = TPTR;
        MainTimer[lastTimerIndex].arg_n = n;
        MainTimer[lastTimerIndex].arg_p = data;
        MainTime[lastTimerIndex] = New_Time;
        lastTimerIndex++;
        if(flag_inter) INTERRUPT_ENABLE;
        return;
    }
    #ifdef MAXIMIZE_OVERFLOW_ERROR
    #warning "if queue task timers is overflow programm will be stoped"
     	 MaximizeErrorHandler();
    #else
        if(flag_inter) INTERRUPT_ENABLE;
        return; //  тут можно сделать return c кодом ошибки - нет свободных таймеров
    #endif
}
static u08 findTimer(TaskMng TPTR, BaseSize_t n, BaseParam_t data)
{
    register u08 index = 0;
    for(;index < lastTimerIndex; index++)
    {
        if((MainTimer[index].Task  == TPTR)&& /* Если уже есть запись с таким же адресом*/
           (MainTimer[index].arg_p == data)&&
           (MainTimer[index].arg_n == n))     /* и с таким же списком параметров*/
        {
            break;      // Досрочно прекращаем работу цикла
        }
    }
    return index;
}

bool_t updateTimer(TaskMng TPTR, BaseSize_t n, BaseParam_t data, Time_t New_Time) {
    u08 index = findTimer(TPTR,n,data);
    if(index < lastTimerIndex) {
        bool_t flag_inter = FALSE;
        if(INTERRUPT_STATUS){
            INTERRUPT_DISABLE;
            flag_inter = TRUE;
        }
        MainTime[index] = New_Time;
        if(flag_inter) INTERRUPT_ENABLE;
        return TRUE;
    }
    return FALSE;
}

void delTimerTask(TaskMng TPTR, BaseSize_t n, BaseParam_t data)
{
    u08 index = findTimer(TPTR,n,data);
    if(index < lastTimerIndex)
    {
        bool_t flag_inter = FALSE;
        if(INTERRUPT_STATUS)
        {
            INTERRUPT_DISABLE;
            flag_inter = TRUE;
        }
        lastTimerIndex--;
        MainTimer[index].Task  = MainTimer[lastTimerIndex].Task;    // На место этого таймера перемещаем последний
        MainTimer[index].arg_n = MainTimer[lastTimerIndex].arg_n;
        MainTimer[index].arg_p = MainTimer[lastTimerIndex].arg_p;
        MainTime[index] = MainTime[lastTimerIndex];
        if(flag_inter) INTERRUPT_ENABLE;
    }
}

u08 getFreePositionForTimerTask(void) {
	return TIME_LINE_LEN - lastTimerIndex;
}

//destination - адрес в памяти КУДА копируем source - адрес в памяти ОТКУДА копируем n - количество БАЙТ копируемых
void memCpy(void* destination, const void* source, const BaseSize_t num) {
#if ARCH == 32
		BaseSize_t blocks = num>>2;		// 4-ре байта копируются за один раз
		u08 last = num & 0x03; // остаток
		for(BaseSize_t i = 0; i<blocks; i++) {
			*((u32*)destination) = *((u32*)source);
			destination+=4; source+=4;
		}
		for(u08 i = 0; i<last; i++) {
			*((byte_ptr)destination) = *((byte_ptr)source);
			(byte_ptr)destination++; (byte_ptr)source++;
		}
#elif ARCH == 16
		BaseSize_t blocks = num>>1;		// 2 байта копируются за один раз
		BaseSize_t last = num & 0x01; // остаток
		for(BaseSize_t i = 0; i<blocks; i++) {
			*((u16*)destination) = *((u16*)source);
			destination+=2; source+=2;
		}
		if(last) *((byte_ptr)destination) = *((byte_ptr)source);
#else
			for (BaseSize_t i = 0; i < num; i++){ //Копирование будет побайтное
				*((byte_ptr)destination + i) = *((byte_ptr)source + i); // Выполняем копирование данных
			}
#endif

}

void memSet(void* destination, const BaseSize_t size, const u08 value) {
#if ARCH == 32
    BaseSize_t blocks = size>>2; // 4-ре байта копируются за один раз
	u08 last = size & 0x03;      // остаток
	if(blocks > 0) {
        u32 val = (u32)value<<24 | (u32)value<<16 | (u16)value<<8 | value;
        for(BaseSize_t i = 0; i<blocks; i++) {
			*((u32*)destination) = val;
			destination+=4;
		}
	}
    for(BaseSize_t i = 0; i<last; i++) {
		*((byte_ptr)destination) = value;
		(byte_ptr)destination++;
	}
#elif ARCH == 16
    BaseSize_t blocks = size>>1;		// 2 байта копируются за один раз
	u08 last = size & 0x01; // остаток
    u16 val = (u16)value<<8 | value;
    for(BaseSize_t i = 0; i<blocks; i++) {
        *((u16*)destination) = val;
		destination+=2;
    }
    if(last) *((byte_ptr)destination) = value;
#else
	for (BaseSize_t i = 0; i < size; i++){
		*((byte_ptr)destination + i) = value;
	}
#endif // ARCH
}


#ifdef __cplusplus
}
#endif
