#ifndef PROGRAMM_UART_H
#define PROGRAMM_UART_H

#include "TaskMngr.h"

#ifdef USE_SOFT_UART

#define SOFT_UART_WORK_FLAG 1<<2

#define UART_NUMB 1    /*Колличество программных ЮАРТов*/

#define BAUD_150   127
#define BAUD_300   64
#define BAUD_600   32
#define BAUD_1200  16
#define BAUD_2400  8
#define BAUD_4800  4
#define BAUD_9600  2

#define DATA_BITS  8   /*Количество бит данных в посылке*/

/*
 *  Настраиваем прерывания по достижению события совпадения каждые 26 мкс
 *  Для организации основных скоростей UART таймер работает с постоянной частотой прервыний
 *  Например 9600 бод/с = 1/(BAUD_9600*26мкс)
*/
void initSoftUART();
void enableSoftUART(bool_t txEnable, bool_t rxEnable);
void disableSoftUART();
void CreateSoftUART(const BaseSize_t buffTXsize, const BaseSize_t buffRXsize, const s08 BAUD,
                    const u08 numbUART, const u08 TXpinNumber, const u08 RXpinNumber);
void sendUART_byte(const u08 numbUART, const u08 U_data);
u08 readUART_byte(u08 numbUART);
BaseSize_t readUART_array(const u08 numbUART, const BaseSize_t size, const unsigned char* U_data); // Вернет количество прочитанного
void delSoftUART(const u08 numbUART);
void clearSoftUartRxBuffer(const u08 numbUART);
void sendUART_str(const u08 numbUART, const string_t U_data);
void sendUART_array(const u08 numbUART, const BaseSize_t size, const unsigned char* U_data);
void UARTTimerISR(); // Само прерывание
#endif //USE_SOFT_UART
#endif //PROGRAMM_UART_H
