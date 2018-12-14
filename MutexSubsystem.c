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
    unlock_t unlock = lock(&MyMutex);
    MyMutex |= 1<<mutexNumb;
    unlock(&MyMutex);
    return FALSE; // Все впорядке мьютекс успешно захвачен этой функцией.
}

#include "logging.h"
// TRUE - Если мьютекс захватить НЕ УДАЛОСЬ
bool_t getMutex(const mutexType mutexNumb, TaskMng TPTR, BaseSize_t n, BaseParam_t data) {
    if(mutexNumb >= MUTEX_SIZE) return FALSE;// Если номер мьютекса больше возможного варианта выходим из функции
    unlock_t unlock = lock(&MyMutex);
    if(MyMutex & (1<<mutexNumb)) { // Если мьютекс с таким номером захвачен
    	SetTimerTask(TPTR, n, data, 2); // Попытаем счастья позже
    	unlock(&MyMutex);
        return TRUE; // Мьютекс уже захвачен кем-то возвращаем результат
    }
    // Здесь окажемся если мьютекс свободен
    MyMutex |= 1<<mutexNumb;
    unlock(&MyMutex);
    return FALSE; // Все впорядке мьютекс успешно захвачен этой функцией.
}

void freeMutex(const mutexType mutexNumb) {
    if(mutexNumb >= MUTEX_SIZE) return;// Если номер мьютекса больше возможного варианта выходим из функции
    unlock_t unlock = lock(&MyMutex);
    MyMutex &= ~(1<<mutexNumb);
    unlock(&MyMutex);
}

#endif //MUTEX_ENABLE

#ifdef __cplusplus
}
#endif
