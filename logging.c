/*
 * logging.c
 *
 *  Created on: 19 жовт. 2017 р.
 *      Author: oleksiy.khanin
 */

#include "TaskMngr.h"
#include "MyString.h"
//#include "UART2.h"
#include <stdio.h>

void enableLogging() {
//	enableUART2();
}

void disableLogging(){
	//disableUART2();
}

void writeLogStr(const string_t c_str){
//	sendCOM2_buf(0, (u08*)c_str);
//	sendUART2_buf((u08)'\n');
    printf("%s\n",c_str);
}

void writeLogTempString(string_t tempStr){
	u08 size =  strSize(tempStr);
//	for(u08 i = 0; i<size; i++) sendUART2_buf((u08)tempStr[i]);
//	sendUART2_buf((u08)'\n');
    printf("%s, %d\n",tempStr, size);
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
