/*
 * types.h
 *
 *  Created on: 18 лип. 2018 р.
 *      Author: Admin
 */

#ifndef FEMTOXTYPES_H_
#define FEMTOXTYPES_H_

#include "platform.h"
#ifdef ARM_STM32
#include "FemtoxTypes_ARM.h"
#endif
#ifdef _X86
#include "FemtoxTypes_X86.h"
#endif
#ifdef MSP430
#include "FemtoxTypes_MSP430.h"
#endif

#endif /* FEMTOXTYPES_H_ */
