/*
 * logging.c
 *
 *  Created on: 19 жовт. 2017 р.
 *      Author: oleksiy.khanin
 */

#include "PlatformSpecific.h"
#include "MyString.h"
#include "logging.h"

#ifdef _X86
#include <stdio.h>
void enableUART2() {}
void disableUART2() {}
static void sendCOM2_buf(u08 size, byte_ptr data) {
	if(size == 0) printf("%s\n", data);
	else {
		for(u08 i = 0; i<size; i++) {
			printf("%x ",data[i]);
		}
	}
}

static void sendUART2_buf(u08 c) {
	printf("%c",c);
}
#else
#include "UART2.h"
/*
#include "ProgrammUART_MSP430.h"
void enableUART2(u32 i) {enableSoftUART(TRUE,FALSE);}
void disableUART2(){disableSoftUART();}
void sendCOM2_buf(u08 u, byte_ptr buf){ sendUART_str(0,(string_t)buf); }
void sendUART2_buf(u08 byte) {sendUART_byte(0, byte);}
*/
#endif

static string_t disableLavel = NULL;

void enableLogging(void) {
	enableUART2(115200);
}

void disableLogging(void){
	disableUART2();
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
	sendCOM2_buf(0, (byte_ptr)c_str);
	sendCOM2_buf(0,(byte_ptr)"\r\n");
}

void writeLogTempString(string_t tempStr){
	if(str1_str2(disableLavel,tempStr)) return;
	u08 size =  strSize(tempStr);
	for(u08 i = 0; i<size; i++) sendUART2_buf(tempStr[i]);
	sendCOM2_buf(0,(byte_ptr)"\r\n");
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
	sendUART2_buf((u08)symb);
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
