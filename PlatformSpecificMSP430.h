#ifndef PLATFORMSPECIFIC
#define PLATFORMSPECIFIC
#include "../standardLibrary/msp430f2272.h"

/********************************************************************************************************************
*********************************************************************************************************************
                                            ПЛАТФОРМО-ЗАВИСИМЫЕ ФУНКЦИИ														|
*********************************************************************************************************************
*********************************************************************************************************************/

#define ARCH 16 /*Архитектура процессора 8, 16, 32 байта (разрядность шины данных)*/

void initWatchDog();
void resetWatchDog();

#define INTERRUPT_ENABLE  {asm("nop"); __asm__ __volatile__("eint");}
#define INTERRUPT_DISABLE {__asm__ __volatile__("dint nop"); asm("nop");}
#define INTERRUPT_STATUS  __get_interrupt_state()
#define WATCH_DOG_ON  WDTCTL = WDTPW /*Генерируем Reset*/
#define TICK_PER_SECOND 100 /*Колличество тиков в секунду*/

void _init_Timer(void);	// Инициализация таймера 0, настройка прерываний каждую 1 мс, установки начальных значений для массива таймеров


/*
 * Для программного UART
 * Все програмные UART задействует прерывания одного таймера
 * Все програмные UART должны находится на одном порту ввода вывода, который указывается здесь же
 * Если программных UART будет больше двух необходимо добавлять новые функции в ProgramUART.c
*/
void _initTimerSoftUart();
void initProgramUartGPIO(unsigned short RX_MASK, unsigned short TX_MASK);
#define TX_PORT   P1OUT
#define RX_PORT   P1OUT
#define RX_PIN    P1IN
#define TX_DIR    P1DIR
#define RX_DIR    P1DIR

#define ENABLE_UART_TIMER_ISR  (TACCTL0 |= CCIE)
#define DISABLE_UART_TIMER_ISR (TACCTL0 &= ~CCIE)
#define START_TIMER  TACTL |= MC0
#define CLEAR_TIMER  TACTL |= TACLR

#define READ_RX_PIN(PORT,PIN_MASK)  
#define WRITE_TX_PIN(PORT,PIN_MASK) 
#define CLEAR_TX_PIN(PORT,PIN_MASK) 


#endif // PLATFORMSPECIFIC
