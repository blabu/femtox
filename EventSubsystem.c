/*
 * EventSubsystem.c
 *
 *  Created on: 28 квіт. 2017 р.
 *      Author: oleksiy.khanin
 */

#include "TaskMngr.h"
#include "PlatformSpecific.h"

#ifdef __cplusplus
extern "C" {
#endif
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//                                  РЕАЛИЗАЦИЯ СОБЫТИЙ
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
#ifdef EVENT_LOOP_TASKS
#if EVENT_LIST_SIZE > 0xFE
#error "incompatible size"
#endif
typedef struct {
    Predicat_t      Predicat;   // Указатель на функцию условия
    CycleFuncPtr_t  CallBack;
} EventsType;
volatile static EventsType EventList[EVENT_LIST_SIZE];

void initEventList(void) {
    for(u08 i = 0; i<EVENT_LIST_SIZE; i++) {
        EventList[i].Predicat = NULL;
    }
}

void EventManager( void ) {
    for(u08 i = 0; i<EVENT_LIST_SIZE; i++) {
        if(EventList[i].Predicat == NULL) break;        // При первом
        if(EventList[i].Predicat()) {
		#ifdef SET_FRONT_TASK_ENABLE
        	SetFrontTask((TaskMng)EventList[i].CallBack,0,0);
		#else
        	SetTask((TaskMng)EventList[i].CallBack,0,0);
		#endif
        }
    }
}

bool_t CreateEvent(Predicat_t condition, CycleFuncPtr_t effect) {// Регистрирует новое событие в списке событий
    u08 i = 0;
    unlock_t unlock = lock((const void*const)EventList);
    for(;i < EVENT_LIST_SIZE; i++)
    {
        if(EventList[i].Predicat == NULL) break; // find empty event task
    }
    if(i < EVENT_LIST_SIZE) {
    	EventList[i].Predicat = condition;
    	EventList[i].CallBack = effect;
    	unlock((const void*const)EventList); //Далее востанавливаем прерывания (если необходимо)
    	return TRUE;
    }
    unlock((const void*const)EventList);
    return FALSE; // Событие невозможно создать т.к. очередь событий переполнена
}

void delEvent(Predicat_t condition){
    u08 i = 0;
    u08 countDeletedEvent = 0;
    unlock_t unlock = lock((const void*const)EventList);
    for(;i<EVENT_LIST_SIZE;i++)     // Выполняем поиск нашего события
    {
        if(EventList[i].Predicat == NULL) break;    // Если дошли до пустого, а значит последнего выходим из цикла
        if(EventList[i].Predicat == condition) // find event
        {
            EventList[i].Predicat = NULL;   // Удаляем событие
            countDeletedEvent++;            // и увеличиваем счетчик удаленных событий на один
            continue;
        }
        if(countDeletedEvent)  // Если факт того что задача была удалена установлен
        {
            EventList[i-countDeletedEvent].CallBack = EventList[i].CallBack;  // Переносим текущее событие на место удаленного
            EventList[i-countDeletedEvent].Predicat = EventList[i].Predicat;  // Переносим текущее
            EventList[i].Predicat = NULL;   // А текущее место освобождаем
        }
    }
    unlock((const void*const)EventList);
}
#endif //EVENT_LOOP_TASKS

#ifdef __cplusplus
}
#endif
