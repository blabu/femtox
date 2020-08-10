/*
 * PlatformSpecific.h
 *
 *  Created on: 27 жовт. 2017 р.
 *      Author: oleksiy.khanin
 */

//#ifndef PLATFORMSPECIFIC_H_
//#define PLATFORMSPECIFIC_H_

#include "platform.h"

#ifdef MSP430
#include "PlatformSpecificMSP430.h"
#elif ARM_STM32
#include "PlatformSpecificARM.h"
#elif _X86
#include "PlatformSpecificX86.h"
#elif ESP32
#include "PlatformSpecificESP32.h"
#else
#error "Undefined platform"
#include "PlatformSpecificSceleton.h"
#endif

//#endif /* PLATFORMSPECIFIC_H_ */
