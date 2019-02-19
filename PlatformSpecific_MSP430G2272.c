#include "PlatformSpecific.h"
#include "TaskMngr.h"
#ifdef MSP430
/********************************************************************************************************************
*********************************************************************************************************************
                                            ПЛАТФОРМО-ЗАВИСИМЫЕ ФУНКЦИИ														|
*********************************************************************************************************************
*********************************************************************************************************************/


#ifdef MAXIMIZE_OVERFLOW_ERROR
    void MaximizeErrorHandler(const string_t str){
        initWatchDog();
        resetWatchDog();
        _enable_interrupt();
        while(1);
    }
#else
    void MaximizeErrorHandler(const string_t str){
    }
#endif

void initWatchDog(void) {
    WDTCTL = WDT_ARST_1000;
}
void resetWatchDog(void) {
    WDTCTL = WDT_ARST_1000; // При 32768Гц = 1секунда У нас 4096Гц => 8секунд
}

// Функции void _init_Timer() - устанавливают начальное значения Т/С0. настраивает частоту тактирования и включает таймер
//Включение таймеров происходит после установки битов CSn0-CSn2 (рекомендуется в функции main)
#define TIMER_CONST 5  /*4096/8=512 тиков в секунду => 5/512=9.76 мс*/
void _init_Timer() { // Настроим таймер на прерывания каждые 10 мс
  TBCTL = 0;
  TBCTL |= TBCLR;         // Очистка таймера В
  TBCTL &= ~(CNTL0+CNTL1); // разрядность счетчика - 16 бит (считает до 0xFFFF)
  TBCTL |= TBSSEL0;       // Источник тактов - ACLK (4096 Hz)
  TBCTL |= (ID0+ID1);     // Делитель входного сигнала = 8 (512 Гц) => 512 импульсов = 1сек
  TBCTL &= ~TBIE;          // Не разрешаем прерывания от таймера В по флагу TAIFG
  TBCCTL0 = 0;
  TBCCR0 = TIMER_CONST;    // До этого значения досчитывает таймер с данной частотой тактирования за 10мс.
  TBCCTL0 = 0;
//  TBCCTL0 |= CLLD0;       // Загрузка уставки в регистр защелку происходит в момент обнуления счетчика.
  TBCCTL0 &= ~CAP;       // Выбираем режим сравнения
  TBCCTL0 |= CCIE;      // Разрешаем прерывания по событию захват/сравнение канала 0
  
  //TBCTL |= MC0;  // Выбираем режим прямого счета (таймер считает до регистра защелки TBCL0)
  TBCTL |= MC1;  // Выбираем режим продолжительного счета (таймер считает до максимума 0xFFFF сбрасівается и сново считает)
}

static u32 TimerDelay = TIMER_CONST; // Во избежания переполнения TimerDelay - побольше

//Обработчик прерывания по совпадению теущего значения таймера и счетчика.
#pragma vector=TIMERB0_VECTOR
__interrupt void TCB_ISR(void){
     TBCCR0 = (u16)(TBR + TimerDelay);
     TimerISR();
     LPM3_EXIT; // Выход из режима пониженного энергопотребления (Здесь в любом случае лишним не будет)
} 	//Отработка прерывания по переполнению TCNT0

#ifdef NATIVE_TIMER_PWR_SAVE
//// Выполняется при запрещенных прервыниях
//u32 _setTickTime(u32 timerTicks) {
//  u32 oldTimer = TimerDelay;
//  TimerDelay = TIMER_CONST;
//  u32 i = 1;
//  for(; i<timerTicks; i++) {
//    TimerDelay += TIMER_CONST;
//    if(TimerDelay > (0xFFFF - (TIMER_CONST<<1))) break; // Следующая итерация может переполнить 16-ти битній счетчик MSP
//  }
//  if(TimerDelay != oldTimer) {
//    TBCTL &= ~MC1; // STOP TIMER
//    TBCCR0 = (u16)(TBR + TimerDelay);
//    TBCTL |= MC1; //START TIMER
//  }
//  return i;
//}

// Выполняется при запрещенных прервыниях
u32 _setTickTime(u32 timerTicks) {
    u32 oldTimer = TimerDelay;
    if(!timerTicks) timerTicks = 1;
    TimerDelay = timerTicks * TIMER_CONST;
    if(TimerDelay != oldTimer) {
        if(TimerDelay > (0xFFFF - (TIMER_CONST))) { // Если такая итерация вызовет переполнение таймера
            #define maxTicks (0xFFFF/TIMER_CONST) // TODO check it in debugger
            #define TIME_TICKS maxTicks * TIMER_CONST
            TimerDelay = TIME_TICKS;      // Выставляем TimerDelay максимально возможной величины
            TBCTL &= ~MC1; // STOP TIMER
            TBCCR0 = (u16)(TBR + TimerDelay);
            TBCTL |= MC1; //START TIMER
            return maxTicks;
            #undef maxTicks
            #undef TIMER_TICKS
        }
        TBCTL &= ~MC1; // STOP TIMER
        TBCCR0 = (u16)(TBR + TimerDelay);
        TBCTL |= MC1; //START TIMER
    }
    return timerTicks;
}

u32 _getTickTime() { // Сколько времени прошло с момента начала отсета до сейчас в стандартных тиках ОС
    u16 res = 0;
    TBCTL &= ~MC1; // STOP TIMER
    if(TBCCR0 > TBR) res = TimerDelay - (TBCCR0 - TBR);
    else res = TimerDelay - ((0xFFFF-TBR)+TBCCR0);
    TBCTL |= MC1; //START TIMER
    return res;
}
#endif

static void unlock(const void*const resourceId) {
    __enable_interrupt();
}

static void empty(const void*const resourceId) {}

unlock_t lock(const void*const resourceId){
    if((__get_interrupt_state() & GIE)) {
        __disable_interrupt();
        return unlock;
    }
    return empty;
}


#ifdef USE_SOFT_UART
/*
****************************************
\\\\   Платформозависимые функции   ////
\\\\         и настройки            ////
****************************************
*/
#define RX_PORT   P1OUT
#define TX_DIR    P1DIR
#define RX_DIR    P1DIR

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
#endif
