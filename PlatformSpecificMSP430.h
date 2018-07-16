#ifndef PLATFORMSPECIFIC
#define PLATFORMSPECIFIC
#include "io430.h"

/********************************************************************************************************************
*********************************************************************************************************************
                                            ПЛАТФОРМО-ЗАВИСИМЫЕ ФУНКЦИИ														|
*********************************************************************************************************************
*********************************************************************************************************************/

#define ARCH 8/*Архитектура процессора 8, 16, 32 байта (разрядность шины данных)*/
#define _IAR_

void initWatchDog(void);
void resetWatchDog(void);

#define INTERRUPT_ENABLE  __enable_interrupt()
#define INTERRUPT_DISABLE __disable_interrupt()
#define INTERRUPT_STATUS  __get_interrupt_state()
#define WATCH_DOG_ON  WDTCTL = WDTPW /*Генерируем Reset*/
#define TICK_PER_SECOND 100 /*Колличество тиков в секунду*/

void _init_Timer(void);	// Инициализация таймера 0, настройка прерываний каждую 1 мс, установки начальных значений для массива таймеров
unsigned short _setTickTime(unsigned short timerTicks); // В качестве аргумента передается кол-во стандартных тиков таймера
//(Таймер начинает тикать значительно реже что значительно увеличивает энергоэффективность)
// Вернет занчение на которое реально смог изменить частоту прерываний

/*
 * Для программного UART
 * Все програмные UART задействует прерывания одного таймера
 * Все програмные UART должны находится на одном порту ввода вывода, который указывается здесь же
 * Если программных UART будет больше двух необходимо добавлять новые функции в ProgramUART.c
*/
void _initTimerSoftUart();
void _deInitTimerSoftUart();
void initProgramUartGPIO(unsigned short RX_MASK, unsigned short TX_MASK);
#define TX_PORT   P3OUT
#define RX_PORT   P3OUT
#define RX_PIN    P3IN
#define TX_DIR    P3DIR
#define RX_DIR    P3DIR

#define ENABLE_UART_TIMER_ISR  (TACCTL0 |= CCIE)
#define DISABLE_UART_TIMER_ISR (TACCTL0 &= ~CCIE)
#define START_TIMER  TACTL |= MC0
#define CLEAR_TIMER  TACTL |= TACLR

#define READ_RX_PIN(PORT,PIN_MASK)  
#define WRITE_TX_PIN(PORT,PIN_MASK) 
#define CLEAR_TX_PIN(PORT,PIN_MASK) 


#endif // PLATFORMSPECIFIC
