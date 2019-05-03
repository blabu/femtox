/*
 * FemtoxConf.h
 *
 *	Конфигурация всей системы
 *
 *  Created on: 18 лип. 2018 р.
 *      Author: Admin
 */

#ifndef FEMTOXCONF_H_
#define FEMTOXCONF_H_

#include "platform.h"
#ifdef ARM_STM32
#include "FemtoxConf_ARM.h"
#endif
#ifdef _X86
#include "FemtoxConf_X86.h"
#endif
#ifdef MSP430
#include "FemtoxConf_MSP430.h"
#endif
#ifdef ESP32
#include "FemtoxConf_ESP32.h"
#endif

#endif /* FEMTOXCONF_H_ */