/*
 * logging.c
 *
 *  Created on: 19 жовт. 2017 р.
 *      Author: oleksiy.khanin
 */

#include "TaskMngr.h"
#include "MyString.h"
#include "UART2.h"

void enableLogging(void) {
	enableUART2();
}

void disableLogging(void){
	disableUART2();
}

void writeLogStr(const string_t c_str){
	sendCOM2_buf(0, (u08*)c_str);
	sendUART2_buf((u08)'\n');
}

void writeLogTempString(string_t tempStr){
	u08 size =  strSize(tempStr);
	for(u08 i = 0; i<size; i++) sendUART2_buf((u08)tempStr[i]);
	sendUART2_buf((u08)'\n');
}

void writeLogFloat(float data) {
	char tempStr[12];
	doubleToString(data,tempStr,2);
	writeLogTempString(tempStr);
}

void writeLogU32(u32 data) {
	char tempStr[14];
	toStringDec(data,tempStr);
	writeLogTempString(tempStr);
}

void writeSymb(char symb) {
	sendUART2_buf((u08)symb);
}

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
