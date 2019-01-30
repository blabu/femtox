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
volatile static mutexType MyMutex = 0;

// TRUE - Если мьютекс захватить НЕ УДАЛОСЬ
bool_t tryGetMutex(const mutexType mutexNumb) {
    if(mutexNumb >= MUTEX_SIZE) return FALSE;// Если номер мьютекса больше возможного варианта выходим из функции
    if(MyMutex & (1<<mutexNumb)) // Если мьютекс с таким номером захвачен
    {
        return TRUE; // Мьютекс уже захвачен кем-то возвращаем результат
    }
    // Здесь окажемся если мьютекс свободен
    unlock_t unlck = lock(&MyMutex);
    MyMutex |= 1<<mutexNumb;
    unlck(&MyMutex);
    return FALSE; // Все впорядке мьютекс успешно захвачен этой функцией.
}

#include "logging.h"
// TRUE - Если мьютекс захватить НЕ УДАЛОСЬ
bool_t getMutex(const mutexType mutexNumb, const TaskMng TPTR, const BaseSize_t n, const BaseParam_t data) {
    if(mutexNumb >= MUTEX_SIZE) return FALSE;// Если номер мьютекса больше возможного варианта выходим из функции
    unlock_t unlck = lock(&MyMutex);
    if(MyMutex & (1<<mutexNumb)) { // Если мьютекс с таким номером захвачен
    	SetTimerTask(TPTR, n, data, 2); // Попытаем счастья позже
    	unlck(&MyMutex);
        return TRUE; // Мьютекс уже захвачен кем-то возвращаем результат
    }
    // Здесь окажемся если мьютекс свободен
    MyMutex |= 1<<mutexNumb;
    unlck(&MyMutex);
    return FALSE; // Все впорядке мьютекс успешно захвачен этой функцией.
}

void freeMutex(const mutexType mutexNumb) {
    if(mutexNumb >= MUTEX_SIZE) return;// Если номер мьютекса больше возможного варианта выходим из функции
    unlock_t unlck = lock(&MyMutex);
    MyMutex &= ~(1<<mutexNumb);
    unlck(&MyMutex);
}

#endif //MUTEX_ENABLE

#ifdef __cplusplus
}
#endif
