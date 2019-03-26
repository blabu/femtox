/*
 * ch.h
 *
 *  Created on: 25 мар. 2019 г.
 *      Author: blabu
 */

#ifndef CH_H_
#define CH_H_

#include "TaskMngr.h"
#include <stdint.h>

/**
 * @brief   Generic 'false' preprocessor boolean constant.
 * @note    It is meant to be used in configuration files as switch.
 */
#if !defined(FALSE) || defined(__DOXYGEN__)
#define FALSE                   0
#endif

/**
 * @brief   Generic 'true' preprocessor boolean constant.
 * @note    It is meant to be used in configuration files as switch.
 */
#if !defined(TRUE) || defined(__DOXYGEN__)
#define TRUE                    1
#endif

#ifndef bool
#define bool bool_t
#endif

#ifndef false
#define false FALSE
#endif

#ifndef true
#define true TRUE
#endif

#include "ports/ARMCMx/compilers/GCC/chtypes.h"
#include "ports/ARMCMx/chcore.h" // Platform dependent part of the thread_t structure


#endif /* CH_H_ */
