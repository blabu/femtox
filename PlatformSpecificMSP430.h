#ifndef PLATFORMSPECIFIC
#define PLATFORMSPECIFIC
#include <msp430.h>
#include "FemtoxConf.h"

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
#define INTERRUPT_STATUS  (__get_interrupt_state() & GIE)
#define WATCH_DOG_ON  WDTCTL = WDTPW /*Генерируем Reset*/
#define TICK_PER_SECOND 100UL /*Колличество тиков в секунду*/

void _init_Timer(void);	// Инициализация таймера 0, настройка прерываний каждую 1 мс, установки начальных значений для массива таймеров

#ifdef USE_SOFT_UART
#define TX_PORT   P3OUT
#define RX_PIN    P3IN

/*
 * Для программного UART
 * Все програмные UART задействует прерывания одного таймера
 * Все програмные UART должны находится на одном порту ввода вывода, который указывается здесь же
 * Если программных UART будет больше двух необходимо добавлять новые функции в ProgramUART.c
*/
void _initTimerSoftUart();
void _deInitTimerSoftUart();
void initProgramUartGPIO(unsigned short TX_MASK, unsigned short RX_MASK);
void deInitProgramUartGPIO(unsigned short TX_MASK, unsigned short RX_MASK);

#define ENABLE_UART_TIMER_ISR  (TACCTL0 |= CCIE)
#define DISABLE_UART_TIMER_ISR (TACCTL0 &= ~CCIE)
#define START_TIMER  TACTL |= MC0
#define CLEAR_TIMER  TACTL |= TACLR

#define READ_RX_PIN(PORT,PIN_MASK)   (PORT & (PIN_MASK))
#define WRITE_TX_PIN(PORT,PIN_MASK)  (PORT |=  (PIN_MASK))
#define CLEAR_TX_PIN(PORT,PIN_MASK)  (PORT &= ~(PIN_MASK))
#endif //USE_SOFT_UART
#endif // PLATFORMSPECIFIC
