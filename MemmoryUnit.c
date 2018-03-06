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


#ifdef MEMMORY_LEAK_CONTROL
struct {
	byte_ptr data;
	byte_ptr* dataPtr;
}memmoryAllocateList[MAX_DATA_ALOCATE_COUNTER];

#include <stdio.h>

void checkLeak() {
	for(u08 i = 0; i<MAX_DATA_ALOCATE_COUNTER; i++) {
		if(memmoryAllocateList[i].data != NULL) {
			if(memmoryAllocateList[i].dataPtr == NULL) {
				freeMem(memmoryAllocateList[i].data);
				continue;
			}
			printf("%p, %p\n", memmoryAllocateList[i].data, *memmoryAllocateList[i].dataPtr);
			if(memmoryAllocateList[i].data != *memmoryAllocateList[i].dataPtr) {
				freeMem(memmoryAllocateList[i].data);
				memmoryAllocateList[i].data = NULL;
			}
		}
	}
}

byte_ptr** leakControlAlloc(u08 size, byte_ptr** res) {
	u08 i = 0;
	for(; i<MAX_DATA_ALOCATE_COUNTER; i++) { // Поиск пустого места для данных
		if(memmoryAllocateList[i].data == NULL) break; // Нашли пустышку
	}
	if(i == MAX_DATA_ALOCATE_COUNTER) {
		return NULL;
	}
	memmoryAllocateList[i].data = allocMem(size); // Выделяем память
	if(memmoryAllocateList[i].data == NULL) return NULL;
	memmoryAllocateList[i].dataPtr = &(memmoryAllocateList[i].data);
	if(res != NULL)	*res = memmoryAllocateList[i].dataPtr;// Модифицируем то на что раньше указывал res (это дает возможность удалить память)
	return &memmoryAllocateList[i].dataPtr; // Вернем новое значение
}
#endif



void initHeap(void)
{
#ifdef MEMMORY_LEAK_CONTROL
	for(u08 i = 0; i<MAX_DATA_ALOCATE_COUNTER; i++){
		memmoryAllocateList[i].data = NULL;
		memmoryAllocateList[i].dataPtr = NULL;
	}
#endif
  if(heap == NULL) {
      heap[0]=(1<<7)+1; // Заблокируем начало памяти для выделения
  }
}

u16 getFreeMemmorySize(void){
    return sizeAllFreeMemmory;
}

u16 getAllocateMemmorySize(byte_ptr data) {
    u16 size = 0;
	if(data > heap &&
       data < heap + HEAP_SIZE)  // Если мы передали валидный указатель
    {
		size = *(data-1);
		if(!(size & (1<<7))) size = 0; // Если старший бит не установлен значит память пустая
    }
	return size;
}

void clearAllMemmory(void){
	u16 i = 0;
    bool_t flag_int = FALSE;
    if(INTERRUPT_STATUS)
    {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    while(i < HEAP_SIZE) {
    	u08 blockSize = heap[i] & 0x7F;
    	if(!blockSize) break;
    	heap[i] &= ~(1<<7);
    	i+=blockSize;
    }
    if(flag_int) INTERRUPT_ENABLE;
}

byte_ptr allocMem(u08 size)  //size - до 127 размер блока выделяемой памяти
{
    if(size > 127 || !size) return NULL;  // Если попросили больше чем можем дать возвращаем ноль
    if(size == 1) size = 2;
    u16 i = 0;  // Поиск свободного места начнем с нулевого элемента, максимум определен размером u16
    bool_t flag_int = FALSE;
    if(INTERRUPT_STATUS)
    {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    while((i+size+1) < HEAP_SIZE) // Пока мы можем выделить тот объем памяти который у нас попросили
    {
        u08 blockSize = heap[i] & 0x7F;  // Вычисляем размер следующего блока памяти
        if(blockSize == 0)     // Если память здесь еще не выделялась
        {
            heap[i] = (1<<7) + size;    // Выделяем нужный объем памяти
            break;
        }
        if((heap[i] >> 7) ||   //  Если этот блок занят (последний бит равен 1)
            blockSize < size)   // или размер этого блока слишком маленький
        {
            i += (blockSize+1); // Перескакиваем через этот блок
            continue;
        }
        if((blockSize - size) < 2)  // Если размеры блоков совпали с точность до плюс одного байта
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
    if(flag_int) INTERRUPT_ENABLE;
    if((i+size+1) >= HEAP_SIZE) return NULL; // Если мы вышли из цикла по причине окончания кучи, вернем ноль
    return (heap + i + 1); // Иначе вернем валидный указатель на начало массива
}

void freeMem(byte_ptr data) {
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
    bool_t flag_int = FALSE;
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
                if(INTERRUPT_STATUS){
                    flag_int = TRUE;
                    INTERRUPT_DISABLE;
                }
                heap[i - (blockSize+1)] = SumBlock;
                blockSize = SumBlock;
                i += currentBlockSize + 1;
                if(flag_int) INTERRUPT_ENABLE;
                continue;
            }
        }
        blockSize = currentBlockSize;
        i += blockSize + 1;
    }
#ifdef MEMMORY_LEAK_CONTROL
    checkLeak();
#endif
}

#endif //ALLOC_MEM


#ifdef __cplusplus
}
#endif
