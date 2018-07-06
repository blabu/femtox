#include "PlatformSpecific.h"
#include "TaskMngr.h"

/********************************************************************************************************************
*********************************************************************************************************************
                                            ПЛАТФОРМО-ЗАВИСИМЫЕ ФУНКЦИИ														|
*********************************************************************************************************************
*********************************************************************************************************************/

// Функции void _init_Timer() - устанавливают начальное значения Т/С0. настраивает частоту тактирования и включает таймер
//Включение таймеров происходит после установки битов CSn0-CSn2 (рекомендуется в функции main)
#define TIMERB_10MS 320
void _init_Timer()  // Настроим таймер на прерывания каждые 10 мс
{
  TBCTL = 0;
  TBCTL |= TBCLR;         // Очистка таймера В
  TBCTL &= ~(CNTL0+CNTL1); // разрядность счетчика - 16 бит (считает до 0xFFFF)
  TBCTL |= TBSSEL0;       // Источник тактов - ACLK (32 KHz)
  TBCTL &= ~(ID0+ID1);    // Делитель входного сигнала = 1
  TBCTL &= ~TBIE;          // Не разрешаем прерывания от таймера В по флагу TAIFG
  TBCCTL0 = 0;
  TBCCR0 = TIMERB_10MS;    // До этого значения досчитывает таймер с данной частотой тактирования за 10мс.
  TBCCTL0 = 0;
  TBCCTL0 |= CLLD0;       // Загрузка уставки в регистр защелку происходит в момент обнуления счетчика.
  TBCCTL0 &= ~CAP;       // Выбираем режим сравнения
  TBCCTL0 |= CCIE;      // Разрешаем прерывания по событию захват/сравнение канала 0
  
  TBCTL |= MC0;  // Выбираем режим прямого счета (таймер считает до регистра защелки TBCL0)
}

unsigned int _setTickTime(unsigned int timerTicks) {
	//FIXME Not implemented yet
}

//Обработчик прерывания по совпадению теущего значения таймера и счетчика.
__interrupt_vec(TIMERB0_VECTOR) void TCB_ISR(void){
     __low_power_mode_off_on_exit();  // Выход из режима пониженного энергопотребления (Здесь в любом случае лишним не будет)
     TimerISR();
} 	//Отработка прерывания по переполнению TCNT0


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
  TACTL = 0; // Обнуляем регистр управления таймером А (TimerA ConTroL)
  TACTL |= TACLR; // Сбросим таймер и все предделители
  TACTL |= TASSEL1; //Выбираем источником тактовых сигналов SMCLK
  TACTL |= (ID0+ID1); // Выбираем делитель тактовой частоты, равный 8
  // Записываем необходимые значения в регистры захвата сравнения
  TACCR0 = TIME_CLK_us;  // Записываем необходимое число для генерации с заданной частотой
  TACCTL0 |= CCIE; // Разрешаем прерывания по достижении события совпадения. Генерирует прерывание по вектору TACCR0 (флаг TACCR0 CCIFG)
  TACTL |= MC0; // Выбираем режим таймера "вверх до значения записанного в TACCR0" после этой инструкции таймер начнет считать
}

__interrupt_vec(TIMERA0_VECTOR)void TimerA_ISR(){
    UARTTimerISR();
}
#endif
