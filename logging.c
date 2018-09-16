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

#ifdef _X86
#include <stdio.h>
void enableUART3() {}
void disableUART3() {}
static void sendCOM3_buf(u08 size, byte_ptr data) {
	if(size == 0) printf("%s\n", data);
	else {
		for(u08 i = 0; i<size; i++) {
			printf("%x ",data[i]);
		}
	}
	fflush(stdout);
}

static void sendUART3_buf(u08 c) {
	printf("%c",c);
	fflush(stdout);
}
#else
#ifdef USE_SOFT_UART
static void enableUART2(u32 i) {enableSoftUART(TRUE,FALSE);}
static void disableUART2(){disableSoftUART();}
static void sendCOM2_buf(u08 u, byte_ptr buf){ sendUART_str(0,(string_t)buf); }
static void sendUART2_buf(u08 byte) {sendUART_byte(0, byte);}
#elif ARM_STM32
#include "UART3.h"
#endif
#endif
#ifndef NULL
#define NULL ((void*)0)
#endif
static string_t disableLavel = NULL;

void enableLogging(void) {
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_SET);
	enableUART3(57600);
}

void disableLogging(void){
	disableUART3();
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_RESET);
}

void disableLogLevel(string_t level) {
	disableLavel = level;
}

void writeLogWhithStr(const string_t c_str, u32 n) {
	char str[50];
	if(str1_str2(disableLavel,c_str)) return;
	u08 size = strSize(c_str);
	if(size > 50) {
		writeLogStr("Error: too long string");
		return;
	}
	strClear(str); strCat(str,c_str);
	toStringDec(n,str+size);
	writeLogTempString(str);
}

void writeLogStr(const string_t c_str){
	if(str1_str2(disableLavel,c_str)) return;
	sendCOM3_buf(0, (byte_ptr)c_str);
	sendCOM3_buf(0,(byte_ptr)"\r\n");
}

void writeLogTempString(string_t tempStr){
	if(str1_str2(disableLavel,tempStr)) return;
	u08 size =  strSize(tempStr);
	for(u08 i = 0; i<size; i++) sendUART3_buf(tempStr[i]);
	sendCOM3_buf(0,(byte_ptr)"\r\n");
}

void writeLogFloat(float data) {
	char tempStr[10];
	doubleToString(data,tempStr,2);
	writeLogTempString(tempStr);
}

void writeLogU32(u32 data) {
	char tempStr[10];
	toStringDec(data,tempStr);
	writeLogTempString(tempStr);
}

void writeSymb(char symb) {
	sendUART3_buf((u08)symb);
}

#ifdef ALLOC_MEM
void writeLogByteArray(u08 sizeBytes, byte_ptr array){
	static string_t str = NULL;
	if(str != NULL) freeMem((byte_ptr)str);
	u08 totalSize = sizeBytes*2 + sizeBytes + 1;
	str = (string_t)allocMem(totalSize); // Выделяем память под строку + под пробелы между символами + байт конца
	if(str == NULL){
		writeLogStr("mem err in logging");
		return;
	}
	memSet(str,totalSize,0);
	u08 poz = 0;
	for(u08 i = 0; i<sizeBytes; i++) {
		toStringDec(array[i],&str[poz]);
		poz=strSize(str);
		str[poz] = ' ';
		poz++;
	}
	str[poz] = '\0';
	writeLogStr(str);
}
#endif
