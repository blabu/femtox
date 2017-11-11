/*
 * logging.h
 *
 *  Created on: 19 жовт. 2017 р.
 *      Author: oleksiy.khanin
 */

#ifndef LOGGING_H_
#define LOGGING_H_

#include "TaskMngr.h"

void enableLogging();
void disableLogging();
void writeLogTempString(string_t tempStr);
void writeLogStr(string_t c_str);
void writeLogFloat(float data);
void writeLogU32(u32 data);
void writeSymb(char symb);
void writeLogByteArray(u08 sizeBytes, byte_ptr array);

#endif /* LOGGING_H_ */