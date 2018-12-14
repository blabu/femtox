#include "PlatformSpecific.h"
#ifdef ARM_STM32
#include "stm32f103xb.h"
#include "TaskMngr.h"

#ifdef MAXIMIZE_OVERFLOW_ERROR
	void MaximizeErrorHandler(string_t str){
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
#define TIME_WATCH_DOG 5000UL /*Время перезапуска таймера в мс*/
#define RELOAD_VALUE ((40UL*TIME_WATCH_DOG)/128UL) // Встроенный генератор низкой частоты 40 кГц
#if(RELOAD_VALUE > 0xFFF)
#error "RELOAD value to longer"
#endif

static IWDG_HandleTypeDef watchDog;
void initWatchDog(){
	watchDog.Instance = IWDG;
	watchDog.Init.Prescaler = IWDG_PRESCALER_128;
	watchDog.Init.Reload = RELOAD_VALUE;
	if(HAL_IWDG_Init(&watchDog) == HAL_OK) {
		HAL_IWDG_Start(&watchDog);
	}else {
		writeLogStr("ERROR: Watchdog init error");
	}
}

void resetWatchDog(void){
	HAL_IWDG_Refresh(&watchDog);
}


#include "config.h"
static TIM_HandleTypeDef TIM2InitStruct;
void initTimer2(void){ // APB1 = 72MHz
	__TIM2_CLK_ENABLE();
	TIM2InitStruct.Instance = TIM2;
	TIM2InitStruct.Init.CounterMode = TIM_COUNTERMODE_UP;
#ifdef SERVER
	TIM2InitStruct.Init.Period = 1000-1;
#else
	TIM2InitStruct.Init.Period = 10000-1;
#endif
	TIM2InitStruct.Init.Prescaler = 72-1;
	TIM2InitStruct.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_Base_Init(&TIM2InitStruct);     // Init timer
	HAL_TIM_Base_Start_IT(&TIM2InitStruct);
	HAL_NVIC_EnableIRQ(TIM2_IRQn);
	__HAL_RCC_TIM2_CLK_ENABLE(); // Не отключать таймер в спящем режиме
}

void TIM2_IRQHandler(void){
	if(__HAL_TIM_GET_FLAG(&TIM2InitStruct, TIM_FLAG_UPDATE)){
		__HAL_TIM_CLEAR_FLAG(&TIM2InitStruct, TIM_FLAG_UPDATE);
		TimerISR();
	}
}


void _init_Timer(){
	initTimer2();
	HAL_PWR_DisableSleepOnExit(); // После пробуждения мы работаем в активном режиме
}

#ifdef USE_SOFT_UART
/*
****************************************
\\\\   Платформозависимые функции   ////
\\\\         и настройки            ////
****************************************
*/
TIM_HandleTypeDef TIM7InitStruct;
#include "ProgrammUART.h"
#include "stm32l1xx_hal_gpio.h"

//52 мкс
void _initTimerSoftUart()
{
	__TIM7_CLK_ENABLE();
	TIM7InitStruct.Instance = TIM7;
	TIM7InitStruct.Init.CounterMode = TIM_COUNTERMODE_UP;
	TIM7InitStruct.Init.Period = 52-1;
	TIM7InitStruct.Init.Prescaler = 4-1;
	TIM7InitStruct.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_Base_Init(&TIM7InitStruct);     // Init timer
	HAL_TIM_Base_Start_IT(&TIM7InitStruct);
	HAL_NVIC_EnableIRQ(TIM7_IRQn);

}

void initProgramUartGPIO(unsigned short TX_MASK, unsigned short RX_MASK){
    GPIO_InitTypeDef gpioStruct;
    gpioStruct.Mode = GPIO_MODE_OUTPUT_PP;
    gpioStruct.Pin = TX_MASK;
    gpioStruct.Pull = GPIO_NOPULL;
    gpioStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(PROGRAMM_TX_PORT,&gpioStruct);
    HAL_GPIO_WritePin(PROGRAMM_TX_PORT,TX_MASK,GPIO_PIN_SET);

    gpioStruct.Mode = GPIO_MODE_INPUT;
    gpioStruct.Pin = RX_MASK;
    gpioStruct.Pull = GPIO_PULLUP;
    gpioStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(PROGRAMM_RX_PIN,&gpioStruct);
}

void TIM7_IRQHandler(void){
	if(__HAL_TIM_GET_FLAG(&TIM7InitStruct, TIM_FLAG_UPDATE)){
		__HAL_TIM_CLEAR_FLAG(&TIM7InitStruct, TIM_FLAG_UPDATE);
    	UARTTimerISR();
	}
}

#endif

#endif
