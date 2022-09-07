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

#include <String.h>
#include "PlatformSpecific.h" // for _X86
#include "TaskMngr.h"
#include "logging.h"


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

u32 sizeRx3Buffer() {return 0;}
void* ReceiveUART3NewPackageLabel = (void*)sizeRx1Buffer;
void readBufUART3(BaseSize_t size, byte_ptr data) {}
void setReceiveTimeoutUART3(u16 time) {}


#define LOCAL_MUTEX 1<<7

#ifdef __unix__
#define fprintf_s fprintf
#endif

//#define TO_FILE
void enableUART3(u32 baud) {}

void disableUART3() {}

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

static void sendCOM3_buf(u08 size, byte_ptr data) {
    GET_MUTEX(LOCAL_MUTEX, sendCOM3_buf, size, data);
    if (size == 0) fprintf_s(file, "%s", data);
    else {
        for (u08 i = 0; i < size; i++) {
            fprintf_s(file, "%x ", data[i]);
        }
    }
    fflush(file);
    FREE_MUTEX(LOCAL_MUTEX);
}

static void sendUART3_buf(u08 c) {
    GET_MUTEX(LOCAL_MUTEX, sendUART3_buf, c, NULL);
    fprintf_s(file, "%c", c);
    fflush(file);
    FREE_MUTEX(LOCAL_MUTEX);
}

#endif

#ifdef ARM_STM32
#include <usbd_cdc_if.h>

#define ReceiveConsoleBuff ReceiveUSBPackageLabel
#define readConsoleBuff readUSB
#define setReceiveTimeoutConsole setReceiveTimeoutUSB
#define _sendData writeUSB
#define _sendByte writeSymbUSB
#define _clearData clearUSB
#define _enableData(baud)
#define _disableData()

static u08 countEnableLogging = 0;

#ifdef SIGNALS_TASK
static void readCMD(BaseSize_t count, BaseParam_t arg);
#endif

void enableLogging(void) {
    if(countEnableLogging > 0) {
        countEnableLogging++;
        writeLogStr("LOG: Fake enable logging");
        return;
    }
    countEnableLogging = 1;
    _enableData(115200);
    setReceiveTimeoutConsole(TICK_PER_SECOND>>1);
    writeLogStr("LOG: Enable logging");
#ifdef SIGNALS_TASK
    connectTaskToSignal(readCMD, ReceiveConsoleBuff);
#endif
}

void disableLogging(void){
    if(countEnableLogging > 0) countEnableLogging--;//---------------------------------------
    if(!countEnableLogging) {
    	_disableData();
    } else {
    	writeLogStr("LOG: Fake disable logging");
    }
}
#endif

#ifdef SOFT_UART_FOR_LOGGING
static void enableUART3(u32 i) {enableSoftUART(TRUE,FALSE);}
static void disableUART3(){disableSoftUART();}
static void sendCOM3_buf(u08 u, byte_ptr buf){ sendUART_str(0,(string_t)buf); }
static void sendUART3_buf(u08 byte) {sendUART_byte(0, byte);}
#endif

#ifdef MSP430
static void sendCOM3_buf(u08 u, byte_ptr buf){}
static void sendUART3_buf(u08 byte) {}
#endif

static string_t disableLavel = NULL;

void disableLogLevel(string_t level) {
	if(level != NULL) {
		writeLog2Str((string_t)"LOG: Disable log level ", level);
	}
	disableLavel = level;
}

void writeLogWithStr(const string_t c_str, u32 n) {
    char str[20];
    if (str1_str2(disableLavel, c_str)) return;
    strClear(str);
    toStringDec((s64)n, str);
    _sendData(0, (byte_ptr)c_str);
    u08 size = strSize(str);
    for (u08 i = 0; i < size; i++) _sendByte(str[i]);
    _sendByte('/r');
    _sendByte('/n');
}

void writeLogStr(const string_t c_str) {
    if (str1_str2(disableLavel, c_str)) return;
    changeCallBackLabel(writeLogStr,_sendData);
    _sendData(0, (byte_ptr) c_str);
    _sendData(0, (byte_ptr) "\r\n");
}

void writeLog2Str(const string_t c_str1, const string_t c_str2) {
    if (str1_str2(disableLavel, c_str1)) return;
    changeCallBackLabel(writeLog2Str,_sendData);
    _sendData(0, (byte_ptr) c_str1);
    _sendByte(' ');
    _sendData(0, (byte_ptr) c_str2);
    _sendData(0, (byte_ptr) "\r\n");
}

void writeLog3Str(const string_t c_str1, const string_t c_str2, const string_t c_str3) {
    if (str1_str2(disableLavel, c_str1)) return;
    changeCallBackLabel(writeLog3Str,_sendData);
    _sendData(0, (byte_ptr) c_str1);
    _sendByte(' ');
    _sendData(0, (byte_ptr) c_str2);
    _sendByte(' ');
    _sendData(0, (byte_ptr) c_str3);
    _sendData(0, (byte_ptr) "\r\n");
}

void writeLog4Str(const string_t c_str1, const string_t c_str2, const string_t c_str3, const string_t c_str4) {
    if (str1_str2(disableLavel, c_str1)) return;
    changeCallBackLabel(writeLog4Str,_sendData);
    _sendData(0, (byte_ptr) c_str1);
    _sendByte(' ');
    _sendData(0, (byte_ptr) c_str2);
    _sendByte(' ');
    _sendData(0, (byte_ptr) c_str3);
    _sendByte(' ');
    _sendData(0, (byte_ptr) c_str4);
    _sendData(0, (byte_ptr) "\r\n");
}

void writeLogTempString(const string_t tempStr) {
    if (str1_str2(disableLavel, tempStr)) return;
    u08 size = strSize(tempStr);
    for (u08 i = 0; i < size; i++) _sendByte(tempStr[i]);
    changeCallBackLabel(writeLogTempString,_sendData);
    _sendData(0, (byte_ptr) "\r\n");
}

void writeLogFloat(float data) {
    char tempStr[10];
    doubleToString(data, tempStr, 2);
    writeLogTempString(tempStr);
}

void writeLogU32(u32 data) {
    char tempStr[14];
    toStringDec(data, tempStr);
    writeLogTempString(tempStr);
}

void writeSymb(char symb) {
	_sendByte((u08) symb);
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
        writeLogStr("ERROR: mem err in logging");
		#ifdef CALL_BACK_TASK
        execCallBack(writeLogByteArray);
		#endif
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
	#ifdef CALL_BACK_TASK
	changeCallBackLabel(writeLogByteArray, writeLogStr);
	#endif
    writeLogStr(str);
}
#endif // ALLOC_MEM
#ifdef ALLOC_MEM_LARGE
void writeLogByteArray(BaseSize_t sizeBytes, byte_ptr array) {
    static string_t str = NULL;
    BaseSize_t totalSize = sizeBytes * 2 + sizeBytes + 1;
    if (totalSize > getAllocateMemmorySize((byte_ptr) (str))) {
        freeMem((byte_ptr) str);
        str = (string_t) allocMemComment(totalSize, "For byteArray log"); // Выделяем память под строку + под пробелы между символами + байт конца
    }
    if (str == NULL) {
        writeLogStr("ERROR: mem err in logging");
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

#ifdef COMMAND_TASK

static void sizeMemHandler() {
	writeLogWithStr("LOG: Free memory size: " , getFreeMemmorySize());
	writeLogWithStr("LOG: Heap size is: " , HEAP_SIZE);
	execCallBack(sizeMemHandler);
}

static void tasksHandler(){
	writeLogWithStr("LOG: Free position in task list ", getFreePositionForTask());
	writeLogWithStr("LOG: Free position in timers list ", getFreePositionForTimerTask());
	execCallBack(tasksHandler);
}

static void timeHandler() {
	char str[18];
	strClear(str);
	const Time_t t = getAllSeconds();
	const Date_t d = getDateFromSeconds(t, TRUE);
	dateToString(str,(Date_t*)&d);
	writeLogWithStr("LOG: Current time in seconds is ", t);
	writeLogTempString(str);
	execCallBack(timeHandler);
}

static void clearScreenHandler() {
    for(u08 i = 0; i<50; i++) {
        writeSymb('\n');
        writeSymb('\r');
    }
    execCallBack(clearScreenHandler);
}

static void enableLogHandler() {
	enableLogging();
	disableLogLevel(NULL);
	execCallBack(enableLogHandler);
}

static void disableLogHandler(BaseSize_t n, string_t subcmds) {
	if(n > 1) {
		writeLogStr("ERROR: this command support only one subcommand");
		execCallBack(disableLogHandler);
		return;
	}
	if(!n) {
		writeLogStr("DEBUG: Disable logging after 1 second. Bye bye...");
		SetTimerTask((TaskMng)disableLogging,0,NULL, TICK_PER_SECOND);
		execCallBack(disableLogHandler);
		return;
	}
	string_t commandCopy = (string_t)allocMem(strSize(subcmds)+1);
	if(commandCopy != NULL) {
		strClear(commandCopy);
		strCat(commandCopy, subcmds);
		freeMem((byte_ptr)disableLavel);
		writeLog2Str("LOG: Disable log level ", commandCopy);
		disableLavel = commandCopy;
	}
	execCallBack(disableLogHandler);
}

static void setTimeHandler(BaseSize_t n, BaseParam_t timeStr) {
	if(n != 1) {
		writeLogStr("ERROR: This command should has exact one parameter \"unix timestamp\" in dec format");
		writeLogWithStr("But get ", n);
		return;
	}
	u32 time = (u32)toIntDec(timeStr);
	writeLogWithStr("DEBUG: Set current system time ", time);
	setSeconds(time);
	execCallBack(setTimeHandler);
}

static void _helpHandler(TaskMng _h, string_t cmd, string_t description) {
	writeLog3Str("LOG: ", cmd, description);
}
static void helpHandler() {
	forEachCommand(_helpHandler);
	execCallBack(helpHandler);
}

static void echoHandler(BaseSize_t n, string_t arguments) {
	for(BaseSize_t i = 0; i<n; i++) {
		writeLogStr(arguments);
		arguments += strSize(arguments)+1;
	}
	execCallBack(echoHandler);
}

static void execHandler(BaseSize_t n, string_t commands) {
	for(BaseSize_t i = 0; i<n; i++) {
		execCommand(commands);
		commands += strSize(commands)+1;
	}
	execCallBack(execHandler);
}

void cronjobWrapper(BaseSize_t n, BaseParam_t args)  {
	changeCallBackLabel(cronjobWrapper, execWithSubCommand);
	execWithSubCommand(args, n-1, args+strSize(args)+1);
}

static void cronjobHandler(BaseSize_t n, string_t args) {
	if(n < 2) {
		writeLogStr("ERROR: This command should has more then one parameter first it is delay time in seconds and second parameter is a command");
		return;
	}
	Time_t delay = (Time_t)toIntDec(args);
	SetTimerTask(cronjobWrapper, n-1, args+strSize(args)+1, delay*TICK_PER_SECOND);
	changeCallBackLabel(cronjobHandler, cronjobWrapper);
}
static void clearAllTaskHandler() {
	delAllTimerTask();
	delAllTask();
	execCallBack(clearAllTaskHandler);
}

void initStandardConsoleCommands() {
	writeLogStr("INFO: Commands are initialize. Please type \"help\" to get more information about commands ");
	addTaskCommand((TaskMng)helpHandler, "help", "Show this help message");
	addTaskCommand((TaskMng)defragmentation, "defra", "Run defragmentation heap memory");
	addTaskCommand((TaskMng)sizeMemHandler, "sizeMem", "Show free heap memory size");
	addTaskCommand((TaskMng)tasksHandler, "tasks", "Show all free position in task list, show all free position in timers list");
	addTaskCommand((TaskMng)timeHandler, "time", "Show current time");
	addTaskCommand(setTimeHandler, "setTime", "set a new current system time in seconds (unix timestamp). Example: \"setTime 1656794245\"");
	addTaskCommand((TaskMng)clearAllMemmory, "clearMem", "Clear all heap. WARNING! may be destroy application");
	addTaskCommand((TaskMng)clearAllCallBackList, "clearCallBack","Clear all call back tasks. WARNING! may be destroy application");
	addTaskCommand((TaskMng)clearAllTaskHandler, "clearTask", "Clear all task from task list and timer list (analog restart system, but more safety)");
	addTaskCommand((TaskMng)ResetFemtOS, "restart", "Restart system. Dangerous operation");
	addTaskCommand((TaskMng)clearScreenHandler, "clearScreen", "Clear log screen");
	addTaskCommand((TaskMng)enableLogHandler, "enableLog", "Enable logging, reset all log filters. Most verbosity logs");
	addTaskCommand((TaskMng)disableLogHandler, "disableLog", "Disable all logs that contains filtered word. If nothing. Disable all logs and console will be also disabled (can not inserted any command through console). Example \"disableLog LOG\"");
	addTaskCommand((TaskMng)echoHandler, "echo", "Print arguments to the console. Example: \"echo Hello world\"");
	addTaskCommand((TaskMng)execHandler, "exec", "Try exec a list of command from the argument. All command should not have arguments in this case. Example: \"exec sizeMem time tasks\"");
	addTaskCommand((TaskMng)cronjobHandler, "cronjob", "Execute any commnad (second parameter) with arguments (third and more parameter) and time delay (first parameter). Example: \"cronjob 10 echo hello world\"");
#ifdef DEBUG_CHEK_ALLOCATED_MOMORY
	addTaskCommand((TaskMng)showAllBlocks, "showAllBlocks", "show all blocks of memory allocated with associated description");
#endif
}

static void readCMD(BaseSize_t sz, BaseParam_t data) {
	static string_t command = NULL;
	freeMem((byte_ptr)command);
	command = (string_t)allocMemComment(sz+1, "For command console");
	if(command == NULL) {
		writeLogStr("ERROR: Command interface memory error");
		_clearData();
		return;
	}
    readConsoleBuff(sz, (byte_ptr)command);
    command[sz] = END_STRING;
    writeLogStr(command);
    if(execCommand(command) == NOT_FOUND_DATA_STRUCT_ERROR) {
    	writeLog3Str("ERROR: Undefined command",command,". Please type \"help\" to find out command list");
    }
}
#endif
#endif // ENABLE_LOGGING

#ifdef __cplusplus
}
#endif
