/*
MIT License

Copyright (c) 2017 Oleksiy Khanin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 * */
#include "TaskMngr.h"
#include "PlatformSpecific.h"

#ifdef __cplusplus
extern "C" {
#endif
/*
Создание простого диспетчера задач. Пример построения программы
 */

/*
 * V1.0     - Add versioning
 * V1.0.1   - Add PAIR data struct
 * V1.0.2   - Add signals and slot
 * V1.1.0   - Add power save mode for OS
 * V1.3.1   - Fix datastruct module
 * V1.3.2   - Fix FOREACH algoritm for datastructs
 * V1.3.3   - MyString module has Sprintf
 * V1.3.4.2 - MaximizeErrorHandler now receive a string
 * V1.3.4.5 - Fix some bugs
 * V1.4.0.0 - Add lock and unlock function instead INTERRUPT_ENABLE, INTERRUPT_DISABLE (for x86 perfomance upgrade)
 * V1.4.1.0 - Add macros ENABLE_LOGGING if logging not need
 * V1.4.2    - Add large memory manager
 * V1.4.3    - Small changes in datastruct manager
 * V1.4.3.1  - Small add volatile qualificators in all global data
 * V1.4.4    - Fix power save bugs
 * V1.4.4.1  - Technical version (fix bug in MSP430 powersave mode with NATIVE_TIMER_PWR_SAVE)
 * V1.4.4.2  - Techinical commit
 * V1.4.4.3  - Small fixes timer interrupt (add clean interrupt flag instrution) + small optimiztion with strings
 * V1.4.4.4  - Fix Sprintf in MyString for print float
 * V1.4.5.0  - Add loadAverage in OS (not tested yet)
 * V1.4.5.1  - Add compiler specific attributes
 * V1.4.5.2  - Change load avarage coefficient
 * V1.4.5.3  - Delete double blocking timer queue in power save mode + fix bug in PlatformSpecificMSP in power save mode
 * V1.4.5.4  - Add Readme.md, Now compiled in Visual Studio
 * V1.4.5.5  - Add defragmentation function when allocMem fail and try again
 * V1.4.6    - Add SHA256 (not tested)
 * V1.4.61   - Tested SHA256 and BASE64
 * V1.4.62	 - Add delete callback by task (Don't test yet)
 * V1.4.63   - Add compare function for compare to array data (Don't test yet)
 * V1.4.64   - compare to arrays function is work fine
 * V1.4.7	 - Add new linked array datatype. Dont't full tested yet
 * V1.4.71   - fix some bugs (lock heap in defragmentation)
 * V1.5.0    - Add command list module
 * V1.5.1    - Add json don't tested yet
 * V1.5.2    - Add const qualifier in local variables
 * V1.5.3    - Add enableLogging and disableLogging= standart command
 * V1.5.4    - Rewrite command line tool
 * V1.5.5    - Add function to get free size of callbacks, signals and cycle tasks
 * V1.5.6    - Small fix in the string library (findStr didn't work properly in some cases)
 * */
const char* const _osVersion = "V1.5.6";
const BaseSize_t _MAX_BASE_SIZE = (1LL<<(sizeof(BaseSize_t)<<3))-1;

static void TaskManager(void);
#ifdef _PWR_SAVE
volatile u32 _minTimeOut = 1; // Минимальное время таймоута для задач из списка таймеров используется в таймерах и в циклических задачах
static u32 TimerService(void);
#else
static void TimerService(void);
#endif

#ifdef CYCLE_FUNC
extern void initCycleTask( void );
#ifdef _PWR_SAVE
extern u32 CycleService( void );
#else
extern void CycleService( void );
#endif
#endif

#ifdef ALLOC_MEM
extern void initHeap(void);
#endif
#ifdef ALLOC_MEM_LARGE
extern void initHeap(void);
#endif

#ifdef DATA_STRUCT_MANAGER
extern void initDataStruct( void );
#endif

#ifdef EVENT_LOOP_TASKS
extern void EventManager( void );
extern void initEventList( void );
#endif

#ifdef _DYNAMIC_ARRAY
void initDynamicArray();
#endif

#ifdef  COMMAND_TASK
void initCommandList();
#endif

#ifdef CLOCK_SERVICE
extern volatile Time_t __systemSeconds;
#endif

#ifdef LOAD_STATISTIC
static u32 idleTicks = 0;					 // Кол-во времени в Idle процессе
#define STATISTIC_KOEF 100UL
u32 getLoadAvarage() {
	u32 workTicks = getTick();
	u32 t = idleTicks;
	while(t != idleTicks) t=idleTicks;
	u32 onTaskTicks = workTicks - t;
	if(workTicks > t) return (u32)((u64)(onTaskTicks*STATISTIC_KOEF*STATISTIC_KOEF)/workTicks);
	return 0; // Произошло переполнение
}
#endif

static void ClockService( void );

#ifdef CALL_BACK_TASK
extern void initCallBackTask(void);
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


static volatile IdleTask_t IdleTask=NULL;
static volatile TaskList_t TaskList[TASK_LIST_LEN];   // Очередь задач - это глобальный масив переменных. Каждый элемент которой состоит из трех переменных.

// Очередь системных таймеров
//В очередь записывается задача (указатель на функцию) и выдержка времени необходимая перед постановкой задачи в очередь
static volatile Time_t MainTime[TIME_LINE_LEN];// Выдержка времени для конкретной задачи в мс.
static volatile TaskList_t MainTimer[TIME_LINE_LEN]; // Указатель задачи, которая состоит из указателя на функцию задачи, и двух аргументов

volatile static Time_t GlobalTick;
u32 getTick(void) {
	u32 time_res = 0;
	while(time_res != GlobalTick) time_res = (u32)GlobalTick;
#ifdef CLOCK_SERVICE
	const Time_t sec = getAllSeconds();
	time_res += sec*TICK_PER_SECOND;
#endif
	return time_res;
}

static void ClockService(void){
	const unlock_t unlock = lock((const void* const)(&GlobalTick));
#ifdef _PWR_SAVE
	GlobalTick += _minTimeOut;
#else
	GlobalTick++;
#endif
#ifdef LOAD_STATISTIC
	if(GlobalTick < idleTicks) idleTicks = 0; // Если глобальный счетчик тиков ОС переполнился чистим и счетчик времени в idle процессе
#endif
#ifdef CLOCK_SERVICE
	while(GlobalTick >= TICK_PER_SECOND) {
		GlobalTick -= TICK_PER_SECOND;
		__systemSeconds++;
	}
#endif
	unlock((const void* const)(&GlobalTick));
}

void SetIdleTask(const IdleTask_t Task){
	const unlock_t unlock = lock(SetIdleTask);
	IdleTask = (IdleTask_t)Task;
	unlock(SetIdleTask);
}

static void Idle(void) { // Функция включает режим пониженного электропотребления микроконтроллера. При этом перестает работать ядро.
#ifdef LOAD_STATISTIC
	 const u32 startTick = getTick();
#endif
	if(IdleTask != NULL) IdleTask();
#ifdef LOAD_STATISTIC
	const u32 stopTick = getTick();
	if(stopTick > startTick) {
		const unlock_t unlock = lock(&idleTicks);
		idleTicks += stopTick-startTick;
		unlock(&idleTicks);
	}
#endif
}

/********************************************************************************************************************
 *********************************************************************************************************************
|					МЕНЕДЖЕР ЗАДАЧ	 														|
 *********************************************************************************************************************
 *********************************************************************************************************************
---------------------------------------------------------------------------------------------------------------------*/
void initFemtOS (void) {  // Инициализация менеджера задач
	register u08 i;
	// Отметим, что имя функции является ее указателем.
	// Вызвать любую функцию можно двояко:
	//  1. Стандартным способом через ее имя и список параметров Например, shov1();
	//  2. Через указатель на функцию. К примеру, (*show1)() - операция разыменовывания указателя на функцию;
	//INTERRUPT_DISABLE;
	for(i=0;i<TASK_LIST_LEN;i++) { //Набираем очередь задач // Это масив указателей на функции
			TaskList[i].Task = NULL;
	}
	for(i=0; i<TIME_LINE_LEN;i++) {
		MainTimer[i].Task = NULL; // Вся очередь таймеров состоит из пустышек
	}
	_init_Timer();
	IdleTask = NULL;
	GlobalTick = 0;
#ifdef ALLOC_MEM
	initHeap();
#endif
#ifdef ALLOC_MEM_LARGE
	initHeap();
#endif
#ifdef DATA_STRUCT_MANAGER
	initDataStruct();
#endif
#ifdef _DYNAMIC_ARRAY
	initDynamicArray();
#endif
#ifdef CYCLE_FUNC
	initCycleTask();
#endif
#ifdef EVENT_LOOP_TASKS
	initEventList();
#endif
#ifdef CALL_BACK_TASK
	initCallBackTask();
#endif
#ifdef  COMMAND_TASK
    initCommandList();
#endif
}

CC_NO_RETURN void runFemtOS( void ) {
	while(TRUE) {
#ifdef EVENT_LOOP_TASKS
		EventManager();
#endif
		TaskManager();
	}
}

CC_NO_RETURN void ResetFemtOS(void){
	initWatchDog();
	while(1);
}

#ifdef _PWR_SAVE
extern u32 _setTickTime(u32 timerTicks); // В качестве аргумента передается кол-во стандартных тиков таймера
//(Таймер начинает тикать значительно реже что значительно увеличивает энергоэффективность)
// Вернет занчение на которое реально смог изменить частоту прерываний
u32 _getTickTime(); // Сколько времени прошло с момента начала отсета до сейчас
// Необходимо для осущществление коррекции в случае если внезапно появился таймер меньше ранее установленного времени
#ifndef NATIVE_TIMER_PWR_SAVE
static u32 isrCounter = 0;
u32 _setTickTime(u32 timerTicks) {
	return timerTicks;
}
u32 _getTickTime(){ // Сколько времени осталось с момента начала отсета до сейчас в стандартных тиках ОС
    return isrCounter; // Вернем счетчик прерываний таймера
}
#endif
#endif

void TimerISR(void) {
#ifdef _PWR_SAVE
#ifndef NATIVE_TIMER_PWR_SAVE
	if(++isrCounter < _minTimeOut) return;
	isrCounter = 0;
#endif
	ClockService();
	const u32 minTimerService = TimerService();	// Пересчет всех системных таймеров из очереди, вернет минимальный таймер
#ifdef CYCLE_FUNC
	const u32 minCycleService = CycleService(); // Вернет минимальное время из циклических задач
	if(minTimerService && minCycleService) {
		if(minTimerService < minCycleService) _minTimeOut = minTimerService;
		else _minTimeOut = minCycleService;
	}
	else if(minTimerService) _minTimeOut = minTimerService;
	else if(minCycleService) _minTimeOut = minCycleService;
	else _minTimeOut = 1;
	_minTimeOut = _setTickTime(_minTimeOut);
#else  //NOT CYCLE_FUNC
	if(minTimerService) _minTimeOut = minTimerService;
	else _minTimeOut = 1;
	_minTimeOut = _setTickTime(minTimerService);
#endif
#else //NOT _PWR_SAVE
	ClockService();
	TimerService();	// Пересчет всех системных таймеров из очереди
#ifdef CYCLE_FUNC
	CycleService();
#endif
#endif
} 	//Отработка прерывания по таймеру

static volatile u08 countBegin = 0;    // Указатель на НАЧАЛО очереди (нужен для быстрого диспетчера)
static volatile u08 countEnd = 0;      // Указатель на КОНЕЦ очереди (нужен для быстрого диспетчера)

// Функция Планировщика (Менеджера) задач. Она запускает ту функцию, которая должна сейчас выполнятся.
/*	Берется первая функция из очереди задач (TaskLine[0]) и проверяется не пустая ли она. Если не пустая, то смещаем всю чередь
на один элемент вверх, а в последний элемент очереди ставим Idle (пустую функцию, включающую режим ожидания МК).
Берем количество параметров из глобального стека и передаем взятой функции, которая берет свои параметры из глобального стека.
 */
static void TaskManager(void) {
	const unlock_t unlock = lock((const void* const)TaskList);
	if(countBegin != countEnd) { // Если очередь не пустая
	// Необходимо помнить про конвеерный способ выборки команд в микроконтроллере (if - как можно чаще должен быть истиной)
		TaskMng Func_point = TaskList[countBegin].Task; // countBegin - указывает на начало очереди на рабочую задачу
		BaseParam_t a = TaskList[countBegin].arg_p;
		BaseSize_t n = TaskList[countBegin].arg_n;
		countBegin = (countBegin < TASK_LIST_LEN-1)? countBegin+1:0;
		unlock((void*)TaskList);
		Func_point(n,a);
	} else {
		unlock((const void* const)TaskList);
		Idle();
	}
}

void SetTask(const TaskMng New_Task, const BaseSize_t n, const BaseParam_t data) {
	const unlock_t unlock = lock((void*)TaskList);
	const u08 count = (countEnd < TASK_LIST_LEN-1)? countEnd+1:0; //Кольцевой буфер
	if(count != countBegin){ // Если после добавления задачи countEnd не догонит countBegin значит очередь не переполнена
	// Необходимо помнить про конвеерный способ выборки команд в микроконтроллере (if - как можно чаще должен быть истиной)
		TaskList[countEnd].Task = New_Task; // Если очередь не переполнится добавляем элемент в очередь
		TaskList[countEnd].arg_n = n;       // countEnd
		TaskList[countEnd].arg_p = data;
		countEnd = count;
		unlock((void*)TaskList);
		return;
	}// Здесь мы окажемся в редких случаях когда oчередь переполнена
    MaximizeErrorHandler("ERROR: task queue overflow");
    unlock((void*)TaskList);
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
void SetFrontTask (const TaskMng New_Task, const BaseSize_t n, const BaseParam_t data){ // Функция помещает в НАЧАЛО очереди задачу New_Task
	const unlock_t unlock = lock((const void* const)TaskList);
	const u08 count = (countBegin)? countBegin-1:TASK_LIST_LEN-1; // Определяем указатель начала очереди куда должны вставить новую задачку
	if(count != countEnd) {   // Если очередь еще не переполнена
	// Необходимо помнить про конвеерный способ выборки команд в микроконтроллере (if - как можно чаще должен быть истиной)
		countBegin = count;
		TaskList[countBegin].Task = New_Task;
		TaskList[countBegin].arg_n = n;
		TaskList[countBegin].arg_p = data;
		unlock((const void* const)TaskList);
		return;
	}
	// Здесь мы окажемся если все таки очередь переполнена (мало вероятный случай)
	SetTimerTask(New_Task, n, data, TIME_DELAY_IF_BUSY);
	unlock((const void* const)TaskList);
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
static u08 _lastTimerIndex = 0; // Указывает на индекс следующего СВОБОДНОГО таймера
#ifdef _PWR_SAVE
static u32 TimerService (void) {
	const unlock_t unlock = lock((void*)MainTime);
	u08 index = 0;
	u32 tempMinTime = 0;
	while(index < _lastTimerIndex) {  // Перебираем всю очередь таймеров
		if(MainTime[index] > _minTimeOut) {    // Если таймер еще не дотикал (наиболее вероятно)
			MainTime[index] -= _minTimeOut;      // Тикаем им
			if( MainTime[index] < tempMinTime || !tempMinTime) tempMinTime = MainTime[index]; // Сохраняем новое значения минимального
			index++;                			// И переходим на следующую итерацию цикла
			continue;
		}
		SetTask (MainTimer[index].Task,  // Ставим нашу задачу в конец очередь
				MainTimer[index].arg_n,
				MainTimer[index].arg_p);
		_lastTimerIndex--;
		MainTimer[index].Task  = MainTimer[_lastTimerIndex].Task;    // На место этого таймера перемещаем последний
		MainTimer[index].arg_n = MainTimer[_lastTimerIndex].arg_n;
		MainTimer[index].arg_p = MainTimer[_lastTimerIndex].arg_p;
		MainTime[index] = MainTime[_lastTimerIndex];
	}
	unlock((void*)MainTime);
	return tempMinTime;
}
#else // Класический таймер. Без регулирования скорости работы таймер ОС
static void TimerService (void) {
	const unlock_t unlock = lock((const void*const)MainTime);
	u08 index = 0;
	while(index < _lastTimerIndex) {  // Перебираем всю очередь таймеров
		if(MainTime[index] > 1) {  // Если таймер еще не дотикал (наиболее вероятно)
			MainTime[index]--;      // Тикаем им
			index++;                // И переходим на следующую итерацию цикла
			continue;
		}
		SetTask (MainTimer[index].Task,  // Ставим нашу задачу в конец очередь
				MainTimer[index].arg_n,
				MainTimer[index].arg_p);

		_lastTimerIndex--;
		MainTimer[index].Task  = MainTimer[_lastTimerIndex].Task;    // На место этого таймера перемещаем последний
		MainTimer[index].arg_n = MainTimer[_lastTimerIndex].arg_n;
		MainTimer[index].arg_p = MainTimer[_lastTimerIndex].arg_p;
		MainTime[index] = MainTime[_lastTimerIndex];
	}
	unlock((void*)MainTime);
}
#endif

void SetTimerTask(const TaskMng TPTR, const BaseSize_t n, const BaseParam_t data, const Time_t New_Time){
	if(New_Time == 0) {SetTask(TPTR, n, data); return;}
	if(_lastTimerIndex < TIME_LINE_LEN){ // Если очередь не переполнена
#ifdef _PWR_SAVE
	    /*Вначале производим апдейт всех уже существующих таймеров*/
        if(New_Time < _minTimeOut) { // Если новое время меньше установленного сейчас
            _minTimeOut = _getTickTime(); // Получаем сколько времени уже успело дотикать
            TimerISR(); // Вызываем апдейт всех часов всех задач
            if(New_Time < _minTimeOut) { // Если новое время все еще меньше
                _minTimeOut = _setTickTime(New_Time); // Обновляем время
            }
        }
#endif
        const unlock_t unlock = lock((void*)MainTime);
		MainTimer[_lastTimerIndex].Task = TPTR;
		MainTimer[_lastTimerIndex].arg_n = n;
		MainTimer[_lastTimerIndex].arg_p = data;
		MainTime[_lastTimerIndex] = New_Time;
		_lastTimerIndex++;
		unlock((void*)MainTime);
	} else {
		MaximizeErrorHandler("PANIC: HAVE NOT MORE TIMERS");
	}
}

static u08 findTimer(const TaskMng TPTR, const BaseSize_t n, const BaseParam_t data) {
	register u08 index = 0;
	for(;index<_lastTimerIndex; index++)	{
		if((MainTimer[index].Task  == TPTR)&& /* Если уже есть запись с таким же адресом*/
		   (MainTimer[index].arg_p == data)&&
		   (MainTimer[index].arg_n == n))     /* и с таким же списком параметров*/
		{
			break;
		}
	}
	return index;
}

bool_t updateTimer(const TaskMng TPTR, const BaseSize_t n, const BaseParam_t data, const Time_t New_Time) {
	const u08 index = findTimer(TPTR,n,data);
	if(index < _lastTimerIndex) {
		unlock_t unlock = lock((void*)MainTime);
		MainTime[index] = New_Time;
		unlock((void*)MainTime);
		return TRUE;
	}
	return FALSE;
}

void delTimerTask(const TaskMng TPTR, const BaseSize_t n, const BaseParam_t data) {
	const u08 index = findTimer(TPTR,n,data);
	if(index < _lastTimerIndex){
		unlock_t unlock = lock((void*)MainTime);
		_lastTimerIndex--;
		MainTimer[index].Task  = MainTimer[_lastTimerIndex].Task;    // На место этого таймера перемещаем последний
		MainTimer[index].arg_n = MainTimer[_lastTimerIndex].arg_n;
		MainTimer[index].arg_p = MainTimer[_lastTimerIndex].arg_p;
		MainTime[index] = MainTime[_lastTimerIndex];
		unlock((void*)MainTime);
	}
}

void delAllTimerTask(void){
	unlock_t unlock = lock((void*)MainTime);
    _lastTimerIndex = 0;
    unlock((void*)MainTime);
}

u08 getFreePositionForTimerTask(void) {
	return TIME_LINE_LEN - _lastTimerIndex;
}

#ifndef STANDART_MEMCPY_MEMSET
//destination - адрес в памяти КУДА копируем source - адрес в памяти ОТКУДА копируем n - количество БАЙТ копируемых
void memCpy(void* destination, const void* source, const BaseSize_t num) {
#if ARCH == 64
		const BaseSize_t blocks = num>>3;		// 8-мь байт копируются за один раз
		const BaseSize_t last = num & 0x07; // остаток
		for(BaseSize_t i = 0; i<blocks; i++) {
			*((u64*)destination) = *((u64*)source);
			destination = (void*)((byte_ptr)destination + 8);
			source = (void*)((byte_ptr)source + 8);
		}
#elif ARCH == 32
	const BaseSize_t blocks = num>>2;		// 4-ре байта копируются за один раз
	const BaseSize_t last = num & 0x03; // остаток
	for(BaseSize_t i = 0; i<blocks; i++) {
		*((u32*)destination) = *((u32*)source);
		destination = (void*)((byte_ptr)destination + 4);
		source = (void*)((byte_ptr)source + 4);
	}
#else
	BaseSize_t last = num;
#endif
	for (BaseSize_t i=0; i<last; i++){ //Копирование будет побайтное
		*((byte_ptr)destination + i) = *((byte_ptr)source + i); // Выполняем копирование данных
	}
}

void memSet(void* destination, const BaseSize_t size, const u08 value) {
#if ARCH == 64
	const BaseSize_t blocks = size>>3; // 8 байт копируются за один раз
	const BaseSize_t last = size & 0x07;      // остаток
	const u64 val = (u64)value<<56 | (u64)value<<48 | (u64)value<<40 | (u64)value<<32 |
			  (u32)value<<24 | (u32)value<<16 | (u16)value<<8 | value;
	for(BaseSize_t i = 0; i<blocks; i++) {
		*((u64*)destination) = val;
		destination = (void*)((byte_ptr)destination + 8);
	}
#elif ARCH == 32
	const BaseSize_t blocks = size>>2; // 4-ре байта копируются за один раз
	const BaseSize_t last = size & 0x03;      // остаток
	const u32 val = (u32)value<<24 | (u32)value<<16 | (u16)value<<8 | value;
	for(BaseSize_t i = 0; i<blocks; i++) {
		*((u32*)destination) = val;
		destination = (void*)((byte_ptr)destination + 4);
	}
#else
	BaseSize_t last = size;
#endif // ARCH
	for(BaseSize_t i = 0; i<last; i++) {
		*((byte_ptr)destination) = value;
		(byte_ptr)destination++;
	}
}
#else
#include <string.h>
void memSet(void* destination, const BaseSize_t size, const u08 value) {
    memset(destination, value, size);
}

void memCpy(void* destination, const void* source, const BaseSize_t num) {
    memcpy(destination,source,num);
}
#endif

bool_t compare(const void* block1, const void* block2, const BaseSize_t size) {
#if ARCH == 64
	const BaseSize_t blocks = size>>3; // 8 байт копируются за один раз
	const BaseSize_t last = size & 0x07;      // остаток
	for(BaseSize_t i = 0; i<blocks; i++) {
		if(*((u64*)block1) == *((u64*)block2)) {
			block1 = (void*)((byte_ptr)block1 + 8);
			block2 = (void*)((byte_ptr)block2 + 8);
		}
		else return FALSE;
	}
#elif ARCH == 32
	const BaseSize_t blocks = size>>2; // 4-ре байта проверяются за один раз
	const BaseSize_t last = size & 0x03;      // остаток
	for(BaseSize_t i = 0; i<blocks; i++) {
		if(*((u32*)block1) == *((u32*)block2)) {
			block1 = (void*)((byte_ptr)block1 + 4);
			block2 = (void*)((byte_ptr)block2 + 4);
			continue;
		}
		else return FALSE;
	}
#else
	BaseSize_t last = size;
#endif
	for(BaseSize_t i = 0; i<last; i++) {
		if(*((byte_ptr)block1 + i) == *((byte_ptr)block2 + i)) {
			continue;
		}
		else return FALSE;
	}
	return TRUE;
}

void shiftLeftArray(BaseParam_t source, BaseSize_t sourceSize, BaseSize_t shiftSize) {
	BaseSize_t i = 0, j = shiftSize;
	byte_ptr src = (byte_ptr)source;
	while(j < sourceSize) {
		src[i++] = src[j++];
	}
}

void swapByte(byte_ptr byte1, byte_ptr byte2) {
  const unsigned char temp = *byte1;
  *byte1 = *byte2;
  *byte2 = temp;
}


void swapInt(unsigned int* int1, unsigned int* int2) {
  const unsigned int temp = *int1;
  *int1 = *int2;
  *int2 = temp;
}

#ifdef __cplusplus
}
#endif
