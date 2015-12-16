#include "PlatformSpecific.h"
#include "TaskMngr.h"

/********************************************************************************************************************
*********************************************************************************************************************
                                            ПЛАТФОРМО-ЗАВИСИМЫЕ ФУНКЦИИ														|
*********************************************************************************************************************
*********************************************************************************************************************/

void initWatchDog(void) {
    WDTCTL = WDT_ARST_1000;
}
void resetWatchDog(void) {
    WDTCTL = WDT_ARST_1000;
}

// Функции void _init_Timer() - устанавливают начальное значения Т/С0. настраивает частоту тактирования и включает таймер
//Включение таймеров происходит после установки битов CSn0-CSn2 (рекомендуется в функции main)
static const unsigned char TIMER_CONST = 40;
void _init_Timer()  // Настроим таймер на прерывания каждые 10 мс
{
  TBCTL = 0;
  TBCTL |= TBCLR;         // Очистка таймера В
  TBCTL &= ~(CNTL0+CNTL1); // разрядность счетчика - 16 бит (считает до 0xFFFF)
  TBCTL |= TBSSEL0;       // Источник тактов - ACLK (32 KHz)
  TBCTL |= (ID0+ID1);    // Делитель входного сигнала = 8
  TBCTL &= ~TBIE;          // Не разрешаем прерывания от таймера В по флагу TAIFG
  TBCCTL0 = 0;
  TBCCR0 = TIMER_CONST;    // До этого значения досчитывает таймер с данной частотой тактирования за 10мс.
  TBCCTL0 = 0;
//  TBCCTL0 |= CLLD0;       // Загрузка уставки в регистр защелку происходит в момент обнуления счетчика.
  TBCCTL0 &= ~CAP;       // Выбираем режим сравнения
  TBCCTL0 |= CCIE;      // Разрешаем прерывания по событию захват/сравнение канала 0
  
  TBCTL |= MC0;  // Выбираем режим прямого счета (таймер считает до регистра защелки TBCL0)
}

//Обработчик прерывания по совпадению теущего значения таймера и счетчика.
#pragma vector=TIMERB0_VECTOR
__interrupt void TCB_ISR(void){
     TimerISR();
     __low_power_mode_off_on_exit();  // Выход из режима пониженного энергопотребления (Здесь в любом случае лишним не будет)
} 	//Отработка прерывания по переполнению TCNT0

#ifdef NATIVE_TIMER_PWR_SAVE
static unsigned short TIMERB_10MS = 40;
// Выполняется при запрещенных прервыниях
unsigned int _setTickTime(unsigned int timerTicks) {
  unsigned int oldTimer = TIMERB_10MS;
  TIMERB_10MS = TIMER_CONST;
  unsigned int i = 1;
  for(; i<timerTicks; i++) {
    TIMERB_10MS += TIMER_CONST;
    if(TIMERB_10MS > (0xFFFF - (TIMER_CONST<<1))) break; // Следующая итерация может переполнить счетчик
  }
  if(TIMERB_10MS != oldTimer) {
    TBCCR0 = TIMERB_10MS;
  }
  return i;
}
#endif

#ifdef USE_SOFT_UART
/*
****************************************
\\\\   Платформозависимые функции   ////
\\\\         и настройки            ////
****************************************
*/
#define RX_PORT   P3OUT
#define TX_DIR    P3DIR
#define RX_DIR    P3DIR

#define TIME_CLK_us 52/*104-52 мкс (16 МГц)  78 - 52 мкс (12 МГц)  52 - SMCLK = 8MHz*/
void _initTimerSoftUart() {
  TACCTL0 &= ~CCIE;     // disable interrupt
  TACTL = 0; // Обнуляем регистр управления таймером А (TimerA ConTroL)
  TACTL |= TACLR; // Сбросим таймер и все предделители
  TACTL |= TASSEL1; //Выбираем источником тактовых сигналов SMCLK
  TACTL |= (ID0+ID1); // Выбираем делитель тактовой частоты, равный 8
  // Записываем необходимые значения в регистры захвата сравнения
  TACCR0 = TIME_CLK_us-1;  // Записываем необходимое число для генерации с заданной частотой
  TACCTL0 |= CCIE; // Разрешаем прерывания по достижении события совпадения. Генерирует прерывание по вектору TACCR0 (флаг TACCR0 CCIFG)
  TACTL |= MC0; // Выбираем режим таймера "вверх до значения записанного в TACCR0" после этой инструкции таймер начнет считать
}

void _deInitTimerSoftUart() {
  TACCTL0 &= ~CCIE;     // disable interrupt
  TACTL &= ~(MC0+MC1); // stop timer
  TACTL |= TACLR; // Сбросим таймер и все предделители
}

void initProgramUartGPIO(unsigned short TX_MASK, unsigned short RX_MASK) {
    TX_DIR  |= TX_MASK;
    TX_PORT |= TX_MASK;
    RX_DIR  &= ~RX_MASK;
    RX_PORT |= RX_MASK;
}

void deInitProgramUartGPIO(unsigned short TX_MASK, unsigned short RX_MASK) {
    TX_DIR  |=  TX_MASK;
    RX_DIR  |=  RX_MASK;
    TX_PORT &= ~TX_MASK;
    RX_PORT &= ~RX_MASK;
}

#pragma vector=TIMERA0_VECTOR
__interrupt void TimerA_ISR(){
    UARTTimerISR();
}
#endif
