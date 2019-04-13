/*
 * PlatformSpecificSceleton.h
 *
 *  Created on: 11 февр. 2019 г.
 *      Author: blabu
 */

#ifndef FEMTOX_PLATFORMSPECIFICSCELETON_H_
#define FEMTOX_PLATFORMSPECIFICSCELETON_H_

#include "FemtoxTypes.h"

#define TICK_PER_SECOND 128 /*Колличество тиков в секунду*/

unlock_t lock(const void*const resourceId);

void initWatchDog(void);
void resetWatchDog(void);

void _init_Timer(void);

void _initTimerSoftUart();
void initProgramUartGPIO(unsigned short TX_MASK, unsigned short RX_MASK);
void _deInitTimerSoftUart();
void deInitProgramUartGPIO(unsigned short TX_MASK, unsigned short RX_MASK);


#endif /* FEMTOX_PLATFORMSPECIFICSCELETON_H_ */
