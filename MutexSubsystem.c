/*
 * MutexSubsystem.c
 *
 *  Created on: 28 квіт. 2017 р.
 *      Author: oleksiy.khanin
 */

#include "TaskMngr.h"
#include "PlatformSpecific.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MUTEX_ENABLE
volatile static u08  MyMutex = 0; // 8 - возможных мьютексов

bool_t getMutex(const u08 mutexNumb, TaskMng TPTR, BaseSize_t n, BaseParam_t data)
{
    if(mutexNumb >= 8) return FALSE;// Если номер мьютекса больше возможного варианта выходим из функции
    if(MyMutex & (1<<mutexNumb)) // Если мьютекс с таким номером захвачен
    {
        SetTimerTask(TPTR, n, data, TIME_DELAY_IF_BUSY); // Попытаем счастья позже
        return TRUE; // Мьютекс уже захвачен кем-то возвращаем результат
    }
    register bool_t flag_int = FALSE;
    // Здесь окажемся если мьютекс свободен
    if(INTERRUPT_STATUS)
    {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    MyMutex |= 1<<mutexNumb;
    if(flag_int) INTERRUPT_ENABLE;
    return FALSE; // Все впорядке мьютекс успешно захвачен этой функцией.
}

bool_t freeMutex(const u08 mutexNumb)
{
    if(mutexNumb >= 8) return FALSE;// Если номер мьютекса больше возможного варианта выходим из функции
    register bool_t flag_int = FALSE;
    if(INTERRUPT_STATUS)
    {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    MyMutex &= ~(1<<mutexNumb);
    if(flag_int) INTERRUPT_ENABLE;
    return TRUE;
}

#endif //MUTEX_ENABLE

#ifdef __cplusplus
}
#endif
