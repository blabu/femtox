#include <thread>
#include <chrono>
#include <mutex>
static std::thread* timerThread;
static std::mutex mt;

#ifdef __cplusplus
extern "C" {
#endif
#include "PlatformSpecific.h"
#include "TaskMngr.h"
#include "logging.h"


extern void TimerISR();

#ifdef MAXIMIZE_OVERFLOW_ERROR
	void MaximizeErrorHandler(){
		initWatchDog();
		while(1);
	}
#else
	void MaximizeErrorHandler(){
	}
#endif
/********************************************************************************************************************
*********************************************************************************************************************
                                            ПЛАТФОРМО-ЗАВИСИМЫЕ ФУНКЦИИ														|
*********************************************************************************************************************
*********************************************************************************************************************/

void initWatchDog() {
    writeLogStr("start init watch dog");
    exit(1);
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

static void timer() {
    while(1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000/TICK_PER_SECOND));
        blockIt();
        TimerISR();
        unBlockIt();
    }
}

void _init_Timer(void){// Инициализация таймера 0, настройка прерываний каждую 1 мс, установки начальных значений для массива таймеров
    writeLogStr("start init timer");
    timerThread = new std::thread(timer);
}

/*
 * Для программного UART
 * Все програмные UART задействует прерывания одного таймера
 * Все програмные UART должны находится на одном порту ввода вывода, который указывается здесь же
 * Если программных UART будет больше двух необходимо добавлять новые функции в ProgramUART.c
*/
void _initTimerSoftUart() {

}

void initProgramUartGPIO(unsigned short RX_MASK, unsigned short TX_MASK) {

}
#ifdef __cplusplus
}
#endif