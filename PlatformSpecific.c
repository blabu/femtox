#include "PlatformSpecific.h"
#include "TaskMngr.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_rcc.h"
/********************************************************************************************************************
*********************************************************************************************************************
                                            ПЛАТФОРМО-ЗАВИСИМЫЕ ФУНКЦИИ														|
*********************************************************************************************************************
*********************************************************************************************************************/

// Функции void _init_Timer() - устанавливают начальное значения Т/С0. настраивает частоту тактирования и включает таймер
//Включение таймеров происходит после установки битов CSn0-CSn2 (рекомендуется в функции main)
#define TIMERB_10MS 320
void _init_Timer_()  // Настроим таймер на прерывания каждые 1 мс
{
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
    SysTick_Config(HSI_VALUE/TICK_PER_SECOND);
}


void SysTick_Handler(void)
{//Обработчик прерывания по совпадению теущего значения таймера и счетчика.
    TimerISR();
}

void _init_Timer(){

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); // включаем тактирование таймера

    /* Другие параметры структуры TIM_TimeBaseInitTypeDef
     * не имеют смысла для базовых таймеров.
     */
    TIM_TimeBaseInitTypeDef baseTimerStruct;
    TIM_TimeBaseStructInit(&baseTimerStruct); // Начальная инициализация структуры таймер

    /* Делитель учитывается как TIM_Prescaler + 1, поэтому отнимаем 1 */
    baseTimerStruct.TIM_Prescaler = 16 - 1; // делитель 16 - 1000 импульсов в мс
    baseTimerStruct.TIM_Period = 1000-1; //период 1000 импульсов

    TIM_Cmd(TIM4, ENABLE);  // Включаем таймер
    TIM_TimeBaseInit(TIM4, &baseTimerStruct);

  /* Разрешаем прерывание по обновлению (в данном случае -
   * по переполнению) счётчика таймера TIM6.
   */
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);

  /* Разрешаем обработку прерывания по переполнению счётчика
   * таймера TIM6. это же прерывание
   * отвечает и за опустошение ЦАП.
   */
    NVIC_EnableIRQ(TIM4_IRQn);
}

void TIM4_IRQHandler(void){
    if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET){
        TIM_ClearITPendingBit(TIM4,TIM_IT_Update);
        TimerISR();
    }
}

#ifdef USE_SOFT_UART
/*
****************************************
\\\\   Платформозависимые функции   ////
\\\\         и настройки            ////
****************************************
*/
#include "ProgrammUART.h"

#define TIME_CLK_us 104/*104-52 мкс (16 МГц)  78 - 52 мкс (12 МГц)  */
void _initTimerSoftUart()
{
}

void TimerA_ISR(){
    UARTTimerISR();
}
#endif
