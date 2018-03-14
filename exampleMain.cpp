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



#include <map>

std::map<u16,u16> randomNumbers;
u32 countCheck = 0;
void checkRandom() {
	countCheck++;
	u16 rand = RandomSimple() % 10;
	randomNumbers[rand] += 1;
	rand = RandomMultiply() % 10;
	randomNumbers[rand] += 1;
}

void showMap() {
	static u16 maxData = 1;
	std::cout<<"Call Random numbers: "<<countCheck<<std::endl;
	u16 KeyMax = 0;
	u16 KeyMin = randomNumbers.cbegin()->first;
	for(auto i = randomNumbers.cbegin(); i != randomNumbers.cend(); ++i) {
		u16 count = i->second;
		if(count > randomNumbers[KeyMax]) {KeyMax = i->first;}
		if(count < randomNumbers[KeyMin]) {KeyMin = i->first;}
	}
	std::cout<<"\nMax count: "<<randomNumbers[KeyMax]<<" for digit: "<<KeyMax<<std::endl;
	std::cout<<"\nMin count: "<<randomNumbers[KeyMin]<<" for digit: "<<KeyMin<<std::endl;
	std::cout<<RandomMultiply() % 6<<std::endl;
}

void showAllMap() {
	for(auto i = randomNumbers.cbegin(); i != randomNumbers.cend(); ++i) {
		std::cout<<i->first<<" "<<i->second<<std::endl;
	}
	std::cout<<std::endl;
}

int main() {
	initFemtOS();
	setSeconds(time(NULL));
	SetCycleTask(2, checkRandom,TRUE);
	SetCycleTask(TICK_PER_SECOND<<2,showAllMap,TRUE);
	runFemtOS();
	return 0;
}
