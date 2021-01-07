#include "platform.h"
#ifdef _X86
#ifdef _MSVC_LANG
#include "../stdafx.h"
#endif
#ifdef _WIN32
#include <thread>
#include <mutex>
#elif __unix
#include <thread>
#include <mutex>
#include <map>
#endif
#include <chrono>

static std::thread* timerThread;

extern "C" {
	#include "PlatformSpecific.h"
	#include "TaskMngr.h"
	#include "logging.h"
}


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

std::map<const void*const, std::mutex> resourceMutexList;

static std::mutex mt; // Мьютекс защищающий очередь

static void unlock(const void*const resourceId) {
	std::lock_guard<std::mutex> l(mt);
	try {
		resourceMutexList.at(resourceId).unlock();
	} catch(const std::out_of_range&) {
		writeLogStr((string_t)"ERROR: Undefined resource id message");
	}
}

static void empty(const void* const resourceId) {}

static unlock_t lock3(const void*const resourceId) {
	return empty;
}

static unlock_t lock2(const void*const resourceId) {
	std::lock_guard<std::mutex> l(mt);
	resourceMutexList[resourceId].lock();
	return unlock;
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
		writeSymb('*');
		auto tStart = std::chrono::steady_clock::now();
		TimerISR();
		dT += (std::chrono::steady_clock::now() - tStart);
		if(dT < timeBase) {
			tStart = std::chrono::steady_clock::now();
			std::this_thread::sleep_for( (timeBase-dT) );
			auto dT2 = std::chrono::steady_clock::now() - tStart;
			dT = dT2-(timeBase-dT);
		}
		else { // Произошел пропуск прерывания
			writeSymb('?');
			dT -= timeBase;
		}
	}
}

void _init_Timer(void) {// Инициализация таймера 0, настройка прерываний каждую 1 мс, установки начальных значений для массива таймеров
	writeLogStr((string_t)"Start init timer");
	timerThread = new std::thread(__timer);
}

/*
 * Для программного UART
 * Все програмные UART задействует прерывания одного таймера
 * Все програмные UART должны находится на одном порту ввода вывода, который указывается здесь же
 * Если программных UART будет больше двух необходимо добавлять новые функции в ProgramUART.c
 */
void _initTimerSoftUart() {}

void initProgramUartGPIO(unsigned short TX_MASK, unsigned short RX_MASK) {}
#endif
