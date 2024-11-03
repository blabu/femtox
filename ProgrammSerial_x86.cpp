#include <cstdio>
#include <thread>
extern "C" {
#include "TaskMngr.h"
}

#define LOCAL_MUTEX 1<<7

#ifdef __unix__
#define fprintf_s fprintf
#endif

#ifdef TO_FILE
FILE* file;
#define F_OPEN(_file, _filename , _flags)  fopen_s( (FILE**)(_file), (char const*)(_filename), (char const*)(_flags))
#else
#define file stdout
#define F_OPEN(_file, _filename, _flags) ;
#endif

static char localRecBuf[1000];
static std::thread* serialReadThread;

extern const void*const ReceiveNewPackageLabel = (void*)localRecBuf;  // A signal is emitted when a timeout to get a new byte is passed.
static void endOfReceive() {
	execCallBack(ReceiveNewPackageLabel);
	emitSignal(ReceiveNewPackageLabel, getCurrentSizeDataStruct(localRecBuf), localRecBuf);
}
static u16 nextByteTimeout = 0;
extern "C" void setReceiveTimeoutSerial(u16 tick) { // Время ожидания следующего байта
	delTimerTask((TaskMng)endOfReceive,0,NULL);
	nextByteTimeout = tick;
}

static void __readProcess() {
    while(1) {
        int ch = getchar();
        if(ch < 0) break;
        if(nextByteTimeout && !updateTimer((TaskMng)endOfReceive,0,NULL,(Time_t)nextByteTimeout)) {
			SetTimerTask((TaskMng)endOfReceive,0,NULL,(Time_t)nextByteTimeout);
		}
        PutToBackQ((char*)&ch, localRecBuf);
    }
}

extern "C" void initSerial(BaseSize_t baud) {
    CreateDataStruct(localRecBuf, 1, 1000);
    F_OPEN(&file, (string_t) "log.txt", (string_t) "wt"); // for writing
    serialReadThread = new std::thread(__readProcess); // start read process
}

extern "C" void clearBuf() {
    clearDataStruct(localRecBuf);
}

extern "C" void readBuf(BaseSize_t sz, BaseParam_t buf) {
    BaseSize_t i = 0;
    for(; i<sz; i++) {
        if(GetFromFrontDataStruct((byte_ptr)buf+i,localRecBuf) != EVERYTHING_IS_OK) break;
    }
    for(; i<sz; i++) {
        *((byte_ptr)buf+i) = 0;
    }
}

extern "C" void sendBuf(BaseSize_t sz, byte_ptr buff) {
    GET_MUTEX(LOCAL_MUTEX, sendBuf, sz, buff);
    if (sz == 0) fprintf_s(file, "%s", buff);
    else {
        for (u08 i = 0; i < sz; i++) {
            fprintf_s(file, "%x ", u08(buff[i]));
        }
    }
    fflush(file);
    FREE_MUTEX(LOCAL_MUTEX);
}

extern "C" void sendByte(u08 c) {
    GET_MUTEX(LOCAL_MUTEX, sendByte, c, NULL);
    fprintf_s(file, "%c", c);
    fflush(file);
    FREE_MUTEX(LOCAL_MUTEX);
}