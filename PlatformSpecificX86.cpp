#ifdef _MSVC_LANG
#include "../stdafx.h"
#endif
#ifdef _WIN32
#include <thread>
#include <mutex>
#elif __unix
#include <thread>
#include <mutex>
#endif
#include <chrono>
static std::thread* timerThread;

#ifdef __cplusplus
extern "C" {
#endif
#include "PlatformSpecific.h"
#include "TaskMngr.h"
#include "logging.h"
#ifdef __cplusplus
}
#endif

extern void TimerISR();

#ifdef MAXIMIZE_OVERFLOW_ERROR
void MaximizeErrorHandler(string_t str){
	initWatchDog();
	writeLogStr((string_t)"ERROR handler");
	writeLogStr(str);
	exit(2);
}
#else
void MaximizeErrorHandler(string_t str){
	writeLogStr((string_t)"ERROR handler");
	writeLogStr(str);
}
#endif
/********************************************************************************************************************
 *********************************************************************************************************************
                                            ПЛАТФОРМО-ЗАВИСИМЫЕ ФУНКЦИИ														|
 *********************************************************************************************************************
 *********************************************************************************************************************/

void initWatchDog() {
	writeLogStr(string_t("start init watch dog"));
}

void resetWatchDog() {

}

static std::recursive_mutex mtx;  // Рекурсивный мьютекс для ограничения доступа ко всем ресурсам сразу.
// Не эффективный локер, но надежный 100%
static void unLCK(const void *const resourceId) {mtx.unlock();}
static unlock_t lock1(const void* const resourceId) {
	mtx.lock();
	return unLCK;
}


#define RESOURCE_LIST 25

#if RESOURCE_LIST > 0xFE
#error "Resource size list must be less"
#endif

struct {
	std::mutex mt;  // Мьютекс защищающий определенный ресурс
	void* resourceId;	// Уникальный идентификатор ресурса
}resourceMutexList[RESOURCE_LIST]; // Очередь на ресурсы
static std::mutex mt; // Мьютекс защищающий очередь
static void unlock(const void*const resourceId) {
	mt.lock();
	for(u08 i=0; i<RESOURCE_LIST; i++) {
		if(resourceMutexList[i].resourceId == resourceId) {
			mt.unlock();
			resourceMutexList[i].mt.unlock();
			return;
		}
	}
	mt.unlock();
}
static void empty(const void* const resourceId) {}

static unlock_t lock3(const void*const resourceId) {
	return empty;
}

static unlock_t lock2(const void*const resourceId) {
	s16 saveIndex = -1;
	mt.lock();
	for(u08 i=0; i<RESOURCE_LIST; i++) {
		if(resourceMutexList[i].resourceId == resourceId) {
			if(resourceMutexList[i].mt.try_lock()) mt.unlock();
			else {
				mt.unlock();
				resourceMutexList[i].mt.lock();
			}
			return unlock;
		}
		if(resourceMutexList[i].resourceId == NULL) saveIndex = i;
	}
	if(saveIndex > 0) { // Еще ни разу не залоченный ресурс
		resourceMutexList[saveIndex].resourceId = (void*)resourceId;
		resourceMutexList[saveIndex].mt.lock();
		mt.unlock();
		return unlock;
	}
	mt.unlock();
	writeLogStr((string_t)"WARN, Never be here list empty error");
	return empty;
}


unlock_t lock(const void*const resourceId) {
	//Здесь можно сделать выбор какой локер использовать
	// lock1 - неэффективный по скорости, но надежный и простой на все ресурсы ОДИН примитив синхронизации
	// lock2 - эффективный по скорости (для каждого ресурса свой мьютекс) Но сложнее, занимает больше места
	// lock3 - пустішка для проверки скорости
	return lock2(resourceId);
}


static void __timer() {
	const std::chrono::nanoseconds timeBase =  std::chrono::nanoseconds(1000000000ULL/TICK_PER_SECOND);
	std::chrono::nanoseconds dT = std::chrono::nanoseconds(0);
	while(1) {
		auto tStart = std::chrono::steady_clock::now();
		TimerISR();
		dT += (std::chrono::steady_clock::now() - tStart);
		if(dT < timeBase) {
			std::this_thread::sleep_for( (timeBase-dT) );
			dT = std::chrono::nanoseconds(0);
		}
		else {
			writeSymb('?');
			dT -= timeBase;
		}
	}
}

void _init_Timer(void) {// Инициализация таймера 0, настройка прерываний каждую 1 мс, установки начальных значений для массива таймеров
	writeLogStr((string_t)"start init timer");
	mt.lock();
	for(u08 i=0; i<RESOURCE_LIST; i++) {
		resourceMutexList[i].resourceId = NULL;
	}
	mt.unlock();
	timerThread = new std::thread(__timer);
}

/*
 * Для программного UART
 * Все програмные UART задействует прерывания одного таймера
 * Все програмные UART должны находится на одном порту ввода вывода, который указывается здесь же
 * Если программных UART будет больше двух необходимо добавлять новые функции в ProgramUART.c
 */
void _initTimerSoftUart() {

}

void initProgramUartGPIO(unsigned short TX_MASK, unsigned short RX_MASK) {

}