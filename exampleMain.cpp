#ifdef __cplusplus
extern "C" {
#endif
#include "TaskMngr.h"
#include "PlatformSpecific.h"
#include "logging.h"
#include "MyString.h"
#ifdef __cplusplus
}
#endif
#include <iostream>

void testAddr(BaseSize_t count, BaseParam_t res) {
	std::printf("%p, %p\n",res, &res);
	SetTimerTask(testAddr,0,res,10*TICK_PER_SECOND);
}

byte_ptr heapStart;

void SimpleTask(BaseSize_t count, byte_ptr** res){
	count++;
	res = leakControlAlloc(10, res);
	if(res != NULL) {
		if(**res != NULL) {
			for(u08 i = 0; i<10; i++) {
				(**res)[i] = count*i;
			}
		}
		else {
			writeLogStr("Error when try allocate memory");
		}
	}
	SetTimerTask((TaskMng)SimpleTask,count,(BaseParam_t)res,TICK_PER_SECOND*10);
}

void showAllMemmory(){
	char data[HEAP_SIZE*3];
	strClear(data);
	for(u16 i=0; i<HEAP_SIZE; i++) {
		char temp[5];
		toStringDec(heapStart[i],temp);
		strCat(data,temp);
		strCat(data," ");
	}
	writeLogStr(data);
}

int main() {
	initFemtOS();
	heapStart = allocMem(1);
	for(s16 i=-1; i<HEAP_SIZE; i++) {
		heapStart[i] = 0;
	}
//	SetTask(testAddr,0,(BaseParam_t)112);
	SetCycleTask(TICK_PER_SECOND*15, defragmentation, TRUE);
	SetTask((TaskMng)SimpleTask, 0, NULL);
	SetCycleTask(TICK_PER_SECOND*10, showAllMemmory,TRUE);
	runFemtOS();
	return 0;
}
