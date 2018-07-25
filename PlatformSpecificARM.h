#ifndef PLATFORMSPECIFIC
#define PLATFORMSPECIFIC
#include "stm32l1xx_hal.h"
#include "cmsis_gcc.h"
#include "stm32l152xe.h"

#define ARCH 32 /*Архитектура процессора 8, 16, 32 байта (разрядность шины данных)*/
void initWatchDog();
void resetWatchDog();
/********************************************************************************************************************
*********************************************************************************************************************
                                            ПЛАТФОРМО-ЗАВИСИМЫЕ ФУНКЦИИ														|
*********************************************************************************************************************
*********************************************************************************************************************/
#define INTERRUPT_ENABLE  __enable_irq()   //{asm("nop"); __asm__ __volatile__("eint");}
#define INTERRUPT_DISABLE __disable_irq()  //{__asm__ __volatile__("dint nop"); asm("nop");}
#define INTERRUPT_STATUS  (__get_CONTROL() & (uint32_t)(1<<7))
#define WATCH_DOG_ON  initWatchDog()/*Генерируем Reset*/
#define TICK_PER_SECOND 128 /*Колличество тиков в секунду*/

void _init_Timer(void);	// Инициализация таймера 0, настройка прерываний каждую 1 мс, установки начальных значений для массива таймеров


/*
 * Для программного UART
 * Все програмные UART задействует прерывания одного таймера
 * Все програмные UART должны находится на одном порту ввода вывода, который указывается здесь же
 * Если программных UART будет больше двух необходимо добавлять новые функции в ProgramUART.c
*/
void _initTimerSoftUart();
void initProgramUartGPIO(unsigned short TX_MASK, unsigned short RX_MASK);
void _deInitTimerSoftUart();
void deInitProgramUartGPIO(unsigned short TX_MASK, unsigned short RX_MASK);
extern TIM_HandleTypeDef TIM7InitStruct;
#define TX_PORT   GPIOA
#define RX_PIN    GPIOA
#define ENABLE_UART_TIMER_ISR  __HAL_TIM_ENABLE_IT(&TIM7InitStruct,TIM_IT_UPDATE)
#define DISABLE_UART_TIMER_ISR __HAL_TIM_DISABLE_IT(&TIM7InitStruct,TIM_IT_UPDATE)
#define START_TIMER			   __HAL_TIM_ENABLE(&TIM7InitStruct)
#define CLEAR_TIMER			   __HAL_TIM_SET_COUNTER(&TIM7InitStruct,0)

#define READ_RX_PIN(PORT,PIN_MASK)  HAL_GPIO_ReadPin(PORT, PIN_MASK)
#define WRITE_TX_PIN(PORT,PIN_MASK) HAL_GPIO_WritePin(PORT,PIN_MASK,GPIO_PIN_SET)
#define CLEAR_TX_PIN(PORT,PIN_MASK) HAL_GPIO_WritePin(PORT,PIN_MASK,GPIO_PIN_RESET)


#endif // PLATFORMSPECIFIC
