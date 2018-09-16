#ifndef PLATFORMSPECIFIC_X86
#define PLATFORMSPECIFIC_X86

#define ARCH 32 /*Архитектура процессора 8, 16, 32 байта (разрядность шины данных)*/

void initWatchDog();
void resetWatchDog();
/********************************************************************************************************************
*********************************************************************************************************************
                                            ПЛАТФОРМО-ЗАВИСИМЫЕ ФУНКЦИИ														|
*********************************************************************************************************************
*********************************************************************************************************************/

unsigned char statusIt();
void blockIt();
void unBlockIt();

#define INTERRUPT_ENABLE    unBlockIt()
#define INTERRUPT_DISABLE   blockIt()
#define INTERRUPT_STATUS    statusIt()
#define WATCH_DOG_ON  initWatchDog()/*Генерируем Reset*/
#define TICK_PER_SECOND 500 /*Колличество тиков в секунду*/

void _init_Timer(void);	// Инициализация таймера 0, настройка прерываний каждую 1 мс, установки начальных значений для массива таймеров

/*
 * Для программного UART
 * Все програмные UART задействует прерывания одного таймера
 * Все програмные UART должны находится на одном порту ввода вывода, который указывается здесь же
 * Если программных UART будет больше двух необходимо добавлять новые функции в ProgramUART.c
*/
void _initTimerSoftUart();
void initProgramUartGPIO(unsigned short RX_MASK, unsigned short TX_MASK);

#endif // PLATFORMSPECIFIC_X86
