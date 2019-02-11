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
 * 1.4.2    - Add large memory manager
 * 1.4.3    - Small changes in datastruct manager
 * 1.4.4    - Small add volatile qualificators in all global data
 * */
const char* const _osVersion = "V1.4.4";
const BaseSize_t _MAX_BASE_SIZE = (1LL<<(sizeof(BaseSize_t)<<3))-1;

#ifdef _PWR_SAVE
volatile u32 minTimeOut = 1; // Минимальное время таймоута для задач из списка таймеров используется в таймерах и в циклических задачах
extern unsigned int _setTickTime(unsigned int timerTicks); // В качестве аргумента передается кол-во стандартных тиков таймера
//(Таймер начинает тикать значительно реже что значительно увеличивает энергоэффективность)
// Вернет занчение на которое реально смог изменить частоту прерываний
#endif

static void TaskManager(void);
#ifdef _PWR_SAVE
static u32 TimerService(void);
#else
static void TimerService(void);
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

#ifdef CYCLE_FUNC
extern void initCycleTask( void );
#ifdef _PWR_SAVE
extern u32 CycleService( void );
#else
extern void CycleService( void );
#endif
#endif

#ifdef EVENT_LOOP_TASKS
extern void EventManager( void );
extern void initEventList( void );
#endif

#ifdef CLOCK_SERVICE
extern volatile Time_t __systemSeconds;
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
	Time_t sec = getAllSeconds();
	time_res += sec*TICK_PER_SECOND;
#endif
	return time_res;
}


static void ClockService(void){
#ifdef _PWR_SAVE
	GlobalTick += minTimeOut;
#else
	GlobalTick++;
#endif
#ifdef CLOCK_SERVICE
	while(GlobalTick >= TICK_PER_SECOND) {
		__systemSeconds++;
		GlobalTick -= TICK_PER_SECOND;
	}
#endif
}

void SetIdleTask(const IdleTask_t Task){
	unlock_t unlock = lock(SetIdleTask);
	IdleTask = (IdleTask_t)Task;
	unlock(SetIdleTask);
}

static void Idle(void) { // Функция включает режим пониженного электропотребления микроконтроллера. При этом перестает работать ядро.
	if(IdleTask != NULL) IdleTask();
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

void runFemtOS( void ) {
	while(TRUE) {
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

#ifdef _PWR_SAVE
#ifndef NATIVE_TIMER_PWR_SAVE
unsigned int _setTickTime(unsigned int timerTicks) {
	return timerTicks;
}
#endif
#endif

void TimerISR(void) {
#ifdef _PWR_SAVE
#ifndef NATIVE_TIMER_PWR_SAVE
	static BaseSize_t isrCounter = 0;
	if(++isrCounter < minTimeOut) return;
	isrCounter = 0;
#endif
	ClockService();
	u32 minTimerService = TimerService();	// Пересчет всех системных таймеров из очереди, вернет минимальный таймер
#ifdef CYCLE_FUNC
	u32 minCycleService = CycleService(); // Вернет минимальное время из циклических задач
	if(minTimerService && minCycleService) {
		if(minTimerService < minCycleService) minTimeOut = minTimerService;
		else if(minCycleService != 0) minTimeOut = minCycleService;
	}
	else if(minTimerService) minTimeOut = minTimerService;
	else if(minCycleService) minTimeOut = minCycleService;
	else minTimeOut = 1;
	minTimeOut = _setTickTime(minTimeOut);
#else  //NOT CYCLE_FUNC
	if(minTimerService) minTimeOut = minTimerService;
	else minTimeOut = 1;
	minTimeOut = _setTickTime(minTimerService);
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
	unlock_t unlock = lock((void*)TaskList);
	if(countBegin != countEnd) { // Если очередь не пустая
	// Необходимо помнить про конвеерный способ выборки команд в микроконтроллере (if - как можно чаще должен быть истиной)
		TaskMng Func_point = TaskList[countBegin].Task; // countBegin - указывает на начало очереди на рабочую задачу
		BaseParam_t a = TaskList[countBegin].arg_p;
		BaseSize_t n = TaskList[countBegin].arg_n;
		countBegin = (countBegin < TASK_LIST_LEN-1)? countBegin+1:0;
		unlock((void*)TaskList);
		Func_point(n,a);
	} else {
		unlock((void*)TaskList); // Если очередь пустая включаем прерывания
		Idle();           // И выполняем функция простоя
	}
}

void SetTask(const TaskMng New_Task, const BaseSize_t n, const BaseParam_t data) {
	unlock_t unlock = lock((void*)TaskList);
	register u08 count = (countEnd < TASK_LIST_LEN-1)? countEnd+1:0; //Кольцевой буфер
	if(count != countBegin){ // Если после добавления задачи countEnd не догонит countBegin значит очередь не переполнена
	// Необходимо помнить про конвеерный способ выборки команд в микроконтроллере (if - как можно чаще должен быть истиной)
		TaskList[countEnd].Task = New_Task; // Если очередь не переполнится добавляем элемент в очередь
		TaskList[countEnd].arg_n = n;       // countEnd
		TaskList[countEnd].arg_p = data;
		countEnd = count;
		unlock((void*)TaskList);
		return;
	}// Здесь мы окажемся в редких случаях когда oчередь переполнена
    #ifndef USE_TIMER_IF_OVERFLOW_TASK_LIST
    MaximizeErrorHandler("ERROR: task queue overflow");
    unlock((void*)TaskList);
    #else
	SetTimerTask(New_Task, n, data, TIME_DELAY_IF_BUSY);  //Ставим задачу в очередь(попытаемся записать ее позже)
	unlock((void*)TaskList);
    #endif
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
	unlock_t unlock = lock(TaskList);
	register u08 count = (countBegin)? countBegin-1:TASK_LIST_LEN-1; // Определяем указатель начала очереди куда должны вставить новую задачку
	if(count != countEnd) {   // Если очередь еще не переполнена
	// Необходимо помнить про конвеерный способ выборки команд в микроконтроллере (if - как можно чаще должен быть истиной)
		countBegin = count;
		TaskList[countBegin].Task = New_Task;
		TaskList[countBegin].arg_n = n;
		TaskList[countBegin].arg_p = data;
		unlock(TaskList);
		return;
	}
	// Здесь мы окажемся если все таки очередь переполнена (мало вероятный случай)
	SetTimerTask(New_Task, n, data, TIME_DELAY_IF_BUSY);
	unlock(TaskList);
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
#ifdef _PWR_SAVE
static u32 TimerService (void) {
	unlock_t unlock = lock((void*)MainTime);
	u08 index = 0;
	u32 tempMinTime = 0;
	while(index < lastTimerIndex) {  // Перебираем всю очередь таймеров
		if(MainTime[index] > minTimeOut) {    // Если таймер еще не дотикал (наиболее вероятно)
			MainTime[index] -= minTimeOut;      // Тикаем им
			if( MainTime[index] < tempMinTime || !tempMinTime) tempMinTime = MainTime[index]; // Сохраняем новое значения минимального
			index++;                			// И переходим на следующую итерацию цикла
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
	unlock((void*)MainTime);
	return tempMinTime;
}
#else // Класический таймер. Без регулирования скорости работы таймер ОС
static void TimerService (void) {
	unlock_t unlock = lock((void*)MainTime);
	u08 index = 0;
	while(index < lastTimerIndex) {  // Перебираем всю очередь таймеров
		if(MainTime[index] > 1) {  // Если таймер еще не дотикал (наиболее вероятно)
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
	unlock((void*)MainTime);
}
#endif

void SetTimerTask(const TaskMng TPTR, const BaseSize_t n, const BaseParam_t data, const Time_t New_Time){
	if(New_Time == 0) {SetTask(TPTR, n, data); return;}
	unlock_t unlock = lock((void*)MainTime);
	if(lastTimerIndex < TIME_LINE_LEN){ // Если очередь не переполнена
		MainTimer[lastTimerIndex].Task = TPTR;
		MainTimer[lastTimerIndex].arg_n = n;
		MainTimer[lastTimerIndex].arg_p = data;
		MainTime[lastTimerIndex] = New_Time;
		lastTimerIndex++;
#ifdef _PWR_SAVE
		if(New_Time < minTimeOut) minTimeOut = _setTickTime(New_Time);
#endif
		unlock((void*)MainTime);
	} else {
		MaximizeErrorHandler("PANIC: HAVE NOT MORE TIMERS");
		unlock((void*)MainTime);
	}
}

static u08 findTimer(const TaskMng TPTR, const BaseSize_t n, const BaseParam_t data) {
	register u08 index = 0;
	for(;index<lastTimerIndex; index++)	{
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
	u08 index = findTimer(TPTR,n,data);
	if(index < lastTimerIndex) {
		unlock_t unlock = lock((void*)MainTime);
		MainTime[index] = New_Time;
		unlock((void*)MainTime);
		return TRUE;
	}
	return FALSE;
}

void delTimerTask(const TaskMng TPTR, const BaseSize_t n, const BaseParam_t data) {
	u08 index = findTimer(TPTR,n,data);
	if(index < lastTimerIndex){
		unlock_t unlock = lock((void*)MainTime);
		lastTimerIndex--;
		MainTimer[index].Task  = MainTimer[lastTimerIndex].Task;    // На место этого таймера перемещаем последний
		MainTimer[index].arg_n = MainTimer[lastTimerIndex].arg_n;
		MainTimer[index].arg_p = MainTimer[lastTimerIndex].arg_p;
		MainTime[index] = MainTime[lastTimerIndex];
		unlock((void*)MainTime);
	}
}

void delAllTimerTask(){
	unlock_t unlock = lock((void*)MainTime);
    lastTimerIndex = 0;
    unlock((void*)MainTime);
}

u08 getFreePositionForTimerTask(void) {
	return TIME_LINE_LEN - lastTimerIndex;
}

//destination - адрес в памяти КУДА копируем source - адрес в памяти ОТКУДА копируем n - количество БАЙТ копируемых
void memCpy(void* destination, const void* source, const BaseSize_t num) {
#if ARCH == 64
		BaseSize_t blocks = num>>4;		// 8-мь байт копируются за один раз
		u08 last = num & 0x0F; // остаток
		for(BaseSize_t i = 0; i<blocks; i++) {
			*((u64*)destination) = *((u64*)source);
			destination = (void*)((byte_ptr)destination + 8);
			source = (void*)((byte_ptr)source + 8);
		}
		for(u08 i = 0; i<last; i++) {
			*((byte_ptr)destination) = *((byte_ptr)source);
			(byte_ptr)destination++; (byte_ptr)source++;
		}
#elif ARCH == 32
	BaseSize_t blocks = num>>2;		// 4-ре байта копируются за один раз
	u08 last = num & 0x03; // остаток
	for(BaseSize_t i = 0; i<blocks; i++) {
		*((u32*)destination) = *((u32*)source);
		destination = (void*)((byte_ptr)destination + 4);
		source = (void*)((byte_ptr)source + 4);
	}
	for(u08 i = 0; i<last; i++) {
		*((byte_ptr)destination) = *((byte_ptr)source);
		destination = (void*)((byte_ptr)destination+1); source = (void*)((byte_ptr)source+1);
	}
#elif ARCH == 16
	BaseSize_t blocks = num>>1;		// 2 байта копируются за один раз
	BaseSize_t last = num & 0x01; // остаток
	for(BaseSize_t i = 0; i<blocks; i++) {
		*((u16*)destination) = *((u16*)source);
		destination = (void*)((byte_ptr)destination + 2);
		source = (void*)((byte_ptr)source + 2);
	}
	if(last) *((byte_ptr)destination) = *((byte_ptr)source);
#else
	for (BaseSize_t i=0; i<num; i++){ //Копирование будет побайтное
		*((byte_ptr)destination + i) = *((byte_ptr)source + i); // Выполняем копирование данных
	}
#endif
}

void memSet(void* destination, const BaseSize_t size, const u08 value) {
#if ARCH == 64
	BaseSize_t blocks = size>>4; // 4-ре байта копируются за один раз
	u08 last = size & 0x0F;      // остаток
	if(blocks > 0) {
		u64 val = (u64)value<<56 | (u64)value<<48 | (u64)value<<40 | (u64)value<<32 |
				  (u32)value<<24 | (u32)value<<16 | (u16)value<<8 | value;
		for(BaseSize_t i = 0; i<blocks; i++) {
			*((u64*)destination) = val;
			destination = (void*)((byte_ptr)destination + 8);
		}
	}
	for(BaseSize_t i = 0; i<last; i++) {
		*((byte_ptr)destination) = value;
		(byte_ptr)destination++;
	}
#elif ARCH == 32
	BaseSize_t blocks = size>>2; // 4-ре байта копируются за один раз
	u08 last = size & 0x03;      // остаток
	if(blocks > 0) {
		u32 val = (u32)value<<24 | (u32)value<<16 | (u16)value<<8 | value;
		for(BaseSize_t i = 0; i<blocks; i++) {
			*((u32*)destination) = val;
			destination = (void*)((byte_ptr)destination + 4);
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
#ifndef _IAR_
		(byte_ptr)destination+=2;
#else
		destination = (void*)((byte_ptr)destination + 2);
#endif
	}
	if(last) *((byte_ptr)destination) = value;
#else
	for (BaseSize_t i = 0; i < size; i++){
		*((byte_ptr)destination + i) = value;
	}
#endif // ARCH
}

void shiftLeftArray(BaseParam_t source, BaseSize_t sourceSize, BaseSize_t shiftSize) {
	BaseSize_t i = 0, j = shiftSize;
	byte_ptr src = (byte_ptr)source;
	while(j < sourceSize) {
		src[i++] = src[j++];
	}
}

#ifdef __cplusplus
}
#endif
