/*
 * CycleTaskSubsystem.c
 *
 *  Created on: 28 квіт. 2017 р.
 *      Author: oleksiy.khanin
 */

#include "TaskMngr.h"
#include "PlatformSpecific.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CYCLE_FUNC
#if TIMERS_ARRAY_SIZE > 0xFE
#error "incompatible size"
#endif
/*
Эта часть содержит набор функция для реализации циклически выполняемых коротких функций
Функции выполняются внутри прерывания и не должны содержать задержек или разрешений прерываний
*/
volatile static struct
{
    CycleFuncPtr_t Call_Back;
    Time_t value;
    Time_t time;
    bool_t flagToManager;
}Timers_Array[TIMERS_ARRAY_SIZE];

void initCycleTask(void)
{
    for(u08 i=0; i<TIMERS_ARRAY_SIZE; i++)
    {
        Timers_Array[i].Call_Back = 0;
        Timers_Array[i].value = 0;
        Timers_Array[i].time = 0;
    }
}

void SetCycleTask(Time_t time, CycleFuncPtr_t CallBack, bool_t toManager) {
    bool_t flag_int = FALSE;
    if(INTERRUPT_STATUS) {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    for(register u08 i = 0; i<TIMERS_ARRAY_SIZE; i++){
        if(Timers_Array[i].value) continue; // Если таймер уже занят (не нулевой) переходим к следющему
        Timers_Array[i].Call_Back = CallBack;  // Запоминаем новый колбэк
        Timers_Array[i].flagToManager = toManager;      // Флаг определяет выполняется задача в таймере или ставится в глобальную очередь
        Timers_Array[i].value = time;       // Первый свободный таймер мы займем своей задачей
        Timers_Array[i].time = time;
        break;                          // выходим из цикла
    }
    if(flag_int) INTERRUPT_ENABLE;
}

void delCycleTask(BaseSize_t arg_n, CycleFuncPtr_t CallBack) {
    bool_t flag_int = FALSE;
    if(INTERRUPT_STATUS) {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    u08 countDeletedTask = 0;  // Количество удаленных задач
    for(register u08 i = 0; i<TIMERS_ARRAY_SIZE; i++) {
        if(!Timers_Array[i].value) break; // Если наткнулись на пустой таймер выходим из цикла(дальше искать нет смысла)
        if(Timers_Array[i].Call_Back == CallBack) // Если нашли нашу задачу
        {
            countDeletedTask++;         // Фиксируем факт того, что задача была удалена (чтобы все следующие задачи сместить наверх)
            Timers_Array[i].value = 0;  // Удаляем её, а все последующие не нулевые функции смещаем на одну позицию наверх
            continue;                   // Переходим к следующей позиции
        }
        if(countDeletedTask)  // Если факт того что задача была удалена установлен
        {
            Timers_Array[i-countDeletedTask].Call_Back = Timers_Array[i].Call_Back;  // Переносим текущую
            Timers_Array[i-countDeletedTask].time = Timers_Array[i].time;  // Переносим текущую
            Timers_Array[i-countDeletedTask].value = Timers_Array[i].value;
            Timers_Array[i-countDeletedTask].flagToManager = Timers_Array[i].flagToManager;
            Timers_Array[i].value = 0; // После перемещения удаляем текущую задачу
        }

    }
    if(flag_int) INTERRUPT_ENABLE;
}


void CycleService(void) {
    register u08 i = 0;
    while(Timers_Array[i].value) // Перебираем массив таймеров пока не встретили пустышку
    {
    	Timers_Array[i].time--;// Если нашли не путой таймер тикаем
        if(!Timers_Array[i].time) // Если таймер дотикал
    	{
            Timers_Array[i].time = Timers_Array[i].value;     // Если таймер дотикал обновляем его значение
            if(!Timers_Array[i].flagToManager) // Если флаг поставить задачу в очередь не выставлен выполняем функцию здесь же
                (*Timers_Array[i].Call_Back)();
            else
                SetTask((TaskMng)Timers_Array[i].Call_Back,0,0); // Если флаг установлен ставим задачу в очередь таймеров
        }
        i++;
        if(i>TIMERS_ARRAY_SIZE) break;
    }
}
#endif  //CYCLE_FUNC

#ifdef __cplusplus
}
#endif
