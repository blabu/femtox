#ifndef PLATFORMSPECIFIC_MSP
#define PLATFORMSPECIFIC_MSP
#include <msp430.h>
#include "FemtoxConf.h"
#include "FemtoxTypes.h"

/********************************************************************************************************************
*********************************************************************************************************************
                                            ПЛАТФОРМО-ЗАВИСИМЫЕ ФУНКЦИИ														|
*********************************************************************************************************************
*********************************************************************************************************************/

void initWatchDog(void);
void resetWatchDog(void);

#define WATCH_DOG_ON  initWatchDog() /*Генерируем Reset*/
#define TICK_PER_SECOND 102UL /*Колличество тиков в секунду*/

unlock_t lock(const void*const resourceId);

void _init_Timer(void);	// Инициализация таймера 0, настройка прерываний каждую 1 мс, установки начальных значений для массива таймеров

#include "../PROPERTIES.h"
#ifdef USE_SOFT_UART
#ifdef YAMPOL
#define TX_PORT   P3OUT
#define RX_PIN    P3IN
#endif
#ifdef G25_TANDEM
#define TX_PORT   P1OUT
#define RX_PIN    P1IN
#endif
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
#endif // PLATFORMSPECIFIC_MSP
