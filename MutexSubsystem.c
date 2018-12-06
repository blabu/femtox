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
    register bool_t flag_int = FALSE;
    // Здесь окажемся если мьютекс свободен
    if(INTERRUPT_STATUS) {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    MyMutex |= 1<<mutexNumb;
    if(flag_int) INTERRUPT_ENABLE;
    return FALSE; // Все впорядке мьютекс успешно захвачен этой функцией.
}

#include "logging.h"
// TRUE - Если мьютекс захватить НЕ УДАЛОСЬ
bool_t getMutex(const mutexType mutexNumb, TaskMng TPTR, BaseSize_t n, BaseParam_t data) {
    if(mutexNumb >= MUTEX_SIZE) return FALSE;// Если номер мьютекса больше возможного варианта выходим из функции
    register bool_t flag_int = FALSE;
    if(INTERRUPT_STATUS) {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    if(MyMutex & (1<<mutexNumb)) { // Если мьютекс с таким номером захвачен
    	SetTimerTask(TPTR, n, data, 2); // Попытаем счастья позже
    	if(flag_int) INTERRUPT_ENABLE;
        return TRUE; // Мьютекс уже захвачен кем-то возвращаем результат
    }
    // Здесь окажемся если мьютекс свободен
    MyMutex |= 1<<mutexNumb;
    if(flag_int) INTERRUPT_ENABLE;
    return FALSE; // Все впорядке мьютекс успешно захвачен этой функцией.
}

void freeMutex(const mutexType mutexNumb) {
    if(mutexNumb >= MUTEX_SIZE) return;// Если номер мьютекса больше возможного варианта выходим из функции
    register bool_t flag_int = FALSE;
    if(INTERRUPT_STATUS){
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    MyMutex &= ~(1<<mutexNumb);
    if(flag_int) INTERRUPT_ENABLE;
}

#endif //MUTEX_ENABLE

#ifdef __cplusplus
}
#endif
