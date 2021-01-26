#ifndef PLATFORMSPECIFIC_ARM
#define PLATFORMSPECIFIC_ARM
#include "platform.h"
#ifdef ARM_STM32
#include "stm32l1xx_hal.h"
#include "stm32l1xx.h"
#include "cmsis_gcc.h"

#include "FemtoxTypes.h"

void initWatchDog(void);
void resetWatchDog(void);
/********************************************************************************************************************
*********************************************************************************************************************
                                            ПЛАТФОРМО-ЗАВИСИМЫЕ ФУНКЦИИ														|
*********************************************************************************************************************
*********************************************************************************************************************/

unlock_t lock(const void*const resourceId);

#define WATCH_DOG_ON  initWatchDog()/*Генерируем Reset*/
#define TICK_PER_SECOND 128UL /*Колличество тиков в секунду*/

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

#endif
#endif // PLATFORMSPECIFIC_ARM
