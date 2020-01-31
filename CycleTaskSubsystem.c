/*
MIT License

Copyright (c) 2017 Oleksiy Khanin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 * */

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
volatile static struct {
    CycleFuncPtr_t Call_Back;
    Time_t value;
    Time_t time;
    bool_t flagToQueue;
}Timers_Array[TIMERS_ARRAY_SIZE];

void initCycleTask(void) {
	for(u08 i=0; i<TIMERS_ARRAY_SIZE; i++) {
        Timers_Array[i].Call_Back = 0;
        Timers_Array[i].value = 0;
        Timers_Array[i].time = 0;
    }
}

void SetCycleTask(Time_t time, CycleFuncPtr_t CallBack, bool_t flagToQueue) {
	const unlock_t unlock = lock((void*)Timers_Array);
    for(register u08 i = 0; i<TIMERS_ARRAY_SIZE; i++){
        if(Timers_Array[i].value) continue; // Если таймер уже занят (не нулевой) переходим к следющему
        Timers_Array[i].Call_Back = CallBack;  // Запоминаем новый колбэк
        Timers_Array[i].flagToQueue = flagToQueue;      // Флаг определяет выполняется задача в таймере или ставится в глобальную очередь
        Timers_Array[i].value = time;       // Первый свободный таймер мы займем своей задачей
        Timers_Array[i].time = time;
        break;                          // выходим из цикла
    }
    unlock((void*)Timers_Array);
}

void delCycleTask(BaseSize_t arg_n, CycleFuncPtr_t CallBack) {
	const unlock_t unlock = lock((void*)Timers_Array);
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
            Timers_Array[i-countDeletedTask].flagToQueue = Timers_Array[i].flagToQueue;
            Timers_Array[i].value = 0; // После перемещения удаляем текущую задачу
        }

    }
    unlock((void*)Timers_Array);
}

#ifdef _PWR_SAVE
extern volatile u32 _minTimeOut;
u32 CycleService(void) {
	const unlock_t unlock = lock((void*)Timers_Array);
    register u08 i = 0;
    u32 tempMinTickCount = 0;
    while(Timers_Array[i].value) // Перебираем массив таймеров пока не встретили пустышку
    {
    	if(Timers_Array[i].time > _minTimeOut) {
    		Timers_Array[i].time -= _minTimeOut;// Если нашли не путой таймер тикаем
    		if(Timers_Array[i].time < tempMinTickCount || !tempMinTickCount) tempMinTickCount = Timers_Array[i].time;
    	}
    	else { // Если таймер дотикал
            Timers_Array[i].time = Timers_Array[i].value;     // Если таймер дотикал обновляем его значение
            if(Timers_Array[i].time < tempMinTickCount || !tempMinTickCount) tempMinTickCount = Timers_Array[i].time;
            if(!Timers_Array[i].flagToQueue) // Если флаг поставить задачу в очередь не выставлен выполняем функцию здесь же
                (*Timers_Array[i].Call_Back)();
            else
                SetTask((TaskMng)Timers_Array[i].Call_Back,0,0); // Если флаг установлен ставим задачу в очередь таймеров
        }
        i++;
        if(i>=TIMERS_ARRAY_SIZE) break;
    }
    unlock((void*)Timers_Array);
    return tempMinTickCount;
}
#else
void CycleService(void) {
	const unlock_t unlock = lock((void*)Timers_Array);
    register u08 i = 0;
    while(Timers_Array[i].value) // Перебираем массив таймеров пока не встретили пустышку
    {
    	Timers_Array[i].time--;// Если нашли не путой таймер тикаем
        if(!Timers_Array[i].time) // Если таймер дотикал
    	{
            Timers_Array[i].time = Timers_Array[i].value;     // Если таймер дотикал обновляем его значение
            if(!Timers_Array[i].flagToQueue) // Если флаг поставить задачу в очередь не выставлен выполняем функцию здесь же
                (*Timers_Array[i].Call_Back)();
            else
                SetTask((TaskMng)Timers_Array[i].Call_Back,0,0); // Если флаг установлен ставим задачу в очередь таймеров
        }
        i++;
        if(i>=TIMERS_ARRAY_SIZE) break;
    }
    unlock((void*)Timers_Array);
}
#endif
#endif  //CYCLE_FUNC

#ifdef __cplusplus
}
#endif
