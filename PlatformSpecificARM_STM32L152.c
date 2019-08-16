#include "PlatformSpecificARM.h"
#include "TaskMngr.h"
#include "stm32l1xx_hal.h"
#include "stm32l152xe.h"

#ifdef MAXIMIZE_OVERFLOW_ERROR
#ifdef ENABLE_LOGGING
#include "logging.h"
#endif
	void MaximizeErrorHandler(string_t str) {
#ifdef ENABLE_LOGGING
		writeLogStr(str);
#endif
		for(u16 i = 0; i<0xFFFF; i++);
		initWatchDog();
		while(1);
	}
#else
#include "logging.h"
	void MaximizeErrorHandler(string_t str){
		writeLogStr(str);
	}
#endif
/********************************************************************************************************************
*********************************************************************************************************************
                                            ПЛАТФОРМО-ЗАВИСИМЫЕ ФУНКЦИИ														|
*********************************************************************************************************************
*********************************************************************************************************************/


//#define INTERRUPT_ENABLE  __enable_irq()   //{asm("nop"); __asm__ __volatile__("eint");}
//#define INTERRUPT_DISABLE __disable_irq()  //{__asm__ __volatile__("dint nop"); asm("nop");}
//#define INTERRUPT_STATUS  (__get_CONTROL() & (uint32_t)(1<<7))

static void unlock(const void*const resourceId) {
    __enable_irq();
}

static void empty(const void*const resourceId) {}

unlock_t lock(const void*const resourceId){
	if((__get_CONTROL() & (uint32_t)(1<<7))) {
	    __disable_irq();
		return unlock;
	}
	return empty;
}


#define TIME_WATCH_DOG 5000UL /*Время перезапуска таймера в мс*/
#define RELOAD_VALUE ((40UL*TIME_WATCH_DOG)/128UL)
#if(RELOAD_VALUE > 0xFFF)
#error "RELOAD value to longer"
#endif

static IWDG_HandleTypeDef watchDog;
void initWatchDog(){
	watchDog.Instance = IWDG;
	watchDog.Init.Prescaler = IWDG_PRESCALER_128;
	watchDog.Init.Reload = RELOAD_VALUE;
	HAL_IWDG_Init(&watchDog);
}

#include "fonTask.h" // FIXME delete it after testing
void resetWatchDog(void){
	blinkLed(3);//FIXME delete it after testing
	HAL_IWDG_Refresh(&watchDog);
}

//#define RTC_CLOCK_SOURCE_LSI
#define RTC_CLOCK_SOURCE_LSE
static void lowLevelInitRTC(void) {
	RCC_OscInitTypeDef        RCC_OscInitStruct;
	RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

	HAL_PWR_EnableBkUpAccess(); // Enable write access using HAL_PWR_EnableBkUpAccess()

	/*##-2- Configue LSE/LSI as RTC clock soucre ###############################*/
#ifdef RTC_CLOCK_SOURCE_LSE
	RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.LSIState = RCC_LSI_OFF;
	if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){
		MaximizeErrorHandler("lowLevelInitRTC osc config error");
	}

	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
	PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
	if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
		MaximizeErrorHandler("lowLevelInitRTC RCC error");
	}
#elif defined (RTC_CLOCK_SOURCE_LSI)
	RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
	if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)  {
		MaximizeErrorHandler();
	}

	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
	PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
	if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
	{
		MaximizeErrorHandler();
	}
#else
#error Please select the RTC Clock source inside the main.h file
#endif /*RTC_CLOCK_SOURCE_LSE*/

	/*##-2- Enable RTC peripheral Clocks Enable RTC Clock */
	__HAL_RCC_RTC_ENABLE();

	/*##-4- Configure the NVIC for RTC Tamper ###################################*/
	HAL_NVIC_SetPriority(RTC_WKUP_IRQn, 0x0F, 0);
	HAL_NVIC_EnableIRQ(RTC_WKUP_IRQn);
}

static RTC_HandleTypeDef RTC_Clock;
void initRTC(void) {
	RTC_Clock.Instance = RTC;
	lowLevelInitRTC();
	HAL_RTCEx_SetWakeUpTimer_IT(&RTC_Clock,(2048/TICK_PER_SECOND)-1,RTC_WAKEUPCLOCK_RTCCLK_DIV16);
}

void RTC_WKUP_IRQHandler(void) {
	HAL_RTCEx_WakeUpTimerIRQHandler(&RTC_Clock);
}

void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc) {
	TimerISR();
}

#ifdef NATIVE_TIMER_PWR_SAVE
#error "Can not compile whith it. Not implemented yet"
#endif

#ifdef TIMER6_USING
static const u32 standartTickTime =  625-1;
static TIM_HandleTypeDef TIM6InitStruct;
void initTimer6(void){ // APB1 = 8MHz
	__TIM6_CLK_ENABLE();
	TIM6InitStruct.Instance = TIM6;
	TIM6InitStruct.Init.CounterMode = TIM_COUNTERMODE_UP;
	TIM6InitStruct.Init.Period = standartTickTime; // OLD_VALUE = 5000-1
	TIM6InitStruct.Init.Prescaler = 128-1;// OLD VALUE = 16-1;
	TIM6InitStruct.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_Base_Init(&TIM6InitStruct);     // Init timer
	HAL_TIM_Base_Start_IT(&TIM6InitStruct);
	HAL_NVIC_EnableIRQ(TIM6_IRQn);
	__HAL_RCC_TIM6_CLK_SLEEP_ENABLE(); // Не отключать таймер в спящем режиме
}

void TIM6_IRQHandler(void){
	if(__HAL_TIM_GET_FLAG(&TIM6InitStruct, TIM_FLAG_UPDATE)){
		__HAL_TIM_CLEAR_FLAG(&TIM6InitStruct, TIM_FLAG_UPDATE);
		TimerISR();
	}
}
#ifdef NATIVE_TIMER_PWR_SAVE
unsigned int _setTickTime(unsigned int timerTicks) {
	unsigned int oldValue = TIM6InitStruct.Init.Period;
	unsigned int newValue = standartTickTime;
	unsigned int i = 1;
	for(;i<timerTicks; i++) {
		if(newValue > 0xFFFF-1) break;
		newValue += standartTickTime;
	}
	if(newValue != oldValue) {
		__HAL_TIM_SET_AUTORELOAD(&TIM6InitStruct,newValue);
	}
	return i;
}
#endif //NATIVE_TIMER_PWR_SAVE
#endif //FOR_TIMER_6

void _init_Timer(){
	initRTC();
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
#include "stm32l1xx_hal_gpio.h"

//26 мкс
void _initTimerSoftUart(){
	__TIM7_CLK_ENABLE();
	TIM7InitStruct.Instance = TIM7;
	TIM7InitStruct.Init.CounterMode = TIM_COUNTERMODE_UP;
	TIM7InitStruct.Init.Period = 26-1;
	TIM7InitStruct.Init.Prescaler = 4-1;
	TIM7InitStruct.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_Base_Init(&TIM7InitStruct);     // Init timer
	HAL_TIM_Base_Start_IT(&TIM7InitStruct);
	HAL_NVIC_EnableIRQ(TIM7_IRQn);
}

void _deInitTimerSoftUart() {
	HAL_TIM_Base_Stop_IT(&TIM7InitStruct);
	HAL_TIM_Base_DeInit(&TIM7InitStruct);
	__TIM7_CLK_DISABLE();
}

void initProgramUartGPIO(unsigned short TX_MASK, unsigned short RX_MASK){
    GPIO_InitTypeDef gpioStruct;
    gpioStruct.Mode = GPIO_MODE_OUTPUT_PP;
    gpioStruct.Pin = TX_MASK;
    gpioStruct.Pull = GPIO_NOPULL;
    gpioStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(TX_PORT,&gpioStruct);
    HAL_GPIO_WritePin(TX_PORT,TX_MASK,GPIO_PIN_SET);

    gpioStruct.Mode = GPIO_MODE_INPUT;
    gpioStruct.Pin = RX_MASK;
    gpioStruct.Pull = GPIO_PULLUP;
    gpioStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(RX_PIN,&gpioStruct);
}

void deInitProgramUartGPIO(unsigned short TX_MASK, unsigned short RX_MASK) {
    HAL_GPIO_WritePin(TX_PORT,TX_MASK,GPIO_PIN_RESET);
    GPIO_InitTypeDef gpioStruct;
    gpioStruct.Mode = GPIO_MODE_OUTPUT_OD;
    gpioStruct.Pin = RX_MASK;
    gpioStruct.Pull = GPIO_NOPULL;
    gpioStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(RX_PIN,&gpioStruct);
    HAL_GPIO_WritePin(RX_PIN,RX_MASK,GPIO_PIN_RESET);

}

void TIM7_IRQHandler(void){
	if(__HAL_TIM_GET_FLAG(&TIM7InitStruct, TIM_FLAG_UPDATE)){
		__HAL_TIM_CLEAR_FLAG(&TIM7InitStruct, TIM_FLAG_UPDATE);
    	UARTTimerISR();
	}
}

#endif
