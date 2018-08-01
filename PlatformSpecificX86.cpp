extern "C" {
#include "PlatformSpecific.h"
}
#ifdef _X86

#include <thread>
#include <chrono>
#include <mutex>
static std::thread* timerThread;
static std::mutex mt;

#ifdef __cplusplus
extern "C" {
#endif
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
    	auto now = std::chrono::steady_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds((1000/TICK_PER_SECOND) - 1));
        blockIt();
        TimerISR();
        unBlockIt();
        auto later = std::chrono::steady_clock::now();
        auto diff = later - now;
        auto d = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
        if(d > 75) {
        	writeLogWhithStr("ERROR too long time is interrupt ", d);
        }
    }
}
using sec32 = std::chrono::duration<Time_t,std::ratio<1,1>>;

void _init_Timer(void){// Инициализация таймера 0, настройка прерываний каждую 1 мс, установки начальных значений для массива таймеров
	auto now = std::chrono::system_clock::now(); // time_point with start time 1970
	auto time = now.time_since_epoch();  // duration
	auto oldFormatTime = std::chrono::system_clock::to_time_t(now);  // get C time int64_t
	writeLogU32((u32)(oldFormatTime));
	auto timeSeconds32 = std::chrono::duration_cast<sec32>(time).count();  // get time in sec32 aka Time_t aka uint32_t
	setSeconds(timeSeconds32);
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
#endif
