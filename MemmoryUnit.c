/*
 * memUnit.c
 *
 *  Created on: 28 квіт. 2017 р.
 *      Author: oleksiy.khanin
 */


#include "TaskMngr.h"
#include "PlatformSpecific.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ALLOC_MEM
/*
Функции работы с кучей. Выделение и удаление памяти в куче. Максмально единоразово можно выделить до 127 байт
Перед блоком памяти хранится байт с размером этого блока (0...6 биты), а последний бит определяет занята
эта память или свободна (поэтому до 127 байт единоразово)
*/
#if HEAP_SIZE  > 0xFFFF
#error "incompatible size"
#endif
static u08 heap[HEAP_SIZE];  // Сама куча
static u16 sizeAllFreeMemmory = HEAP_SIZE;


void initHeap(void){
}

u16 getFreeMemmorySize(void){
    return sizeAllFreeMemmory;
}

u16 getAllocateMemmorySize(const byte_ptr data) {
    u16 size = 0;
	if(data > heap &&
       data < heap + HEAP_SIZE)  // Если мы передали валидный указатель
    {
		size = *(data-1);
		if(!(size & (1<<7))) size = 0; // Если старший бит не установлен значит память пустая
    }
	return size & 0x7F;
}

void clearAllMemmory(void){
	u16 i = 0;
    unlock_t unlock = lock(heap);
    while(i < HEAP_SIZE) {
    	u08 blockSize = heap[i] & 0x7F;
    	if(!blockSize) break;
    	heap[i] &= ~(1<<7);
    	i+=blockSize+1;
    }
    unlock(heap);
}

byte_ptr allocMem(const u08 size) { //size - до 127 размер блока выделяемой памяти
    if(size > 127 || !size) {
    	return NULL;  // Если попросили больше чем можем дать возвращаем ноль
    }
    u16 i = 0;  // Поиск свободного места начнем с нулевого элемента, максимум определен размером u16
    unlock_t unlock = lock(heap);
    while((i+size) < HEAP_SIZE) // Пока мы можем выделить тот объем памяти который у нас попросили
    {
        u08 blockSize = heap[i] & 0x7F;  // Вычисляем размер следующего блока памяти
        if(!blockSize) {    // Если память здесь еще не выделялась
            heap[i] = (1<<7) + size;    // Выделяем нужный объем памяти
            break;
        }
        if((heap[i] >> 7) ||   //  Если этот блок занят (последний бит равен 1)
            blockSize < size)   // или размер этого блока слишком маленький
        {
            i += (blockSize+1); // Перескакиваем через этот блок
            continue;
        }
        if((blockSize - size) < 3)  // Если размеры блоков совпали с точность до плюс двух байт (значит можно будет выделить еще раз)
        {
            heap[i] |= (1<<7);
            break;
        }
        // Сюда мы попадем если размер свободной памяти строго больше размера нам необходимой памяти
        // хотябы на 2 байта (Один байт полезный и один служебный)
        heap[i] = (1<<7) + size;
        heap[i+size+1] = blockSize - size - 1; // Следующий пустой блок будет на один байт короче (этот байт служебный)
        break;
    }
    unlock(heap);
    if((i+size+1) > HEAP_SIZE) {
    	return NULL; // Если мы вышли из цикла по причине окончания кучи, вернем ноль
    }
    return (heap + i + 1); // Иначе вернем валидный указатель на начало массива
}

void freeMem(const byte_ptr data) {
    if(data > heap &&
       data < heap + HEAP_SIZE)  // Если мы передали валидный указатель
    {
        *(data-1) &= ~(1<<7); // Очистим флаг занятости данных (не трогая при этом сами данные и их размер)
    }
}


void defragmentation(void){
    u16 i = 0;
    u08 blockSize = 0;
    sizeAllFreeMemmory=HEAP_SIZE;
    while(i < HEAP_SIZE) {   // Пока не закончится куча
        u08 currentBlockSize = heap[i]&0x7F; //Выделяем размер блока (младшие 7 байт)
        if(!currentBlockSize) break;   // Если размер нулевой, значит выделения памяти еще не было
        if(heap[i] & (1<<7)) {   // Если блок памяти занят
            blockSize = 0;
            i += currentBlockSize + 1;  // переходим к концу этого блока
            sizeAllFreeMemmory -= currentBlockSize + 1;
            continue;
        }
        if(blockSize) { //Если блок памяти свободен
            u08 SumBlock = (u08)(blockSize + currentBlockSize + 1);
            if(SumBlock < 127) {
            	unlock_t unlock = lock(heap);
            	heap[i - (blockSize+1)] = SumBlock;
                blockSize = SumBlock;
                i += currentBlockSize + 1;
                unlock(heap);
                continue;
            }
        }
        blockSize = currentBlockSize;
        i += blockSize + 1;
    }
}

#endif //ALLOC_MEM


#ifdef __cplusplus
}
#endif
