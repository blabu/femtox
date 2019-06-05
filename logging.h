/*
 * logging.h
 *
 *  Created on: 19 жовт. 2017 р.
 *      Author: oleksiy.khanin
 */

#ifndef LOGGING_H_
#define LOGGING_H_

#include "FemtoxTypes.h"

void enableLogging(void);
void disableLogging(void);
void disableLogLevel(string_t level); // Фильтр для исключения каких-либо логов
void writeLogWhithStr(const string_t c_str, u32 n);
void writeLogTempString(const string_t tempStr);
void writeLogStr(const string_t c_str);
void writeLogFloat(float data);
void writeLogU32(u32 data);
void writeSymb(char symb);
#ifdef ALLOC_MEM
void writeLogByteArray(u08 sizeBytes, byte_ptr array);
#endif
#ifdef ALLOC_MEM_LARGE
void writeLogByteArray(BaseSize_t sizeBytes, byte_ptr array);
#endif
#endif /* LOGGING_H_ */