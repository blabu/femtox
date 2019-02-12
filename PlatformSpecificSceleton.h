/*
 * PlatformSpecificSceleton.h
 *
 *  Created on: 11 февр. 2019 г.
 *      Author: blabu
 */

#ifndef FEMTOX_PLATFORMSPECIFICSCELETON_H_
#define FEMTOX_PLATFORMSPECIFICSCELETON_H_

#define TICK_PER_SECOND 1UL
unlock_t lock(const void*const resourceId);
void initWatchDog();
void resetWatchDog(void);

#endif /* FEMTOX_PLATFORMSPECIFICSCELETON_H_ */
