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
volatile static globalFlags_t GlobalFlags = 0;

void setFlags(globalFlags_t flagMask) {
	const unlock_t unlock = lock((void*)(&GlobalFlags));
	GlobalFlags |= flagMask;
	unlock((void*)(&GlobalFlags));
}

void clearFlags(globalFlags_t flagMask) {
	const unlock_t unlock = lock((void*)(&GlobalFlags));
	GlobalFlags &= ~flagMask;
	unlock((void*)(&GlobalFlags));
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
