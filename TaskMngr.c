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
extern u32 seconds;
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
    time_res += seconds*TICK_PER_SECOND;
#endif
    return time_res;
}


static void ClockService(){
    GlobalTick++;
#ifdef CLOCK_SERVICE
    if(GlobalTick >= TICK_PER_SECOND) {
    	seconds++;
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

#ifdef QUICK
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

u08 getFreePositionForTask(){
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

void SetTimerTask(TaskMng TPTR, BaseSize_t n, BaseParam_t data, Time_t New_Time)
{
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

bool_t updateTimer(TaskMng TPTR, BaseSize_t n, BaseParam_t data, Time_t New_Time)
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

u08 getFreePositionForTimerTask() {
	return TIME_LINE_LEN - lastTimerIndex;
}
#else // NOT QUICK
// Функция Планировщика (Менеджера) задач. Она запускает ту функцию, которая должна сейчас выполнятся.
/*	Берется первая функция из очереди задач (TaskLine[0]) и проверяется не пустая ли она. Если не пустая, то смещаем всю чередь
на один элемент вверх, а в последний элемент очереди ставим Idle (пустую функцию, включающую режим ожидания МК).
Берем количество параметров из глобального стека и передаем взятой функции, которая берет свои параметры из глобального стека.
*/
static void TaskManager (void)
{
    u08  i; // Просто счетчик. Для смещения списка задач вверх
    BaseSize_t   n;       // Первый аргумент следующей функции (количество параметров)
    BaseParam_t a;       // Второй аргумент для следующей фунции (адрес первой переменной)
    TaskMng Func_point;       // Определяем временную переменную типа указатель на функцию
    // Если пишем для процессора необходимо позаботится об атомарности (отключить прерывания)
    INTERRUPT_DISABLE;			// Идет обращение и изменение глобальной очереди
    // Это необходимо т.к. обращение идет к глобальной очереди, которая может менятся и в прерывании
    Func_point = TaskList[0].Task;  // Берем первую функцию из списка задач
    n=TaskList[0].arg_n;
    a=TaskList[0].arg_p;

    if (Func_point != NULL) // Если наша задачка не пустышка (наиболее вероятное событие)
    {
        i = 1;
        while(TaskList[i].Task != 0 && i < TASK_LIST_LEN) // Пока не наткнулись на пустышку или не дошли до конца списка
        {
            TaskList[i-1].Task  = TaskList[i].Task;  //Смещаем весь список задач на одну позицию вверх
            TaskList[i-1].arg_n = TaskList[i].arg_n;
            TaskList[i-1].arg_p = TaskList[i].arg_p;
            i++;
        }
        TaskList[i-1].Task  = 0;// Последнему указателю в очереде задач присваиваем указатель на функцию простоя
        // Если пишем для процессора, то разрешаем прерывания
        INTERRUPT_ENABLE;		 //(для АВР это asm("sei");).
        Func_point(n, a);        // Вызов функции из очереди задач
        return;
    } //Здесь мы окажемся если задача пустая
    INTERRUPT_ENABLE; //Если список задач пуст Включаем прерывание
    Idle(); // смещать ничего не надо, выходим
}

void SetTask (TaskMng New_Task, BaseSize_t n, BaseParam_t data) // Функция помещает в конец очереди задачу New_Task
{
    u08 index=0; // Счетчик для перебора очереди
    bool_t flag_inter = FALSE;  // флаг состояния прерывания
    if (INTERRUPT_STATUS) //Если прерывания разрешены, то запрещаем их
    {
        INTERRUPT_DISABLE;
        flag_inter = TRUE;                     // И устанавливаем флаг, что мы не в прерывании
    }
    //Постановка задачи в очередь
    //Перебираем очередь в поисках свободного места
    while(TaskList[index].Task != NULL) // Пока не нашли пустышку или не дошли до конца списка
    {
        index++;
        if (index != TASK_LIST_LEN) continue; // Если это еще не последний номер в очереди переходим к следующему
        //Если дошли до конца списка, значит очередь переполнена
        SetTimerTask(New_Task, n, data, TIME_DELAY_IF_BUSY);  //Ставим задачу в очередь(попытаемся записать ее позже)
        if (flag_inter) INTERRUPT_ENABLE;  //предварительно востановив прерывания, если надо.
        return;
    }
    TaskList[index].Task  = New_Task;      //Помещаем на свободное место нашу функцию и список параметров
    TaskList[index].arg_n = n;
    TaskList[index].arg_p = data;
    if (flag_inter) INTERRUPT_ENABLE;	     //Востанавливаем прерывания, если это необходимо.
}

bool_t isEmptyTaskList( void ){
	TaskMng Func_point = NULL;
	while(Func_point != TaskList[0].Task) Func_point = TaskList[0].Task;
	if(Func_point == NULL) return TRUE; // Если очередь пустая (чаще всего так и есть)
	return FALSE;
}

#ifdef SET_FRONT_TASK_ENABLE
//Сами задачи следует делать небольшими. Чтобы как можно быстрее передать управление диспетчеру
// Для задач, которые необходимо сделать быстро эта функция ставит их в начало очереди
void SetFrontTask (TaskMng New_Task, BaseSize_t n, BaseParam_t data) // Функция помещает в НАЧАЛО очереди задачу New_Task
{
    u08 index=TASK_LIST_LEN-1; // Счетчик для перебоpа очереди
    bool_t flag_inter = FALSE;  // флаг состояния прерывания
    if (INTERRUPT_STATUS) //Если прерывания разрешены, то запрещаем их
    {
        INTERRUPT_DISABLE;
        flag_inter = TRUE;    // И устанавливаем флаг, что мы не в прерывании
    }
    for(;index != 0; --index) // Пока не нашли пустышку или не дошли до конца списка
    {
        if(TaskList[index].Task == NULL) continue; // Если текущая задача холостая проходим дальше (нет смысла перемещать ее)
        if (index == TASK_LIST_LEN-1) //Если у нас нет ни одного свободного места
        {      // Т.е. (index == TASK_LIST_LEN-1) и при этом на этом месте не пустышка (см. предыдущее условие) Мы не можем сместить очередь и выходим.
            SetTimerTask(New_Task, n, data, TIME_DELAY_IF_BUSY);
            if (flag_inter) INTERRUPT_ENABLE;  //предварительно востановив прерывания, если надо.
            return;
        }
        // Сюда мы попадем на последней полезной (не пустой) функции. При этом после нее обязательно будет хотя бы одна пустышка
        TaskList[index].Task  = TaskList[index-1].Task;   // Записываем нашу последнюю полезную функцию на одно место назад в очереди
        TaskList[index].arg_n = TaskList[index-1].arg_n;  // И аргументы
        TaskList[index].arg_p = TaskList[index-1].arg_p;
    } // Если мы дошли до этого места у нас на первое место в очереди освобождено
    TaskList[0].Task  = New_Task;      //Помещаем на первое место нашу функцию и список параметров
    TaskList[0].arg_n = n;
    TaskList[0].arg_p = data;
    if (flag_inter) INTERRUPT_ENABLE;	   //Востанавливаем прерывания, если это необходимо.
}
#endif // SET_FRONT_TASK_ENABLE

void delAllTask(void)
{
    bool_t flag_inter = FALSE;  // флаг состояния прерывания
    u08 i = 0;
    if (INTERRUPT_STATUS) //Если прерывания разрешены, то запрещаем их
    {
        INTERRUPT_DISABLE;
        flag_inter = TRUE;    // И устанавливаем флаг, что мы не в прерывании
    }
    while(TaskList[i].Task != NULL && i < TASK_LIST_LEN) // Пока не наткнулись на пустышку или не дошли до конца списка
    {
        TaskList[i].Task = NULL;
        i++;
    }
    for(u08 index = 0; index != TIME_LINE_LEN; ++index)
    {
        MainTimer[index].Task = NULL;
    }
    if (flag_inter) INTERRUPT_ENABLE;	   //Востанавливаем прерывания, если это необходимо.
}

/********************************************************************************************************************
*********************************************************************************************************************
|						 СИСТЕМНЫЙ ТАЙМЕР														|
*********************************************************************************************************************
*********************************************************************************************************************
---------------------------------------------------------------------------------------------------------------------

Служба таймера
В прерывании таймера, вызываемого каждые 1мс (например) выполняетя функция обработчика таймера. Обьявлена как
инлайновая, что встраивает ее прям в обработчик прерывания, без лишних переходов. Экономим стек.
Так как функция работает в прерывании отключать прерывания нет необходимости
*/
static void TimerService (void)	// В прерывании таймера выполняющемся каждые 5мс (можно меньше) выполняется функция обработчика массива таймеров
{
    /*Выполняется функция обработчика массива таймеров. Здесь реализвано поиск всех ненулевых
таймеров и их итерация вплоть до нулевого значения. После того как какой-либо таймер обнулился в список диспетчера задач
дбавляется соответствующая ему задача.*/
    u08 index = 0;
    for(index=0; index < TIME_LINE_LEN; index++)         // Прочесываем массив таймеров
    {
        if (MainTimer[index].Task == NULL) continue;  // Если это Таймер-пустышка переходим к следующему
        if (MainTime[index] != 1) // Если Время истекло и мы находимся в прерывании в соответствующем прерывании таймера
        {
            MainTime[index]--;  // Если таймер не пустышка и не дотикал - тикаем
        }
        else   // Если таймер дотикал
        {
            SetTask (MainTimer[index].Task,  // Ставим нашу задачу в конец очередь
                     MainTimer[index].arg_n,
                     MainTimer[index].arg_p);

            MainTimer[index].Task = NULL;    // А этот таймер глушим.
        }
    }
}

/********************************************************************************************************************

    Функция устанавливающая задачу в очередь по таймеру. На входе адрес перехода (имя задачи) и время в тиках
    службы таймера — миллисекундах. Время двухбайтное, т.е. от 1 до 65535. Если в очереди таймеров уже есть
    таймер с такой задачей, то происходит апдейт времени. Две одинаковых задачи в очереди таймеров не
    возможны. Это можно было бы реализовать, но на практике удобней апдейтить. Число таймеров выбирается
    исходя из одновременно устанавливаемых в очередь задач. Так как работа с глобальной очередью таймеров, то
    надо соблюдать атомарность добавления в очередь. Причем не тупо запрещать/разрешать прерывания, а
    восстанавливать состояние прерываний.
*/
void SetTimerTask(TaskMng TPTR, BaseSize_t n, BaseParam_t data, Time_t New_Time)
{
    u08 index = 0;              // Счетчик очереди таймеров
    bool_t flag_inter = FALSE;  // Флаг состояния прерывания
    for(; index < TIME_LINE_LEN; ++index)	 // Прочесываем очередь таймеров
    {
        if(MainTimer[index].Task == NULL)
        {
            if (INTERRUPT_STATUS)		 	// Проверка в прерывании мы или нет
            {							 	// Значит мы не в прерывании
                INTERRUPT_DISABLE;          // Отключаем прерывание
                flag_inter = TRUE;			// Устанавливаем флаг, что мы не в прерывании
            }
            MainTimer[index].Task  = TPTR;	// Записываем туда новую задачу
            MainTimer[index].arg_n = n;
            MainTimer[index].arg_p = data;
            MainTime[index] = New_Time;	// Устанавливаем выдержку времени для данной задачи
            if(flag_inter) INTERRUPT_ENABLE;		// Востанавливаем прерывание если необходимо.
            return;						// и выходим из функции
        }
    }				//Если мы дошли до конца цикла значит такой задачи мы не нашли
#ifdef MAXIMIZE_OVERFLOW_ERROR
#warning "if queue task timers is overflow programm will be stoped"
    MaximizeErrorHandler();
#else
    if(flag_inter) INTERRUPT_ENABLE;		// Востанавливаем прерывание если необходимо.
    return; //  тут можно сделать return c кодом ошибки - нет свободных таймеров
#endif
}

static u08 findTimer(TaskMng TPTR, BaseSize_t n, BaseParam_t data)
{
    register u08 index=0;
    for(;index < TIME_LINE_LEN; ++index)	 // Прочесываем очередь таймеров в поисках уже существующей задачи
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

bool_t updateTimer(TaskMng TPTR, BaseSize_t n, BaseParam_t data, Time_t New_Time)
{
    u08 index = findTimer(TPTR,n,data);
    if(index < TIME_LINE_LEN) // Если таймер нашли
    {
        bool_t flag_inter = FALSE;
        if (INTERRUPT_STATUS)		 	// Проверка в прерывании мы или нет
        {							 	// Значит мы не в прерывании
            INTERRUPT_DISABLE;          // Отключаем прерывание
            flag_inter = TRUE;			// Устанавливаем флаг, что мы не в прерывании
        }
        MainTime[index]=New_Time; // Мы обновяем таймер этой задачи
        if(flag_inter) INTERRUPT_ENABLE; // Далее востанавливаем прерывания (если необходимо)
        return TRUE;					 // И выходим из функции
    }
    return FALSE;
}

void delTimerTask(TaskMng TPTR, BaseSize_t n, BaseParam_t data)
{
    bool_t flag_inter = FALSE;      // флаг состояния прерывания
    u08 index = findTimer(TPTR,n,data);
    if(index < TIME_LINE_LEN)
    {
        if (INTERRUPT_STATUS)		 	// Проверка в прерывании мы или нет
        {							 	// Значит мы не в прерывании
            INTERRUPT_DISABLE;          // Отключаем прерывание
            flag_inter = TRUE;			// Устанавливаем флаг, что мы не в прерывании
        }
        MainTimer[index].Task = NULL;
        if(flag_inter) INTERRUPT_ENABLE; //Далее востанавливаем прерывания (если необходимо)
    }
}
#endif  // NOT QUICK

#ifdef __cplusplus
}
#endif
