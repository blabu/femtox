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
static void initHeap(void);
#endif

#ifdef DATA_STRUCT_MANAGER
static void initDataStruct( void );
#endif

#ifdef CYCLE_FUNC
static void initCycleTask( void );
static void CycleService( void );
#endif

#ifdef EVENT_LOOP_TASKS
static void EventManager( void );
static void initEventList( void );
#endif

#ifdef CLOCK_SERVICE
volatile static u32 seconds = 0;
#endif
static void ClockService( void );

#ifdef CALL_BACK_TASK
static void initCallBackTask();
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
Time_t getTime(void) {
	Time_t time_res = 0;
    while(time_res != GlobalTick) time_res = GlobalTick;      // Так как переменная у нас двухбайтная
#ifdef CLOCK_SERVICE
    time_res += seconds*TICK_PER_SECOND;
#endif
    return time_res;
}


static void ClockService(){
    GlobalTick++;
#ifdef CLOCK_SERVICE
    if(GlobalTick == TICK_PER_SECOND) {
    	seconds++;
    	GlobalTick = 0;
    }
#endif
}

#ifdef CLOCK_SERVICE

#define SECONDS_2000 946684800UL  /*Колличество секунд с 1970 по 2000*/
#define SECONDS_IN_YEAR 31536000UL
#define SECONDS_IN_DAY 86400UL

#if TIME_INDEX>1
static u08 timeCorrectSummer = TIME_INDEX;
#endif

#define JANUARY   31
#define FEBRUARY  28
#define MARCH	  31
#define APRIL     30
#define MAY		  31
#define JUNE      30
#define JULY	  31
#define AUGUST	  31
#define SEPTEMBER 30
#define OCTOBOR	  31
#define NOVEMBER  30
#define DECEMBER  31
const u08 daysInYear[12] = {JANUARY,FEBRUARY,MARCH,APRIL,MAY,JUNE,JULY,AUGUST,SEPTEMBER,OCTOBOR,NOVEMBER,DECEMBER};

static  u08 getMinutesFromSec(Time_t sec) {
    sec = sec/60; // Определяем сколько всего минут прошло
    sec %= 60; // от 0 до 59 минут
    return (u08)sec;
}

static u08 getHourFromSec(Time_t sec) {
    sec = (sec/3600UL); // Определяем сколько всего часов прошло
    sec %= 24; // от 0 до 23 часов
#ifdef TIME_INDEX
    sec += timeCorrectSummer;
    if(sec > 23) sec -= 24;
#endif
    return (u08)sec;
}

static u16 getYearFromSec(Time_t sec) {
	return (sec/SECONDS_IN_YEAR) + 1970;
}

static u16 getDayInYearFromSec(Time_t sec) { // День в году
	u16 result = (u16)(sec/SECONDS_IN_DAY); // Кол-во дней с 1970 года
	result %= 365;
	// Поправка на высокосные годы
	u16 year  = getYearFromSec(sec);
	u08 dataOffset = 0;
	if(year > 1972) // Больше первого высокосного года
	 	dataOffset = ((year - 1972)>>2);
	result -= dataOffset;
	return result;
}

static u16 getDayAndMonthFromDate(u16 dayInYear) { //LSB - day, MSB - mounth
	u08 month=0;
	u08 day = 0;
	for(; month<12; month++) {
		if(dayInYear > daysInYear[month]) {
			dayInYear -= (daysInYear[month] );  // Учитываем что день в месяце начинается с 0
		}
		else break;
	}
	day = dayInYear & 0x1F;
	month+=1;
#if TIME_INDEX>1
#ifdef SUMMER_TIME
	//Если месяц больше марта (т.е. апрель или дальше) и меньше ноября (т.е. окябрь или меньше)
	if(month > 3 || month < 11) timeCorrectSummer = TIME_INDEX;
	else timeCorrectSummer = TIME_INDEX-1;
#else
	timeCorrectSummer = TIME_INDEX;
#endif
#endif
	return ((u16)month<<8) | day;
}

Time_t getAllSeconds(){
	u32 temp = 0;
	while(temp != seconds) temp = seconds;
	return temp;
}

u08 getMinutes(){
	return getMinutesFromSec(getAllSeconds());
}

u08 getHour(){
	return getHourFromSec(getAllSeconds());
}

u16 getYear(){
	return getYearFromSec(getAllSeconds());
}

u16 getDayInYear() { // День в году
	return getDayInYearFromSec(getAllSeconds());
}

//LSB - day, MSB - mounth
u16 getDayAndMonth() {
	return getDayAndMonthFromDate(getDayInYear());
}

u08 getDaysInMonth(u08 month) {
	return daysInYear[month];
}

void setSeconds(u32 sec) {
	while(seconds != sec) seconds = sec;
}

Date_t getDateFromSeconds(Time_t sec){
	Date_t res;
	res.sec = sec%60;
	res.min = getMinutesFromSec(sec);
	res.hour = getHourFromSec(sec);
	u16 temp = getDayAndMonthFromDate(getDayInYearFromSec(sec));
	res.day = (u08)(temp & 0xFF);
	res.mon = (u08)(temp>>8);
	res.year = (sec/SECONDS_IN_YEAR) + 1970;
	return res;
}

/*
 * return >0 if date1 > date2
 * return 0 if date = date2
 * return <0 if date1 < date2
 */
s08 compareDates(Date_t* date1, Date_t* date2){
	if(date1 == NULL || date2 == NULL) return 0;
	s16 different = date1->year - date2->year;
	if(different) return (s08)(different);
	different = date1->mon - date2->mon;
	if(different) return (s08)(different);
	different = date1->day - date2->day;
	if(different) return (s08)(different);
	different = date1->hour - date2->hour;
	if(different) return (s08)(different);
	different = date1->min - date2->min;
	if(different) return (s08)(different);
	different = date1->sec - date2->sec;
	if(different) return (s08)(different);
	return 0;
}

#include "MyString.h"
// input date must have format YY.MM.DD hh:mm:ss
void setDate(string_t date) {
	char tempStr[4];
	if(strSize(date) < 17) { return;}
	strCopy(tempStr,date,2,0);
	u08 year = toIntDec(tempStr);
	if(year < 70) { // значит введена дата после 2000-го
		year += 30;
	}
	u08 dayOffset = (year+2)>>2; // Поправка в днях на высокосные годы
	strClear(tempStr);
	strCopy(tempStr,date,2,3);
	u08 mounth = toIntDec(tempStr);
	strClear(tempStr);
	strCopy(tempStr,date,2,6);
	u08 day = toIntDec(tempStr);
	strClear(tempStr);
	strCopy(tempStr,date,2,9);
	u08 hour = toIntDec(tempStr);
	strClear(tempStr);
	strCopy(tempStr,date,2,12);
	u08 minutes = toIntDec(tempStr);
	strClear(tempStr);
	strCopy(tempStr,date,2,15);
	u08 sec = toIntDec(tempStr);
	Time_t tempSeconds = year*SECONDS_IN_YEAR + (daysInYear[mounth-1] + day + dayOffset)*SECONDS_IN_DAY + hour*3600 + minutes*60 + sec;
	setSeconds(tempSeconds);
}
#endif

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

void ResetFemtOS(void)
{
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
    return 1; //  тут можно сделать return c кодом ошибки - нет свободных таймеров
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

//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//                                  РЕАЛИЗАЦИЯ СОБЫТИЙ
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
#ifdef EVENT_LOOP_TASKS
#if EVENT_LIST_SIZE > 0xFE
#error "incompatible size"
#endif
typedef struct
{
    Predicat_t      Predicat;   // Указатель на функцию условия
    CycleFuncPtr_t  CallBack;
} EventsType;
EventsType EventList[EVENT_LIST_SIZE];

static void initEventList()
{
    for(u08 i = 0; i<EVENT_LIST_SIZE; i++)
    {
        EventList[i].Predicat = NULL;
    }
}

static void EventManager( void )
{
    for(u08 i = 0; i<EVENT_LIST_SIZE; i++)
    {
        if(EventList[i].Predicat == NULL) break;        // При первом
        if(EventList[i].Predicat()) EventList[i].CallBack();
    }
}

bool_t CreateEvent(Predicat_t condition, CycleFuncPtr_t effect) // Регистрирует новое событие в списке событий
{
    u08 i = 0;
    bool_t flag_inter = FALSE;
    if (INTERRUPT_STATUS)		 	// Проверка в прерывании мы или нет
    {							 	// Значит мы не в прерывании
        INTERRUPT_DISABLE;          // Отключаем прерывание
        flag_inter = TRUE;			// Устанавливаем флаг, что мы не в прерывании
    }
    for(;i < EVENT_LIST_SIZE; i++)
    {
        if(EventList[i].Predicat == NULL) break; // find empty event task
    }
    if(i == EVENT_LIST_SIZE) {if(flag_inter) INTERRUPT_ENABLE; return FALSE;} // Событие невозможно создать т.к. очередь событий переполнена
    EventList[i].Predicat = condition;
    EventList[i].CallBack = effect;
    if(flag_inter) INTERRUPT_ENABLE; //Далее востанавливаем прерывания (если необходимо)
    return TRUE;
}

void delEvent(Predicat_t condition)
{
    u08 i = 0;
    bool_t flag_inter = FALSE;
    u08 countDeletedEvent = 0;
    if (INTERRUPT_STATUS)		 	// Проверка в прерывании мы или нет
    {							 	// Значит мы не в прерывании
        INTERRUPT_DISABLE;          // Отключаем прерывание
        flag_inter = TRUE;			// Устанавливаем флаг, что мы не в прерывании
    }
    for(;i<EVENT_LIST_SIZE;i++)     // Выполняем поиск нашего события
    {
        if(EventList[i].Predicat == NULL) break;    // Если дошли до пустого, а значит последнего выходим из цикла
        if(EventList[i].Predicat == condition) // find event
        {
            EventList[i].Predicat = NULL;   // Удаляем событие
            countDeletedEvent++;            // и увеличиваем счетчик удаленных событий на один
            continue;
        }
        if(countDeletedEvent)  // Если факт того что задача была удалена установлен
        {
            EventList[i-countDeletedEvent].CallBack = EventList[i].CallBack;  // Переносим текущее событие на место удаленного
            EventList[i-countDeletedEvent].Predicat = EventList[i].Predicat;  // Переносим текущее
            EventList[i].Predicat = NULL;   // А текущее место освобождаем
        }
    }
    if(flag_inter) INTERRUPT_ENABLE;
}
#endif //EVENT_LOOP_TASKS

//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//                                  РЕАЛИЗАЦИЯ СТРУКТУР ДАННЫХ
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
#ifdef DATA_STRUCT_MANAGER
#if ArraySize > 0xFE
#error "incompatible size"
#endif
typedef struct
{
    void* Data;               // Указатель на начало очереди
    BaseSize_t firstCount;    // Указатель на первый свободный элемент абстрактной структуры данных
    BaseSize_t lastCount;     // Указатель на последний фактический элемент в абстрактной структуре данных
    BaseSize_t sizeElement;   // Размер одного элемента абстрактной структуры данных
    BaseSize_t sizeAllElements;// Общий размер в количествах элементов в абстрактной структуре данных
} AbstractDataType;
static AbstractDataType Data_Array[ArraySize];   // Собственно сам массив абстрактных структур данных

/***************************/
/***************************/
void showAllDataStruct(void)
{
}

static inline u08 findNumberDataStruct(const void* const Data)
{
    register u08 i = 0;
    for(; i<ArraySize; i++) // находим абстрактную структуру данных
    {
        if(Data_Array[i].Data == Data) break;
    }
    return i;
}

//dst - адрес в памяти КУДА копируем src - адрес в памяти ОТКУДА копируем n - количество БАЙТ копируемых
static void MyMemcpy(void *dst, const void* src, BaseSize_t n) // Копируем n байт
{
    for (BaseSize_t i = 0; i < n; i++){ //Копирование будет побайтное
        *((byte_ptr)dst + i) = *((byte_ptr)src + i); // Выполняем копирование данных
    }
}

static void initDataStruct(void)  // Инициализация абстрактной структуры данных
{
    for(register u08 i = 0; i<ArraySize; i++)
    {
        Data_Array[i].firstCount = 0;
        Data_Array[i].lastCount = 0;
        Data_Array[i].Data = NULL;
        Data_Array[i].sizeElement = 0;
        Data_Array[i].sizeAllElements = 0;
    }
}

// Функция создает абстрактную структуру данных (резервирует место под нее в глобальном массиве)
// sizeElement - размер одного элемента в БАЙТАХ, sizeAll - размер очереди в ЭЛЕМЕНТАХ
u08 CreateDataStruct(const void* D, const BaseSize_t sizeElement, const BaseSize_t sizeAll){
    bool_t flag_int = FALSE;
    register u08 i = 0;
    for(; i<ArraySize; i++) // Ищем пустое место в списке для новой структуры данных
    {
        if(Data_Array[i].Data == D) return OTHER_ERROR; // Если такая структура уже есть
        if(Data_Array[i].Data == NULL) break;
    }
    if(i == ArraySize) return NOT_FAUND_DATA_STRUCT_ERROR;
    if(INTERRUPT_STATUS) {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    Data_Array[i].Data = (void*)D; // Адрес начала
    Data_Array[i].sizeElement = sizeElement; // размер одного элемента в байтах
    Data_Array[i].sizeAllElements = sizeAll; // Размер всей очереди в элементах
    Data_Array[i].firstCount= sizeAll;
    Data_Array[i].lastCount = sizeAll;
    if(flag_int) INTERRUPT_ENABLE;
    return EVERYTHING_IS_OK;
}

// Удаляем абстрактную структуру данных
u08 delDataStruct(const void* Data)  // Удаляем из массива абстрактную структуру данных с заданным идентификатором
{
    u08 i = findNumberDataStruct(Data);
    if(i == ArraySize) return NOT_FAUND_DATA_STRUCT_ERROR;  // Если такой не существует в массиве, выдаем ошибку
    Data_Array[i].Data = NULL;    // Если абстрактная структура данных есть удаляем ее
    return EVERYTHING_IS_OK;
}

u08 PutToCycleDataStruct(const void* Elem, const void* Array) {
    bool_t flag_int = FALSE;
    register u08 i = findNumberDataStruct(Array);
    if(i == ArraySize) return NOT_FAUND_DATA_STRUCT_ERROR;    // Если мы не нашли абстрактную структуру данных с указанным идентификтором выходим
    BaseSize_t frontCount = (Data_Array[i].firstCount < Data_Array[i].sizeAllElements)? Data_Array[i].firstCount+1:0;
    if(INTERRUPT_STATUS){
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    unsigned int offset = Data_Array[i].firstCount * Data_Array[i].sizeElement; //вычисляем смещение в байтах
    void* dst = (void*)((byte_ptr)Data_Array[i].Data + offset);     // Определяем адресс куда копировать
    MyMemcpy(dst, Elem, Data_Array[i].sizeElement); // Вставляем наш элемент
    Data_Array[i].firstCount = frontCount;
    if(Data_Array[i].lastCount < Data_Array[i].sizeAllElements) Data_Array[i].lastCount++;
    if(flag_int) INTERRUPT_ENABLE;
    return EVERYTHING_IS_OK;
}

u08 GetFromCycleDataStruct(void* returnValue, const void* Array)
{
    bool_t flag_int = FALSE;
    register u08 i = findNumberDataStruct(Array);
    if(i == ArraySize) return NOT_FAUND_DATA_STRUCT_ERROR;    // Если в массиве нет искомой абстрактной структуры данных с заданным идентификатором
    if(Data_Array[i].lastCount > 0) { // Если есть какие либо данные
    	if(INTERRUPT_STATUS) {
    		flag_int = TRUE;
    		INTERRUPT_DISABLE;
    	}
    	Data_Array[i].lastCount--;
    	Data_Array[i].firstCount = (Data_Array[i].firstCount)? Data_Array[i].firstCount : Data_Array[i].sizeAllElements;
    	Data_Array[i].firstCount--;
    	unsigned int offset = Data_Array[i].firstCount * Data_Array[i].sizeElement;  // Определяем смещение на элемент, который надо достать
    	void* dst = (void*)((byte_ptr)Data_Array[i].Data + offset);     // Записываем адрес памяти свободной ячейки
    	MyMemcpy(returnValue, dst, Data_Array[i].sizeElement);   // Если структура данных найдена, читаем от туда первый (самый старый) элемент
    	if(flag_int) INTERRUPT_ENABLE;  // Если все происходило не в прерывании восстанавливаем разрешение прерываний
    	return EVERYTHING_IS_OK;   // Если все впорядке возвращаем ноль
    }
    else {
    	*((byte_ptr)returnValue) = 0;
    	return OVERFLOW_OR_EMPTY_ERROR;
    }
}

//Положить элемент Elem в начало структуры данных Array
u08 PutToFrontDataStruct(const void* Elem, const void* Array)
{
    bool_t flag_int = FALSE;
    register u08 i = findNumberDataStruct(Array);
    if(i == ArraySize) return NOT_FAUND_DATA_STRUCT_ERROR;    // Если мы не нашли абстрактную структуру данных с указанным идентификтором выходим
    BaseSize_t frontCount = (Data_Array[i].firstCount < Data_Array[i].sizeAllElements)? Data_Array[i].firstCount+1:0;
    if(frontCount == Data_Array[i].lastCount) return OVERFLOW_OR_EMPTY_ERROR;  // Если после добавления мы догоним lastCount, значит структура заполнена
    if(INTERRUPT_STATUS)
    {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    unsigned int offset = Data_Array[i].firstCount * Data_Array[i].sizeElement; //вычисляем смещение в байтах
    void* dst = (void*)((byte_ptr)Data_Array[i].Data + offset);     // Определяем адресс куда копировать
    MyMemcpy(dst, Elem, Data_Array[i].sizeElement); // Вставляем наш элемент
    Data_Array[i].firstCount = frontCount;
    if(flag_int) INTERRUPT_ENABLE;
    return EVERYTHING_IS_OK;
}

// Полоожить элемент Elem в конец абстрактной структуры данных Array
u08 PutToEndDataStruct(const void* Elem, const void* Array){
    bool_t flag_int = FALSE;
    register u08 i = findNumberDataStruct(Array);
    if(i == ArraySize) return NOT_FAUND_DATA_STRUCT_ERROR;    // Если мы не нашли абстрактную структуру данных с указанным идентификтором выходим
    BaseSize_t endCount = (Data_Array[i].lastCount)? Data_Array[i].lastCount:Data_Array[i].sizeAllElements;
    endCount--;
    if(endCount == Data_Array[i].firstCount) return OVERFLOW_OR_EMPTY_ERROR;  // Если она заполнена полностью писать некуда
    if(INTERRUPT_STATUS)
    {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    unsigned int offset = endCount * Data_Array[i].sizeElement;  // Определяем смещение на свободную позицию (количество байт)
    void* dst = (void*)((byte_ptr)Data_Array[i].Data + offset); // Записываем адрес памяти начала свободной ячейки
    MyMemcpy(dst, Elem, Data_Array[i].sizeElement);  // Копируем все байты Elem в массив Array с заданным смещением
    Data_Array[i].lastCount = endCount;           // После копирования инкрементируем текущую позицию
    if(flag_int) INTERRUPT_ENABLE;
    return EVERYTHING_IS_OK;
}

u08 GetFromFrontDataStruct(void* returnValue, const void* Array) // Достаем элемент с начала структуры данных
{
    bool_t flag_int = FALSE;
    register u08 i = findNumberDataStruct(Array);
    if(i == ArraySize) return NOT_FAUND_DATA_STRUCT_ERROR;    // Если в массиве нет искомой абстрактной структуры данных с заданным идентификатором
    if(Data_Array[i].firstCount == Data_Array[i].lastCount) {return OVERFLOW_OR_EMPTY_ERROR;} // Если она пустая читать нечего
    if(INTERRUPT_STATUS)
    {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    Data_Array[i].firstCount = (Data_Array[i].firstCount)? Data_Array[i].firstCount : Data_Array[i].sizeAllElements;
    Data_Array[i].firstCount--;
    unsigned int offset = Data_Array[i].firstCount * Data_Array[i].sizeElement;  // Определяем смещение на элемент, который надо достать
    void* dst = (void*)((byte_ptr)Data_Array[i].Data + offset);     // Записываем адрес памяти свободной ячейки
    MyMemcpy(returnValue, dst, Data_Array[i].sizeElement);   // Если структура данных найдена, читаем от туда первый (самый старый) элемент
    if(flag_int) INTERRUPT_ENABLE;  // Если все происходило не в прерывании восстанавливаем разрешение прерываний
    return EVERYTHING_IS_OK;   // Если все впорядке возвращаем ноль
}

u08 GetFromEndDataStruct(void* returnValue, const void* Array) // Достаем элемент с конца структуры данных
{
    bool_t flag_int = FALSE;
    register u08 i = findNumberDataStruct(Array);
    if(i == ArraySize) return NOT_FAUND_DATA_STRUCT_ERROR;    // Если в массиве нет искомой абстрактной структуры данных с заданным идентификатором
    if(Data_Array[i].lastCount == Data_Array[i].firstCount) return OVERFLOW_OR_EMPTY_ERROR; //Проверка пустая ли структура данных
    if(INTERRUPT_STATUS)
    {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    unsigned int offset = Data_Array[i].lastCount * Data_Array[i].sizeElement;
    void* src = (void*)((byte_ptr)Data_Array[i].Data+offset);
    MyMemcpy(returnValue, src, Data_Array[i].sizeElement);   // Если структура данных найдена, читаем от туда первый (самый старый) элемент
    Data_Array[i].lastCount = (Data_Array[i].lastCount < Data_Array[i].sizeAllElements)? Data_Array[i].lastCount+1:0;
    if(flag_int) INTERRUPT_ENABLE;  // Если все происходило не в прерывании восстанавливаем разрешение прерываний
    return EVERYTHING_IS_OK;   // Если все впорядке возвращаем ноль
}

u08 delFromFrontDataStruct(const void* const Data)
{
    bool_t flag_int = FALSE;
    register u08 i = findNumberDataStruct(Data);
    if(i == ArraySize) return NOT_FAUND_DATA_STRUCT_ERROR;    // Если в массиве нет искомой абстрактной структуры данных с заданным идентификатором
    if(Data_Array[i].firstCount == Data_Array[i].lastCount) {return OVERFLOW_OR_EMPTY_ERROR;} // Если она пустая читать нечего
    if(INTERRUPT_STATUS)
    {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    Data_Array[i].firstCount = (Data_Array[i].firstCount)? Data_Array[i].firstCount : Data_Array[i].sizeAllElements; // Закольцовываем счетчики
    Data_Array[i].firstCount--;
    if(flag_int) INTERRUPT_ENABLE;  // Если все происходило не в прерывании восстанавливаем разрешение прерываний
    return EVERYTHING_IS_OK;   // Если все впорядке возвращаем ноль
}

u08 delFromEndDataStruct(const void* const Data)
{
    bool_t flag_int = FALSE;
    register u08 i = findNumberDataStruct(Data);
    if(i == ArraySize) return NOT_FAUND_DATA_STRUCT_ERROR;    // Если в массиве нет искомой абстрактной структуры данных с заданным идентификатором
    if(Data_Array[i].lastCount == Data_Array[i].firstCount) return OVERFLOW_OR_EMPTY_ERROR; //Проверка пустая ли структура данных
    if(INTERRUPT_STATUS)
    {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    Data_Array[i].lastCount = (Data_Array[i].lastCount < Data_Array[i].sizeAllElements)? Data_Array[i].lastCount+1:0;
    if(flag_int) INTERRUPT_ENABLE;  // Если все происходило не в прерывании восстанавливаем разрешение прерываний
    return EVERYTHING_IS_OK;   // Если все впорядке возвращаем ноль  
}

u08 peekFromFrontData(void* returnValue, const void* Array) {
    bool_t flag_int = FALSE;
    register u08 i = findNumberDataStruct(Array);
    if(i == ArraySize) return NOT_FAUND_DATA_STRUCT_ERROR;    // Если в массиве нет искомой абстрактной структуры данных с заданным идентификатором
    if(Data_Array[i].firstCount == Data_Array[i].lastCount) {return OVERFLOW_OR_EMPTY_ERROR;} // Если она пустая читать нечего
    if(INTERRUPT_STATUS) {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    u08 count = (Data_Array[i].firstCount)? Data_Array[i].firstCount-1 : Data_Array[i].sizeAllElements-1;
    unsigned int offset = count * Data_Array[i].sizeElement;  // Определяем смещение на элемент, который надо достать
    void* dst = (void*)((byte_ptr)Data_Array[i].Data + offset); // Записываем адрес памяти свободной ячейки
    MyMemcpy(returnValue, dst, Data_Array[i].sizeElement);   // Если структура данных найдена, читаем от туда первый (самый старый) элемент
    if(flag_int) INTERRUPT_ENABLE;  // Если все происходило не в прерывании восстанавливаем разрешение прерываний
    return EVERYTHING_IS_OK;   // Если все впорядке возвращаем ноль
}

u08 peekFromEndData(void* returnValue, const void* Array)
{
    bool_t flag_int = FALSE;
    register u08 i = findNumberDataStruct(Array);
    if(i == ArraySize) return NOT_FAUND_DATA_STRUCT_ERROR;    // Если в массиве нет искомой абстрактной структуры данных с заданным идентификатором
    if(Data_Array[i].lastCount == Data_Array[i].firstCount) return OVERFLOW_OR_EMPTY_ERROR; //Проверка пустая ли структура данных
    if(INTERRUPT_STATUS)
    {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    unsigned int offset = Data_Array[i].lastCount * Data_Array[i].sizeElement;
    void* src = (void*)((byte_ptr)Data_Array[i].Data+offset);
    MyMemcpy(returnValue, src, Data_Array[i].sizeElement);   // Если структура данных найдена, читаем от туда первый (самый старый) элемент
    if(flag_int) INTERRUPT_ENABLE;  // Если все происходило не в прерывании восстанавливаем разрешение прерываний
    return EVERYTHING_IS_OK;   // Если все впорядке возвращаем ноль
}

void clearDataStruct(const void * const Data)
{
    bool_t flag_int = FALSE;
    register u08 i = findNumberDataStruct(Data);
    if(i == ArraySize) return;
    if(INTERRUPT_STATUS)
    {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    Data_Array[i].firstCount= Data_Array[i].sizeAllElements; // Очищаем от данных наш массив
    Data_Array[i].lastCount = Data_Array[i].sizeAllElements;
    if(flag_int) INTERRUPT_ENABLE;  // Если все происходило не в прерывании восстанавливаем разрешение прерываний
}

bool_t isEmptyDataStruct(const void* const Data)
{
    register u08 i = findNumberDataStruct(Data);
    if(i == ArraySize) return TRUE; // Если такой структуры нет она точно пустая
    bool_t res = (Data_Array[i].firstCount == Data_Array[i].lastCount); // Если они равны друг другу значит пустая
    return res;
}

void for_each(const void* const Array, TaskMng tsk)
{
    register u08 i = findNumberDataStruct(Array);
    if(i == ArraySize) return;
    for(BaseSize_t j=Data_Array[i].firstCount; j!=Data_Array[i].lastCount;)
    {
      BaseParam_t ptr = (BaseParam_t)((byte_ptr)Data_Array[i].Data + j*Data_Array[i].sizeElement);
      tsk(0,ptr);
      j=(j < Data_Array[i].sizeAllElements)? j+1:0;
    }
}

#endif //DATA_STRUCT_MANAGER


#ifdef MUTEX_ENABLE
volatile static u08  MyMutex = 0; // 8 - возможных мьютексов

bool_t getMutex(const u08 mutexNumb, TaskMng TPTR, BaseSize_t n, BaseParam_t data)
{
    if(mutexNumb >= 8) return FALSE;// Если номер мьютекса больше возможного варианта выходим из функции
    if(MyMutex & (1<<mutexNumb)) // Если мьютекс с таким номером захвачен
    {
        SetTimerTask(TPTR, n, data, TIME_DELAY_IF_BUSY); // Попытаем счастья позже
        return TRUE; // Мьютекс уже захвачен кем-то возвращаем результат
    }
    register bool_t flag_int = FALSE;
    // Здесь окажемся если мьютекс свободен
    if(INTERRUPT_STATUS)
    {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    MyMutex |= 1<<mutexNumb;
    if(flag_int) INTERRUPT_ENABLE;
    return FALSE; // Все впорядке мьютекс успешно захвачен этой функцией.
}

bool_t freeMutex(const u08 mutexNumb)
{
    if(mutexNumb >= 8) return FALSE;// Если номер мьютекса больше возможного варианта выходим из функции
    register bool_t flag_int = FALSE;
    if(INTERRUPT_STATUS)
    {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    MyMutex &= ~(1<<mutexNumb);
    if(flag_int) INTERRUPT_ENABLE;
    return TRUE;
}

#endif //MUTEX_ENABLE


#ifdef CYCLE_FUNC
#if TIMERS_ARRAY_SIZE > 0xFE
#error "incompatible size"
#endif
/*
Эта часть содержит набор функция для реализации циклически выполняемых коротких функций
Функции выполняются внутри прерывания и не должны содержать задержек или разрешений прерываний
*/
volatile static struct
{
    CycleFuncPtr_t Call_Back;
    Time_t value;
    Time_t time;
    bool_t flagToManager;
}Timers_Array[TIMERS_ARRAY_SIZE];

static void initCycleTask(void)
{
    for(u08 i=0; i<TIMERS_ARRAY_SIZE; i++)
    {
        Timers_Array[i].Call_Back = 0;
        Timers_Array[i].value = 0;
        Timers_Array[i].time = 0;
    }
}

void SetCycleTask(Time_t time, CycleFuncPtr_t CallBack, bool_t toManager) {
    bool_t flag_int = FALSE;
    if(INTERRUPT_STATUS) {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    for(register u08 i = 0; i<TIMERS_ARRAY_SIZE; i++){
        if(Timers_Array[i].value) continue; // Если таймер уже занят (не нулевой) переходим к следющему
        Timers_Array[i].Call_Back = CallBack;  // Запоминаем новый колбэк
        Timers_Array[i].flagToManager = toManager;      // Флаг определяет выполняется задача в таймере или ставится в глобальную очередь
        Timers_Array[i].value = time;       // Первый свободный таймер мы займем своей задачей
        Timers_Array[i].time = time;
        break;                          // выходим из цикла
    }
    if(flag_int) INTERRUPT_ENABLE;
}

void delCycleTask(CycleFuncPtr_t CallBack) {
    bool_t flag_int = FALSE;
    if(INTERRUPT_STATUS)
    {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    u08 countDeletedTask = 0;  // Количество удаленных задач
    for(register u08 i = 0; i<TIMERS_ARRAY_SIZE; i++)
    {
        if(!Timers_Array[i].value) break; // Если наткнулись на пустой таймер выходим из цикла(дальше искать нет смысла)
        if(Timers_Array[i].Call_Back == CallBack) // Если нашли нашу задачу
        {
            countDeletedTask++;         // Фиксируем факт того, что задача была удалена (чтобы все следующие задачи сместить наверх)
            Timers_Array[i].value = 0;  // Удаляем её, а все последующие не нулевые функции смещаем на одну позицию наверх
            continue;                   // Переходим к следующей позиции
        }
        if(countDeletedTask)  // Если факт того что задача была удалена установлен
        {
            Timers_Array[i-countDeletedTask].Call_Back = Timers_Array[i].Call_Back;  // Переносим текущую
            Timers_Array[i-countDeletedTask].time = Timers_Array[i].time;  // Переносим текущую
            Timers_Array[i-countDeletedTask].value = Timers_Array[i].value;
            Timers_Array[i-countDeletedTask].flagToManager = Timers_Array[i].flagToManager;
            Timers_Array[i].value = 0; // После перемещения удаляем текущую задачу
        }

    }
    if(flag_int) INTERRUPT_ENABLE;
}


static void CycleService(void) {
    register u08 i = 0;
    while(Timers_Array[i].value) // Перебираем массив таймеров пока не встретили пустышку
    {
    	Timers_Array[i].time--;// Если нашли не путой таймер тикаем
        if(!Timers_Array[i].time) // Если таймер дотикал
    	{
            Timers_Array[i].time = Timers_Array[i].value;     // Если таймер дотикал обновляем его значение
            if(!Timers_Array[i].flagToManager) // Если флаг поставить задачу в очередь не выставлен выполняем функцию здесь же
                (*Timers_Array[i].Call_Back)();
            else
                SetTask((TaskMng)Timers_Array[i].Call_Back,0,0); // Если флаг установлен ставим задачу в очередь таймеров
        }
        i++;
        if(i>TIMERS_ARRAY_SIZE) break;
    }
}
#endif  //CYCLE_FUNC

#ifdef GLOBAL_FLAGS
globalFlags_t GlobalFlags = 0;

void setFlags(globalFlags_t flagMask) {
	bool_t flag_int = FALSE;
	if(INTERRUPT_STATUS){
		flag_int = TRUE;
		INTERRUPT_DISABLE;
	}
	GlobalFlags |= flagMask;
	if(flag_int) INTERRUPT_ENABLE;
}

void clearFlags(globalFlags_t flagMask) {
	bool_t flag_int = FALSE;
	if(INTERRUPT_STATUS){
		flag_int = TRUE;
		INTERRUPT_DISABLE;
	}
	GlobalFlags &= ~flagMask;
	if(flag_int) INTERRUPT_ENABLE;
}

bool_t getFlags(globalFlags_t flagMask){
	if(GlobalFlags & flagMask) return TRUE;
	return FALSE;
}

globalFlags_t getGlobalFlags(){
	globalFlags_t result = 0;
	while(result != GlobalFlags) result = GlobalFlags;
	return result;
}
#endif


#ifdef ALLOC_MEM
/*
Функции работы с кучей. Выделение и удаление памяти в куче. Максмально единоразово можно выделить до 127 байт
Перед блоком памяти хранится байт с размером этого блока (0...6 биты), а последний бит определяет занята
эта память или свободна (поэтому до 127 байт единоразово)
*/
#if HEAP_SIZE  > 0xFFFF
#error "incompatible size"
#endif
static u08 heap[HEAP_SIZE];  // Сама куча
static u16 sizeAllFreeMemmory;

static void initHeap(void)
{
  if(heap == NULL) {
      heap[0]=(1<<7)+1; // Заблокируем начало памяти для выделения
  }
}

u16 getFreeMemmorySize(){
    return sizeAllFreeMemmory;
}

void defragmentation(void){
    u16 i = 0;
    u08 blockSize = 0;
    sizeAllFreeMemmory=HEAP_SIZE;
    bool_t flag_int = FALSE;
    while(i < HEAP_SIZE)    // Пока не закончится куча
    {
        u08 currentBlockSize = heap[i]&0x7F; //Выделяем размер блока (младшие 7 байт)
        if(!currentBlockSize) return;   // Если размер нулевой, значит выделения памяти еще не было
        if(heap[i] & (1<<7))    // Если блок памяти занят
        {
            blockSize = 0;
            i += currentBlockSize + 1;  // переходим к концу этого блока
            sizeAllFreeMemmory -= currentBlockSize + 1;
            continue;
        }
        if(blockSize) //Если блок памяти свободен
        {
            u08 SumBlock = (u08)(blockSize + currentBlockSize + 1);
            if(SumBlock < 127)
            {
                if(INTERRUPT_STATUS)
                {
                    flag_int = TRUE;
                    INTERRUPT_DISABLE;
                }
                heap[i - (blockSize+1)] = SumBlock;
                blockSize = SumBlock;
                i += currentBlockSize + 1;
                if(flag_int) INTERRUPT_ENABLE;
                continue;
            }
        }
        blockSize = currentBlockSize;
        i += blockSize + 1;
    }
}

byte_ptr allocMem(u08 size)  //size - до 127 размер блока выделяемой памяти
{
    if(size > 127 || !size) return NULL;  // Если попросили больше чем можем дать возвращаем ноль
    if(size == 1) size = 2;
    u16 i = 0;  // Поиск свободного места начнем с нулевого элемента, максимум определен размером u16
    bool_t flag_int = FALSE;
    if(INTERRUPT_STATUS)
    {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    while((i+size+1) < HEAP_SIZE) // Пока мы можем выделить тот объем памяти который у нас попросили
    {
        u08 blockSize = heap[i] & 0x7F;  // Вычисляем размер следующего блока памяти
        if(blockSize == 0)     // Если память здесь еще не выделялась
        {
            heap[i] = (1<<7) + size;    // Выделяем нужный объем памяти
            break;
        }
        if((heap[i] >> 7) ||   //  Если этот блок занят (последний бит равен 1)
            blockSize < size)   // или размер этого блока слишком маленький
        {
            i += (blockSize+1); // Перескакиваем через этот блок
            continue;
        }
        if((blockSize - size) < 2)  // Если размеры блоков совпали с точность до плюс одного байта
        {
            heap[i] |= (1<<7);
            break;
        }
        // Сюда мы попадем если размер свободной памяти строго больше размера нам необходимой памяти
        // хотябы на 2 байта (Один байт полезный и один служебный)
        heap[i] = (1<<7) + size;
        heap[i+size+1] = blockSize - size - 1; // Следующий пустой блок будет на один байт короче (этот байт служебный)
        break;
    }
    if(flag_int) INTERRUPT_ENABLE;
    if((i+size+1) >= HEAP_SIZE) return NULL; // Если мы вышли из цикла по причине окончания кучи, вернем ноль
    return (heap + i + 1); // Иначе вернем валидный указатель на начало массива
}

void freeMem(byte_ptr data)
{
    if(data > heap &&
       data < heap + HEAP_SIZE)  // Если мы передали валидный указатель
    {
        *(data-1) &= ~(1<<7); // Очистим флаг занятости данных (не трогая при этом сами данные и их размер)
    }
}

#endif //ALLOC_MEM


#ifdef CALL_BACK_TASK
#if CALL_BACK_TASK_LIST_LEN > 0xFE
#error "incompatible size"
#endif

static void* labelPointer[CALL_BACK_TASK_LIST_LEN]; // Массив указателей на функции по завершении которых следует вызвать callBack (по сути метка колбэка)
static TaskList_t callBackList[CALL_BACK_TASK_LIST_LEN];	// Указатель на функцию которая будет вызвана

static void initCallBackTask(){
	for(u08 index = 0;index<CALL_BACK_TASK_LIST_LEN; index++){
		labelPointer[index] = NULL;
	}
}

static u08 findCallBack(void* labelPtr){
	u08 index = 0;
	for(;index < CALL_BACK_TASK_LIST_LEN; index++){
		if(labelPointer[index] == labelPtr) break;
	}
	return index;
}

u08 registerCallBack(TaskMng task, BaseSize_t arg_n, BaseParam_t arg_p, void* labelPtr){
	bool_t flag_isr = FALSE;
	if(INTERRUPT_STATUS){
		flag_isr = TRUE;
		INTERRUPT_DISABLE;
	}
	u08 i = findCallBack(NULL);
	if(i == CALL_BACK_TASK_LIST_LEN) {
		if(flag_isr) INTERRUPT_ENABLE;
		return OVERFLOW_OR_EMPTY_ERROR;
	}
	callBackList[i].Task = task;
	callBackList[i].arg_n = arg_n;
	callBackList[i].arg_p = arg_p;
	labelPointer[i] = labelPtr;
	if(flag_isr) INTERRUPT_ENABLE;
	return EVERYTHING_IS_OK;
}

void execCallBack(void* labelPtr){
	for(u08 i = 0; i < CALL_BACK_TASK_LIST_LEN; i++){
		if(labelPointer[i] == labelPtr){
			if(callBackList[i].Task != NULL) {
			 	SetTask(callBackList[i].Task,callBackList[i].arg_n,callBackList[i].arg_p);		
			}
			labelPointer[i] = NULL;
	    }
	}
}

void execErrorCallBack(BaseSize_t errorCode, void* labelPtr){
	for(u08 i = 0; i < CALL_BACK_TASK_LIST_LEN; i++){
		if(labelPointer[i] == labelPtr){
			if(callBackList[i].Task != NULL) {
			 	SetTask(callBackList[i].Task,errorCode,callBackList[i].arg_p);
			}
			labelPointer[i] = NULL;
	    }	
	}
}

u08 changeCallBackLabel(void* oldLabel, void* newLabel){
	bool_t flag_isr = FALSE;
	if(INTERRUPT_STATUS) {
		flag_isr = TRUE;
		INTERRUPT_DISABLE;
	}
	while(1) {
		u08 i = findCallBack(oldLabel);
		if(i == CALL_BACK_TASK_LIST_LEN) break;
		labelPointer[i] = newLabel;
	}
	if(flag_isr) INTERRUPT_ENABLE;
	return EVERYTHING_IS_OK;
}

#endif // CALL_BACK_TASK
#ifdef __cplusplus
}
#endif
