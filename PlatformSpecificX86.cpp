#ifdef _WIN
#include <mingw.thread.h>
#include <mingw.mutex.h>
#elif __unix
#include <thread>
#include <mutex>
#endif
#include <chrono>
static std::thread* timerThread;
static std::mutex mt;

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
	writeLogStr("Error handler");
	writeLogStr(str);
	exit(1);
}
#else
void MaximizeErrorHandler(string_t str){
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

unsigned char statusIt(){
	bool res = mt.try_lock();
	if(res) {
		mt.unlock();
		return 1;
	}
	return 0;
}

void blockIt() {
	mt.lock();
}

void unBlockIt(){
	mt.unlock();
}

static void __timer() {
	while(1) {
		auto tStart = std::chrono::steady_clock::now();
		blockIt();
		TimerISR();
		unBlockIt();
		auto tStop = std::chrono::steady_clock::now();
		auto dT = (tStop - tStart);
		std::this_thread::sleep_for(std::chrono::nanoseconds(1000000000UL/TICK_PER_SECOND) - dT);
	}
}

void _init_Timer(void){// Инициализация таймера 0, настройка прерываний каждую 1 мс, установки начальных значений для массива таймеров
	writeLogStr("start init timer");
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

