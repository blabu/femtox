#include "PlatformSpecific.h"
#include "TaskMngr.h"

#ifdef USE_SOFT_UART

#ifndef UART_NUMB
#define UART_NUMB 1
#endif

typedef struct {
    u08 Mask;
    u08 Data;         /*Регистр данных. Сюда записываются данные для передачи*/
    s08 Baud;         /*Сюда записывается одна из переменных BAUD_600 - BAUD_19200*/
    s08 curentCount;  /*Счетчик прерываний по передачи (нужны для организации заданной скорости) + выполняет функцию текущего статуса ЮАРТ*/
    u08* buffer;      /*Буфер приема-передачи*/
} SoftUART_t;

volatile static SoftUART_t UART_TX_DATA[UART_NUMB];
volatile static SoftUART_t UART_RX_DATA[UART_NUMB];

#define UART_TRANS_FINISH(numberUART)(UART_TX_DATA[numberUART].curentCount  = -2)                        /*Закончена передача*/
#define UART_TRANS_DISABLE(numberUART)(UART_TX_DATA[numberUART].curentCount  = -1)                       /*Выключить передачу*/
#define UART_TRANS_IS_READY(numberUART)(UART_TX_DATA[numberUART].curentCount == -2)                       /*Байт передан?*/
#define UART_TRANS_IS_DISABLE(numberUART)(UART_TX_DATA[numberUART].curentCount == -1)                     /*Передача выключена?*/

#define UART_RECEIV_DISABLE(numberUART)(UART_RX_DATA[numberUART].curentCount = -1)                       /*Выключить прием*/
#define UART_RECEIV_FINISH(numberUART) (UART_RX_DATA[numberUART].curentCount = -2)                       /*Прием закончен*/
#define UART_RECEIV_IS_READY(numberUART) (UART_RX_DATA[numberUART].curentCount == (-2))                  /*Принят байт?*/

/********************UART_0**********************************/
static bool_t isStartBit0(){ // Для инициализации события (поиск стартоаого бита)
  if(UART_RX_DATA[0].curentCount < 0) { // Если таймер выключен
    if(! READ_RX_PIN(RX_PIN, UART_RX_DATA[0].Mask)){   // Проверяем стартовый бит
      return TRUE;             // Если стартового бита обнаружен
    }
  }
  return FALSE;
}
static void StartReceive0(){ // Запуск приема
  CLEAR_TIMER;
  UART_RX_DATA[0].curentCount = UART_RX_DATA[0].Baud + (UART_RX_DATA[0].Baud>>2);
}
static bool_t UART_RX0_predicate() { // Если байт принят вернем истину
    if(UART_RECEIV_IS_READY(0)) return TRUE;
    return FALSE;
}
static void UART_RX0_to_buff() { // Запись принятого байта в буфер
    UART_RECEIV_DISABLE(0); // Выключаем UART
    PutToBackQ(&UART_RX_DATA[0].Data, UART_RX_DATA[0].buffer); // Записываем байт в буфер
}
static bool_t UART_TX0_predicate() { // Если байт передан вернем истину
    if(UART_TRANS_IS_READY(0)) return TRUE;
    return FALSE;
}
static void UART_TX0_to_buff() { // Запись принятого байта в буфер
    u08 temp = 0;
    if(GetFromQ(&temp, UART_TX_DATA[0].buffer) == EVERYTHING_IS_OK) {
        DISABLE_UART_TIMER_ISR;
        UART_TX_DATA[0].Data = temp;
        UART_TX_DATA[0].curentCount = 0;
        ENABLE_UART_TIMER_ISR;
        return;
    }
    UART_TRANS_DISABLE(0);
}

#if(UART_NUMB > 1)
/********************UART_1**********************************/
static bool_t isStartBit1() { // Для инициализации события (поиск стартоаого бита)
  if(UART_RX_DATA[1].curentCount < 0) { // Если таймер выключен
    if(! READ_RX_PIN(RX_PIN, UART_RX_DATA[1].Mask) ) {  // Проверяем стартовый бит
      return TRUE;             // Если стартового бита обнаружен
    }
  }
  return FALSE;
}
static void StartReceive1() { // Запуск приема
  CLEAR_TIMER;
  UART_RX_DATA[1].curentCount = UART_RX_DATA[1].Baud + (UART_RX_DATA[1].Baud>>2);
}
static bool_t UART_RX1_predicate() { // Если байт принят вернем истину
    if(UART_RECEIV_IS_READY(1)) return TRUE;
    return FALSE;
}
static void UART_RX1_to_buff() { // Запись принятого байта в буфер
    UART_RECEIV_DISABLE(1); // Выключаем UART
    PutToBackQ(&UART_RX_DATA[1].Data, UART_RX_DATA[1].buffer); // Записывае байт в буфер
}
static bool_t UART_TX1_predicate() { // Если байт принят вернем истину
    if(UART_TRANS_IS_READY(1)) return TRUE;
    return FALSE;
}
static void UART_TX1_to_buff() {  // Запись принятого байта в буфер
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
  if(UART_RX_DATA[2].curentCount < 0) { // Если таймер выключен
    if(! READ_RX_PIN(RX_PIN, UART_RX_DATA[2].Mask) ) {  // Проверяем стартовый бит
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
    for(u08 i = 0; i < UART_NUMB; i++){
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

void enableSoftUART(bool_t txEnable, bool_t rxEnable) {
  setFlags(SOFT_UART_WORK_FLAG);
  for(u08 i = 0; i < UART_NUMB; i++){
    initProgramUartGPIO(UART_TX_DATA[i].Mask,UART_RX_DATA[i].Mask);
    if(i == 0){
      if(rxEnable) {
        CreateEvent(isStartBit0,StartReceive0);  // Регистрируем событие поиска стартового бита
        CreateEvent(UART_RX0_predicate, UART_RX0_to_buff); // Регистрируем событие сохранения принятого байта в буфер
      }
      if(txEnable) CreateEvent(UART_TX0_predicate, UART_TX0_to_buff);
    }
#if(UART_NUMB > 1)
    if(i == 1) {
      if(rxEnable) {
        CreateEvent(isStartBit1,StartReceive1);  // Регистрируем событие поиска стартового бита
        CreateEvent(UART_RX1_predicate, UART_RX1_to_buff); // Регистрируем событие сохранения принятого байта в буфер
      }
      if(txEnable) CreateEvent(UART_TX1_predicate, UART_TX1_to_buff);
    }
#endif
#if(UART_NUMB > 2)
    if(i == 2) {
      if(rxEnable) {
        CreateEvent(isStartBit2,StartReceive2);  // Регистрируем событие поиска стартового бита
        CreateEvent(UART_RX2_predicate, UART_RX2_to_buff); // Регистрируем событие сохранения принятого байта в буфер
      }
      if(txEnable) CreateEvent(UART_TX2_predicate, UART_TX2_to_buff);
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
      deInitProgramUartGPIO(UART_TX_DATA[i].Mask,UART_RX_DATA[i].Mask);
      if(i == 0){
        delEvent(isStartBit0);  // Регистрируем событие поиска стартового бита
        delEvent(UART_RX0_predicate); // Регистрируем событие сохранения принятого байта в буфер
        delEvent(UART_TX0_predicate);
      }
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
    if(UART_TX_DATA[numbUART].buffer == NULL) MaximizeErrorHandler("Can not create buffer for soft uart");
    UART_RX_DATA[numbUART].buffer = allocMem(buffRXsize);
    if(UART_RX_DATA[numbUART].buffer == NULL) MaximizeErrorHandler("Can not create buffer for soft uart");
    if(CreateQ(UART_TX_DATA[numbUART].buffer, 1, buffTXsize) != EVERYTHING_IS_OK) MaximizeErrorHandler("Data struct can not create");
    if(CreateQ(UART_RX_DATA[numbUART].buffer, 1, buffRXsize) != EVERYTHING_IS_OK) MaximizeErrorHandler("Data struct can not create");
}

void delSoftUART(const u08 numbUART){
  if(numbUART < UART_NUMB) {
      deInitProgramUartGPIO(UART_TX_DATA[numbUART].Mask, UART_RX_DATA[numbUART].Mask);
      delDataStruct(UART_TX_DATA[numbUART].buffer);
      delDataStruct(UART_RX_DATA[numbUART].buffer);
      freeMem(UART_TX_DATA[numbUART].buffer);
      freeMem(UART_RX_DATA[numbUART].buffer);
  }
  if(numbUART == 0) {
        delEvent(isStartBit0);
        delEvent(UART_RX0_predicate);
        delEvent(UART_TX0_predicate);
  }
#if UART_NUMB > 1
  if(numbUART == 1) {
        delEvent(isStartBit1);
        delEvent(UART_RX1_predicate);
        delEvent(UART_TX1_predicate);
  }
#if UART_NUMB > 2
  if(numbUART == 2) {
        delEvent(isStartBit2);
        delEvent(UART_RX2_predicate);
        delEvent(UART_TX2_predicate);
  }
#if UART_NUMB > 3
  if(numbUART == 3) {
        delEvent(isStartBit3);
        delEvent(UART_RX3_predicate);
        delEvent(UART_TX3_predicate);
  }
#endif
#endif
#endif
}

static void dataReceive(const u08 numbUART) {
  static u08 bitCount[UART_NUMB] = {0};
  static u08 result[UART_NUMB] = {0};
  if(UART_RX_DATA[numbUART].curentCount < 0) { result[numbUART] = bitCount[numbUART] = 0; return; }//UART выключен
  if(UART_RX_DATA[numbUART].curentCount > 1) {UART_RX_DATA[numbUART].curentCount--; return;}
  if(bitCount[numbUART] != DATA_BITS){
      if( READ_RX_PIN(RX_PIN, UART_RX_DATA[numbUART].Mask) ) result[numbUART] |= 1<<bitCount[numbUART];
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
  static u08 bitCount[UART_NUMB] = {0};
  if(UART_TX_DATA[numbUART].curentCount > 1) {UART_TX_DATA[numbUART].curentCount--; return;}
  if(UART_TX_DATA[numbUART].curentCount < 0) {bitCount[numbUART] = 0; return;}// Передача отключена
  if(bitCount[numbUART] < DATA_BITS+1) {
    UART_TX_DATA[numbUART].curentCount = UART_TX_DATA[numbUART].Baud;
    if(bitCount[numbUART]) {
      bitCount[numbUART]++;   // Увеличиваем счетчик переданых бит
      if(UART_TX_DATA[numbUART].Data & 0x01) WRITE_TX_PIN(TX_PORT, UART_TX_DATA[numbUART].Mask); // Передаем младший бит
      else CLEAR_TX_PIN(TX_PORT,UART_TX_DATA[numbUART].Mask);   // Передаем младший бит
      UART_TX_DATA[numbUART].Data >>= 1;              // Смещаем регистр данных
      return;
    } else {
        bitCount[numbUART] = 1; // Счетчик переданых бит
        CLEAR_TX_PIN(TX_PORT,UART_TX_DATA[numbUART].Mask); //  Ножку контроллера в низкий уровень (Старт бит)
        return;
    }
   }
   //Здесь мы если байт уже передан
   bitCount[numbUART]++;  //СТОП БИТОВ может быть несколько
   WRITE_TX_PIN(TX_PORT, UART_TX_DATA[numbUART].Mask); //  Ножку контроллера в высокий уровень (Стоп бит) байт отправлен
   if(bitCount[numbUART] > DATA_BITS+STOP_BITS+1) UART_TRANS_FINISH(numbUART);// Выключаем передачу
}

static void addByte(const u08 numbUART,const u08 byte) {
    if(UART_TRANS_IS_DISABLE(numbUART)) {
        DISABLE_UART_TIMER_ISR;
        UART_TX_DATA[numbUART].Data = byte;
        UART_TX_DATA[numbUART].curentCount = 0;
        ENABLE_UART_TIMER_ISR;
    }
    else{
        PutToBackQ(&byte, UART_TX_DATA[numbUART].buffer);
    }
}

void sendUART_byte(const u08 numbUART,const u08 U_data){ // Функция передачи данных
    if(numbUART >= UART_NUMB) return;
    addByte(numbUART,U_data);
}

void sendUART_str(const u08 numbUART, const string_t U_data){
  if(numbUART >= UART_NUMB) return;
  u08 i = 0;
  while(U_data[i] != '\0'){
    addByte(numbUART, (u08)U_data[i]);
    i++;
  }
}

void sendUART_array(const u08 numbUART, const BaseSize_t size, const unsigned char* U_data){
  if(numbUART >= UART_NUMB) return;
  for(u08 i = 0;i<size;i++){
    addByte(numbUART, U_data[i]);
  }
}

void clearSoftUartRxBuffer(const unsigned char numbUART){
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

void UARTTimerISR(){// Само прерывание
    for(register unsigned char i = 0; i<UART_NUMB; i++){
        dataReceive(i);
        dataTransmit(i);
    }
}

#endif //USE_SOFT_UART
