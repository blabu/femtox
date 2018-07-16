#include "ProgrammUART.h"
#include "PlatformSpecific.h"

#ifdef USE_SOFT_UART

static SoftUART_t UART_TX_DATA[UART_NUMB];
static SoftUART_t UART_RX_DATA[UART_NUMB];

#define UART_TRANS_FINISH(numberUART)(UART_TX_DATA[numberUART].curentCount  = -2)                        /*Закончена передача*/
#define UART_TRANS_DISABLE(numberUART)(UART_TX_DATA[numberUART].curentCount  = -1)                       /*Выключить передачу*/
#define UART_TRANS_IS_READY(numberUART)(UART_TX_DATA[numberUART].curentCount == -2)                       /*Байт передан?*/
#define UART_TRANS_IS_DISABLE(numberUART)(UART_TX_DATA[numberUART].curentCount == -1)                     /*Передача выключена?*/

#define UART_RECEIV_DISABLE(numberUART)(UART_RX_DATA[numberUART].curentCount = -1)                       /*Выключить прием*/
#define UART_RECEIV_FINISH(numberUART) (UART_RX_DATA[numberUART].curentCount = -2)                       /*Прием закончен*/
#define UART_RECEIV_IS_READY(numberUART) (UART_RX_DATA[numberUART].curentCount == (-2))                  /*Принят байт?*/

#if(UART_NUMB > 0)
/********************UART_0**********************************/
static bool_t isStartBit0() // Для инициализации события (поиск стартоаого бита)
{
  if(UART_RX_DATA[0].curentCount < 0) // Если таймер выключен
  {
    if(!READ_RX_PIN(PROGRAMM_RX_PIN, UART_RX_DATA[0].Mask))   // Проверяем стартовый бит
    {
      return TRUE;             // Если стартового бита обнаружен 
    }
  }
  return FALSE;
}
static void StartReceive0() // Запуск приема
{
  CLEAR_TIMER;
  UART_RX_DATA[0].curentCount = UART_RX_DATA[0].Baud + (UART_RX_DATA[0].Baud>>2);
}
static bool_t UART_RX0_predicate()  // Если байт принят вернем истину
{
    if(UART_RECEIV_IS_READY(0)) return TRUE;
    return FALSE;
}
static void UART_RX0_to_buff()  // Запись принятого байта в буфер
{
    UART_RECEIV_DISABLE(0); // Выключаем UART 
    PutToBackQ(&UART_RX_DATA[0].Data, UART_RX_DATA[0].buffer); // Записываем байт в буфер
}
static bool_t UART_TX0_predicate()  // Если байт принят вернем истину
{
    if(UART_TRANS_IS_READY(0)) return TRUE;
    return FALSE;
}
static void UART_TX0_to_buff()  // Запись принятого байта в буфер
{
    unsigned char temp = 0;
    if(GetFromQ(&temp, UART_TX_DATA[0].buffer) == EVERYTHING_IS_OK) 
    {
        DISABLE_UART_TIMER_ISR;
        UART_TX_DATA[0].Data = temp;
        UART_TX_DATA[0].curentCount = 0;
        ENABLE_UART_TIMER_ISR;
        return;
    }
    UART_TRANS_DISABLE(0);
}
#endif

#if(UART_NUMB > 1)
/********************UART_1**********************************/
static bool_t isStartBit1() // Для инициализации события (поиск стартоаого бита)
{
  if(UART_RX_DATA[1].curentCount < 0) // Если таймер выключен
  {
    if(!READ_RX_PIN(PROGRAMM_RX_PIN, UART_RX_DATA[1].Mask))   // Проверяем стартовый бит
    {
      return TRUE;             // Если стартового бита обнаружен 
    }
  }
  return FALSE;
}
static void StartReceive1() // Запуск приема
{
  CLEAR_TIMER;
  UART_RX_DATA[1].curentCount = UART_RX_DATA[1].Baud + (UART_RX_DATA[1].Baud>>2);
}
static bool_t UART_RX1_predicate()  // Если байт принят вернем истину
{
    if(UART_RECEIV_IS_READY(1)) return TRUE;
    return FALSE;
}
static void UART_RX1_to_buff()  // Запись принятого байта в буфер
{
    UART_RECEIV_DISABLE(1); // Выключаем UART 
    PutToBackQ(&UART_RX_DATA[1].Data, UART_RX_DATA[1].buffer); // Записывае байт в буфер
}
static bool_t UART_TX1_predicate()  // Если байт принят вернем истину
{
    if(UART_TRANS_IS_READY(1)) return TRUE;
    return FALSE;
}
static void UART_TX1_to_buff(){  // Запись принятого байта в буфер
    unsigned char temp = 0;
    if(GetFromQ(&temp, UART_TX_DATA[1].buffer) == EVERYTHING_IS_OK) {
        DISABLE_UART_TIMER_ISR;
        UART_TX_DATA[1].Data = temp;
        UART_TX_DATA[1].curentCount = 0;
        ENABLE_UART_TIMER_ISR;
        return;
    }
    UART_TRANS_DISABLE(1);
}
#endif

#if(UART_NUMB > 2)
/********************UART_2**********************************/
static bool_t isStartBit2(){ // Для инициализации события (поиск стартоаого бита)
  if(UART_RX_DATA[2].curentCount < 0) // Если таймер выключен
  {
    if(!READ_RX_PIN(PROGRAMM_RX_PIN, UART_RX_DATA[2].Mask))   // Проверяем стартовый бит
    {
      return TRUE;             // Если стартового бита обнаружен
    }
  }
  return FALSE;
}
static void StartReceive2(){ // Запуск приема
  CLEAR_TIMER;
  UART_RX_DATA[2].curentCount = UART_RX_DATA[2].Baud + (UART_RX_DATA[2].Baud>>2);
}
static bool_t UART_RX2_predicate(){  // Если байт принят вернем истину
    if(UART_RECEIV_IS_READY(2)) return TRUE;
    return FALSE;
}
static void UART_RX2_to_buff(){  // Запись принятого байта в буфер
    UART_RECEIV_DISABLE(2); // Выключаем UART
    PutToBackQ(&UART_RX_DATA[2].Data, UART_RX_DATA[2].buffer); // Записывае байт в буфер
}
static bool_t UART_TX2_predicate(){  // Если байт принят вернем истину
    if(UART_TRANS_IS_READY(2)) return TRUE;
    return FALSE;
}
static void UART_TX2_to_buff(){  // Запись принятого байта в буфер
    unsigned char temp = 0;
    if(GetFromQ(&temp, UART_TX_DATA[2].buffer) == EVERYTHING_IS_OK) {
        DISABLE_UART_TIMER_ISR;
        UART_TX_DATA[2].Data = temp;
        UART_TX_DATA[2].curentCount = 0;
        ENABLE_UART_TIMER_ISR;
        return;
    }
    UART_TRANS_DISABLE(1);
}
#endif

void initSoftUART(){
    for(unsigned char i = 0; i < UART_NUMB; i++){
      UART_RX_DATA[i].Mask = 0;
      UART_TX_DATA[i].Mask = 0;
      UART_RECEIV_DISABLE(i);
      UART_TRANS_DISABLE(i);
      UART_RX_DATA[i].Data = 0;
      UART_TX_DATA[i].Data = 0;
      UART_RX_DATA[i].Baud = 0;
      UART_TX_DATA[i].Baud = 0;
      UART_RX_DATA[i].buffer = NULL;
      UART_TX_DATA[i].buffer = NULL;
    }
}

void enableSoftUART() {
  setFlags(SOFT_UART_WORK_FLAG);
  for(unsigned char i = 0; i < UART_NUMB; i++){
    initProgramUartGPIO(UART_RX_DATA[i].Mask,UART_TX_DATA[i].Mask);
#if(UART_NUMB > 0)
    if(i == 0)
    {
        CreateEvent(isStartBit0,StartReceive0);  // Регистрируем событие поиска стартового бита
        CreateEvent(UART_RX0_predicate, UART_RX0_to_buff); // Регистрируем событие сохранения принятого байта в буфер
        CreateEvent(UART_TX0_predicate, UART_TX0_to_buff);
    }
#endif
#if(UART_NUMB > 1)
    if(i == 1)
    {
        CreateEvent(isStartBit1,StartReceive1);  // Регистрируем событие поиска стартового бита
        CreateEvent(UART_RX1_predicate, UART_RX1_to_buff); // Регистрируем событие сохранения принятого байта в буфер
        CreateEvent(UART_TX1_predicate, UART_TX1_to_buff);
    }
#endif
#if(UART_NUMB > 2)
    if(i == 2)
    {
        CreateEvent(isStartBit2,StartReceive2);  // Регистрируем событие поиска стартового бита
        CreateEvent(UART_RX2_predicate, UART_RX2_to_buff); // Регистрируем событие сохранения принятого байта в буфер
        CreateEvent(UART_TX2_predicate, UART_TX2_to_buff);
    }
#endif
  }
  _initTimerSoftUart();
}


void disableSoftUART() {
  _deInitTimerSoftUart();
   for(unsigned char i = 0; i < UART_NUMB; i++){
      UART_RECEIV_DISABLE(i);
      UART_TRANS_DISABLE(i);
#if(UART_NUMB > 0)
    if(i == 0){
        delEvent(isStartBit0);  // Регистрируем событие поиска стартового бита
        delEvent(UART_RX0_predicate); // Регистрируем событие сохранения принятого байта в буфер
        delEvent(UART_TX0_predicate);
    }
#endif
#if(UART_NUMB > 1)
    if(i == 1) {
        delEvent(isStartBit1);  // Регистрируем событие поиска стартового бита
        delEvent(UART_RX1_predicate); // Регистрируем событие сохранения принятого байта в буфер
        delEvent(UART_TX1_predicate);
    }
#endif
#if(UART_NUMB > 2)
    if(i == 2) {
        delEvent(isStartBit2);  // Регистрируем событие поиска стартового бита
        delEvent(UART_RX2_predicate); // Регистрируем событие сохранения принятого байта в буфер
        delEvent(UART_TX2_predicate);
    }
#endif
    deInitProgramUartGPIO(UART_RX_DATA[i].Mask,UART_TX_DATA[i].Mask);
   }
   clearFlags(SOFT_UART_WORK_FLAG);
}

void CreateSoftUART(const BaseSize_t buffTXsize, const BaseSize_t buffRXsize, const s08 BAUD,
                    const u08 numbUART, const u08 TXpinNumber, const u08 RXpinNumber){
    if(numbUART >= UART_NUMB) return;
    DISABLE_UART_TIMER_ISR;
    UART_TX_DATA[numbUART].Mask = 1<<TXpinNumber;
    UART_RX_DATA[numbUART].Mask = 1<<RXpinNumber;
    UART_TX_DATA[numbUART].Baud = BAUD;
    UART_RX_DATA[numbUART].Baud = BAUD;
    UART_TRANS_DISABLE(numbUART);
    UART_RECEIV_DISABLE(numbUART);
    UART_TX_DATA[numbUART].Data = 0;
    UART_RX_DATA[numbUART].Data = 0;
    UART_TX_DATA[numbUART].buffer = allocMem(buffTXsize);
    if(UART_TX_DATA[numbUART].buffer == NULL) {
#ifdef MAXIMIZE_OVERFLOW_ERROR
		while(1);
#endif
    }
    UART_RX_DATA[numbUART].buffer = allocMem(buffRXsize);
    if(UART_TX_DATA[numbUART].buffer == NULL) {
#ifdef MAXIMIZE_OVERFLOW_ERROR
		while(1);
#endif
    }
    
    if(CreateQ(UART_TX_DATA[numbUART].buffer, 1, buffTXsize) != EVERYTHING_IS_OK) {
#ifdef MAXIMIZE_OVERFLOW_ERROR
		while(1);
#endif
    }
    if(CreateQ(UART_RX_DATA[numbUART].buffer, 1, buffRXsize) != EVERYTHING_IS_OK) {
#ifdef MAXIMIZE_OVERFLOW_ERROR
		while(1);
#endif
    }
}

static void dataReceive(const u08 numbUART){
  static unsigned char bitCount[UART_NUMB] = {0};
  static unsigned char result[UART_NUMB] = {0};
  
  if(UART_RX_DATA[numbUART].curentCount < 0) {bitCount[numbUART] = result[numbUART] = 0; return;} //UART выключен
  if(UART_RX_DATA[numbUART].curentCount > 1) {UART_RX_DATA[numbUART].curentCount--; return;}
  if(bitCount[numbUART] != DATA_BITS) {
    if(READ_RX_PIN(PROGRAMM_RX_PIN, UART_RX_DATA[numbUART].Mask)) result[numbUART] |= 1<<bitCount[numbUART];
    bitCount[numbUART]++;
    UART_RX_DATA[numbUART].curentCount = UART_RX_DATA[numbUART].Baud;
    return;
  }
  UART_RX_DATA[numbUART].Data = result[numbUART];
  result[numbUART] = 0;
  bitCount[numbUART] = 0;
  UART_RECEIV_FINISH(numbUART);
}

static void dataTransmit(const u08 numbUART){ // Передача одного байта
	static unsigned char bitCount[UART_NUMB] = {0};
    if(UART_TX_DATA[numbUART].curentCount < 0) {bitCount[numbUART]=0; return;}// Передача отключена
    if(UART_TX_DATA[numbUART].curentCount > 1) {UART_TX_DATA[numbUART].curentCount--; return;}
    if(bitCount[numbUART] <= DATA_BITS) {
      UART_TX_DATA[numberUART].curentCount = UART_TX_DATA[numberUART].Baud;
      if(bitCount[numbUART]) {
        bitCount[numbUART]++;   // Увеличиваем счетчик переданых бит
        if(UART_TX_DATA[numbUART].Data & 0x01) WRITE_TX_PIN(PROGRAMM_TX_PORT,UART_TX_DATA[numbUART].Mask); // Передаем младший бит
        else CLEAR_TX_PIN(PROGRAMM_TX_PORT,UART_TX_DATA[numbUART].Mask);   // Передаем младший бит
        UART_TX_DATA[numbUART].Data >>= 1;              // Смещаем регистр данных
        return;
      } else {
          bitCount[numbUART] = 1; // Счетчик переданых бит
          CLEAR_TX_PIN(PROGRAMM_TX_PORT,UART_TX_DATA[numbUART].Mask); //  Ножку контроллера в низкий уровень (Старт бит)
          return;
      }
    }
    //Здесь мы если мы все уже передали
    bitCount[numbUART] = 0;
    WRITE_TX_PIN(PROGRAMM_TX_PORT,UART_TX_DATA[numbUART].Mask);//  Ножку контроллера в высокий уровень (Стоп бит) байт отправлен
    UART_TRANS_FINISH(numbUART);// Выключаем передачу  
}

void sendUART_byte(const u08 numbUART,const u08 U_data){ // Функция передачи данных
    if(numbUART >= UART_NUMB) return;
    if(UART_TRANS_IS_DISABLE(numbUART))
    {
        DISABLE_UART_TIMER_ISR;
        UART_TX_DATA[numbUART].Data = U_data;
        UART_TX_DATA[numbUART].curentCount = 0;
        ENABLE_UART_TIMER_ISR;
    }
    else{
        PutToBackQ(&U_data, UART_TX_DATA[numbUART].buffer);
    }
}

void sendUART_str(const u08 numbUART, const string_t U_data){
  u08 i = 0;
  while(U_data[i] != '\0'){
    sendUART_byte(numbUART, (u08)U_data[i]);
    i++;
  }
}

void sendUART_array(const u08 numbUART, const BaseSize_t size, const unsigned char* U_data){
  for(u08 i = 0;i<size;i++)
  {
    sendUART_byte(numbUART, U_data[i]);
  }
}


void delSoftUART(const u08 numbUART){
  if(numbUART < UART_NUMB)
  {
      freeMem(UART_TX_DATA[numbUART].buffer);
      freeMem(UART_RX_DATA[numbUART].buffer);
  }
  if(numbUART == 0)
  {
        delEvent(isStartBit0);
        delEvent(UART_RX0_predicate);
        delEvent(UART_TX0_predicate);
  }
  if(numbUART == 1)
  {
        delEvent(isStartBit1);
        delEvent(UART_RX1_predicate);
        delEvent(UART_TX1_predicate);
  }
}

void clearRxBuffer(const unsigned char numbUART){
  clearDataStruct(UART_RX_DATA[numbUART].buffer);
}

u08 readUART_byte(u08 numbUART){
    unsigned char data = 0;
    GetFromQ(&data, UART_RX_DATA[numbUART].buffer);
    return data;
}

BaseSize_t readUART_array(const u08 numbUART, const BaseSize_t size, const unsigned char* U_data){
    BaseSize_t i=0;
    for(; i<size; i++){
      if(GetFromQ((U_data+i),UART_RX_DATA[numbUART].buffer) != EVERYTHING_IS_OK) break;
    }
    return i;
}

void UARTTimerISR()// Само прерывание
{
    for(register unsigned char i = 0; i<UART_NUMB; i++)
    {
        dataReceive(i);
        dataTransmit(i);
    }
}
#endif //USE_SOFT_UART
