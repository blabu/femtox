#ifdef __cplusplus
extern "C" {
#endif
#include "TaskMngr.h"
#include "PlatformSpecific.h"
#include "logging.h"
#include "MyString.h"
#include "crypt.h"
#ifdef __cplusplus
}
#endif
#include <time.h>
#include <iostream>
//
void testAddr(BaseSize_t count, BaseParam_t res) {
	std::printf("%p, %p\n",res, &res);
	SetTimerTask(testAddr,0,res,10*TICK_PER_SECOND);
}
//
byte_ptr heapStart;
//
//void SimpleTask(BaseSize_t count, byte_ptr** res){
//	count++;
//	res = leakControlAlloc(10, res);
//	if(res != NULL) {
//		if(**res != NULL) {
//			for(u08 i = 0; i<10; i++) {
//				(**res)[i] = count*i;
//			}
//		}
//		else {
//			writeLogStr("Error when try allocate memory");
//		}
//	}
//	SetTimerTask((TaskMng)SimpleTask,count,(BaseParam_t)res,TICK_PER_SECOND*10);
//}
//
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



void checkRandom() {
	//writeLogU32(myRandom());
	myRandom();
}

int main() {
	initFemtOS();
	setSeconds(time(NULL));
	setSeed(getTick());
	SetCycleTask(5, checkRandom,TRUE);
	runFemtOS();
	return 0;
}
