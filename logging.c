/*
MIT License

Copyright (c) 2017 Oleksiy Khanin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 * */

/*
 * logging.c
 *
 *  Created on: 19 жовт. 2017 р.
 *      Author: oleksiy.khanin
 */

#include "PlatformSpecific.h" // for _X86
#include "MyString.h"
#include "logging.h"
#include "TaskMngr.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ENABLE_LOGGING
void enableLogging(void) {}
void disableLogging(void){}

#ifndef ALLOC_MEM_LARGE
void writeLogByteArray(u08 sizeBytes, byte_ptr array){}
#else
void writeLogByteArray(BaseSize_t sizeBytes, byte_ptr array){}
#endif
void disableLogLevel(string_t level) {}

void writeLogWithStr(const string_t c_str, u32 n){}

void writeLogStr(const string_t c_str){}

void writeLog2Str(const string_t c_str1, const string_t c_str2){}

void writeLog3Str(const string_t c_str1, const string_t c_str2, const string_t c_str3){}

void writeLog4Str(const string_t c_str1, const string_t c_str2, const string_t c_str3, const string_t c_str4){}

void writeLogTempString(string_t tempStr){}

void writeLogFloat(float data){}

void writeLogU32(u32 data){}

void writeSymb(char symb) {}
#endif

#ifdef ENABLE_LOGGING

#ifdef _X86

#include <stdio.h>
#include "TaskMngr.h"

#define LOCAL_MUTEX 1<<7

#ifdef __unix__
#define fprintf_s fprintf
#endif

//#define TO_FILE

void enableUART2(u32 baud) {}

void disableUART2() {}

#ifdef TO_FILE
FILE* file;
#define F_OPEN(_file, _filename , _flags)  fopen_s( (FILE**)(_file), (char const*)(_filename), (char const*)(_flags))
#else
#define file stdout
#define F_OPEN(_file, _filename, _flags) ;
#endif

void enableLogging() {
    F_OPEN(&file, (string_t) "log.txt", (string_t) "wt");
}

static void sendCOM2_buf(u08 size, byte_ptr data) {
    GET_MUTEX(LOCAL_MUTEX, sendCOM2_buf, size, data);
    if (size == 0) fprintf_s(file, "%s", data);
    else {
        for (u08 i = 0; i < size; i++) {
            fprintf_s(file, "%x ", data[i]);
        }
    }
    fflush(file);
    FREE_MUTEX(LOCAL_MUTEX);
}

static void sendUART2_buf(u08 c) {
    GET_MUTEX(LOCAL_MUTEX, sendUART2_buf, c, NULL);
    fprintf_s(file, "%c", c);
    fflush(file);
    FREE_MUTEX(LOCAL_MUTEX);
}

#endif

#ifdef ARM_STM32
#include "UART2.h"
static u08 countEnableLogging = 0;
void enableLogging(void) {
    if(countEnableLogging > 0) {
        countEnableLogging++;
        return;
    }
    countEnableLogging = 1;
#ifndef _X86
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_SET);
#endif// _X86
    enableUART2(57600);
}

void disableLogging(void){
    if(countEnableLogging > 0) countEnableLogging--;//---------------------------------------
    if(!countEnableLogging) {
        disableUART2();
#ifndef _X86
//		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_RESET);
#endif// _X86
    }
}
#endif

#ifdef SOFT_UART_FOR_LOGGING
static void enableUART2(u32 i) {enableSoftUART(TRUE,FALSE);}
static void disableUART2(){disableSoftUART();}
static void sendCOM2_buf(u08 u, byte_ptr buf){ sendUART_str(0,(string_t)buf); }
static void sendUART2_buf(u08 byte) {sendUART_byte(0, byte);}
#endif

#ifdef MSP430
static void sendCOM3_buf(u08 u, byte_ptr buf){}
static void sendUART3_buf(u08 byte) {}
#endif

static string_t disableLavel = NULL;

void disableLogLevel(string_t level) {
    disableLavel = level;
}

void writeLogWithStr(const string_t c_str, u32 n) {
    char str[50];
    if (str1_str2(disableLavel, c_str)) return;
    u08 size = strSize(c_str);
    if (size > 50) {
        writeLogStr("Error: too long string");
        return;
    }
    strClear(str);
    strCat(str, c_str);
    strCat(str, " ");
    toStringDec((s64) n, (str + size + 1));
    writeLogTempString(str);
}

void writeLogStr(const string_t c_str) {
    if (str1_str2(disableLavel, c_str)) return;
    sendCOM2_buf(0, (byte_ptr) c_str);
    sendCOM2_buf(0, (byte_ptr) "\r\n");
}

void writeLog2Str(const string_t c_str1, const string_t c_str2) {
    if (str1_str2(disableLavel, c_str1)) return;
    sendCOM2_buf(0, (byte_ptr) c_str1);
    sendCOM2_buf(0, (byte_ptr) c_str2);
    sendCOM2_buf(0, (byte_ptr) "\r\n");
}

void writeLog3Str(const string_t c_str1, const string_t c_str2, const string_t c_str3) {
    if (str1_str2(disableLavel, c_str1)) return;
    sendCOM2_buf(0, (byte_ptr) c_str1);
    sendCOM2_buf(0, (byte_ptr) c_str2);
    sendCOM2_buf(0, (byte_ptr) c_str3);
    sendCOM2_buf(0, (byte_ptr) "\r\n");
}

void writeLog4Str(const string_t c_str1, const string_t c_str2, const string_t c_str3, const string_t c_str4) {
    if (str1_str2(disableLavel, c_str1)) return;
    sendCOM2_buf(0, (byte_ptr) c_str1);
    sendCOM2_buf(0, (byte_ptr) c_str2);
    sendCOM2_buf(0, (byte_ptr) c_str3);
    sendCOM2_buf(0, (byte_ptr) c_str4);
    sendCOM2_buf(0, (byte_ptr) "\r\n");
}

void writeLogTempString(const string_t tempStr) {
    if (str1_str2(disableLavel, tempStr)) return;
    u08 size = strSize(tempStr);
    for (u08 i = 0; i < size; i++) sendUART2_buf(tempStr[i]);
    sendCOM2_buf(0, (byte_ptr) "\r\n");
}

void writeLogFloat(float data) {
    char tempStr[10];
    doubleToString(data, tempStr, 2);
    writeLogTempString(tempStr);
}

void writeLogU32(u32 data) {
    char tempStr[12];
    toStringDec(data, tempStr);
    writeLogTempString(tempStr);
}

void writeSymb(char symb) {
    sendUART2_buf((u08) symb);
}

#ifdef ALLOC_MEM
void writeLogByteArray(u08 sizeBytes, byte_ptr array){
    static string_t str = NULL;
    u08 totalSize = sizeBytes*2 + sizeBytes + 1;
    if(totalSize > getAllocateMemmorySize((byte_ptr)(str))) {
        freeMem((byte_ptr)str);
        str = (string_t)allocMem(totalSize); // Выделяем память под строку + под пробелы между символами + байт конца
    }
    if(str == NULL){
        writeLogStr("mem err in logging");
        return;
    }
    u08 poz = 0;
    for(u08 i = 0; i<sizeBytes; i++) {
        toString(1,array[i],&str[poz]);
        poz=strSize(str);
        str[poz] = ' ';
        poz++;
    }
    str[poz] = '\0';
    writeLogStr(str);
}
#endif // ALLOC_MEM
#ifdef ALLOC_MEM_LARGE

void writeLogByteArray(BaseSize_t sizeBytes, byte_ptr array) {
    static string_t str = NULL;
    BaseSize_t totalSize = sizeBytes * 2 + sizeBytes + 1;
    if (totalSize > getAllocateMemmorySize((byte_ptr) (str))) {
        freeMem((byte_ptr) str);
        str = (string_t) allocMem(totalSize); // Выделяем память под строку + под пробелы между символами + байт конца
    }
    if (str == NULL) {
        writeLogStr("mem err in logging");
        return;
    }
    BaseSize_t poz = 0;
    for (BaseSize_t i = 0; i < sizeBytes; i++) {
        toString(1, array[i], &str[poz]);
        poz = strSize(str);
        str[poz] = ' ';
        poz++;
    }
    str[poz] = '\0';
    writeLogStr(str);
}

#endif // ALLOC_MEM_LARGE

#endif // ENABLE_LOGGING

#ifdef __cplusplus
}
#endif