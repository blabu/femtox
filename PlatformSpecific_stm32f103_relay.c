/*
 * PlatformSpecific_stm32f103_relay.c
 *
 *  Created on: Jun 28, 2022
 *      Author: blabu
 */

#include "PlatformSpecific.h"
#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"
#include "TaskMngr.h"

#ifdef MAXIMIZE_OVERFLOW_ERROR
	void MaximizeErrorHandler(string_t str){
		initWatchDog();
		while(1);
	}
#else
	void MaximizeErrorHandler(string_t str){
		while(1);
	}
#endif

static IWDG_HandleTypeDef hiwdg;

#define TIME_WATCH_DOG 5000UL /*Время перезапуска таймера в мс*/
#define RELOAD_VALUE ((40UL*TIME_WATCH_DOG)/128) // Встроенный генератор низкой частоты 40 кГц
void initWatchDog(void) {
	  hiwdg.Instance = IWDG;
	  hiwdg.Init.Prescaler = IWDG_PRESCALER_128;
	  hiwdg.Init.Reload = RELOAD_VALUE;
	  if (HAL_IWDG_Init(&hiwdg) != HAL_OK) {
		MaximizeErrorHandler("Init watchdog handler");
	  }
}

void resetWatchDog(void) {
	HAL_IWDG_Refresh(&hiwdg);
}

static void unlock(const void*const resourceId) {
    __enable_irq();
}

static void empty(const void*const resourceId) {}

unlock_t lock(const void*const resourceId) {
    /* __get_PRIMASK() - Read PRIMASK register, check interrupt status before you disable them */
    /* Returns 0 if they are enabled, or non-zero if disabled */
	if(!__get_PRIMASK()) {
		__disable_irq();
		return unlock;
	}
	return empty;
}

static void timerINT(TIM_HandleTypeDef *htim) {
	TimerISR();
}

TIM_HandleTypeDef htim2;
static void timInit(void) { //APB1 72 MHz
	TIM_ClockConfigTypeDef sClockSourceConfig = {0};

	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 72-1;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 10000-1;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
		MaximizeErrorHandler("Init timer 2 error");
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
		MaximizeErrorHandler("Init timer clock error");
	}
	htim2.PeriodElapsedCallback = timerINT;
	if(HAL_TIM_Base_Start_IT(&htim2) != HAL_OK) {
		MaximizeErrorHandler("Start timer error");
	}
}


// Инициализация таймера 0, настройка прерываний каждую 1 мс, установки начальных значений для массива таймеров
void _init_Timer(void) {
	timInit();
}
