/*
 * GlobalFlagsUnit.c
 *
 *  Created on: 28 квіт. 2017 р.
 *      Author: oleksiy.khanin
 */

#include "TaskMngr.h"
#include "PlatformSpecific.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef GLOBAL_FLAGS
static globalFlags_t GlobalFlags = 0;

void setFlags(globalFlags_t flagMask) {
	unlock_t unlock = lock(&GlobalFlags);
	GlobalFlags |= flagMask;
	unlock(&GlobalFlags);
}

void clearFlags(globalFlags_t flagMask) {
	unlock_t unlock = lock(&GlobalFlags);
	GlobalFlags &= ~flagMask;
	unlock(&GlobalFlags);
}

bool_t getFlags(globalFlags_t flagMask){
	if(GlobalFlags & flagMask) return TRUE;
	return FALSE;
}

globalFlags_t getGlobalFlags(void){
	globalFlags_t result = 0;
	while(result != GlobalFlags) result = GlobalFlags;
	return result;
}
#endif

#ifdef __cplusplus
}
#endif
