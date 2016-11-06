#ifndef PLATFORMSPECIFIC
#define PLATFORMSPECIFIC
#include "stm32f4xx.h"
#include "misc.h"

/********************************************************************************************************************
*********************************************************************************************************************
                                            ПЛАТФОРМО-ЗАВИСИМЫЕ ФУНКЦИИ														|
*********************************************************************************************************************
*********************************************************************************************************************/
#define INTERRUPT_ENABLE  __enable_irq()   //{asm("nop"); __asm__ __volatile__("eint");}
#define INTERRUPT_DISABLE __disable_irq()  //{__asm__ __volatile__("dint nop"); asm("nop");}
#define INTERRUPT_STATUS  (__get_CONTROL() & (uint32_t)(1<<7))
#define WATCH_DOG_ON  /*Генерируем Reset*/
#define TICK_PER_SECOND 1000 /*Колличество тиков в секунду*/

void _init_Timer(void);	// Инициализация таймера 0, настройка прерываний каждую 1 мс, установки начальных значений для массива таймеров


/*
 * Для программного UART
 * Все програмные UART задействует прерывания одного таймера
 * Все програмные UART должны находится на одном порту ввода вывода, который указывается здесь же
 * Если программных UART будет больше двух необходимо добавлять новые функции в ProgramUART.c
*/
void _initTimerSoftUart();
#define TX_PORT   P1OUT
#define RX_PORT   P1OUT
#define RX_PIN    P1IN
#define TX_DIR    P1DIR
#define RX_DIR    P1DIR

#define ENABLE_UART_TIMER_ISR  (TACCTL0 |= CCIE)
#define DISABLE_UART_TIMER_ISR (TACCTL0 &= ~CCIE)
#define START_TIMER  TACTL |= MC0
#define CLEAR_TIMER  TACTL |= TACLR

#endif // PLATFORMSPECIFIC
