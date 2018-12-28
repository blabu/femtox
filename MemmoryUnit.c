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

#ifdef ALLOC_MEM_LARGE
/*
Функции работы с кучей. Выделение и удаление памяти в куче.
Старший бит определяет свободен ли блок памяти или нет
Следующие два бита определяю начало и конец байтов размерности (аналог скобок)
Далее идут 5 бит размера.
******В случае выделеной (уже занятой) памяти******
100[размер] - единичный размерный байт аллоцированой памяти, заданного размера
101[размер]111[размер]110[размер] - составной размерный байт аллоцированной памяти
101[размер]110[размер]
******В случае свободной памяти******
000[размер]
001[размер]011[размер]010[размер]
001[размер]010[размер]
размер идет младшим байтом вперед
*/
#if HEAP_SIZE  > 0xFFFFFFFF
#error "incompatible size"
#endif
static u08 heap[HEAP_SIZE];  // Сама куча
static BaseSize_t sizeAllFreeMemmory = HEAP_SIZE;


void initHeap(void){
}

BaseSize_t getFreeMemmorySize(void){
    return sizeAllFreeMemmory;
}

static BaseSize_t getCurrentBlockSize(byte_ptr startBlock_ptr) {
	BaseSize_t size = 0;
	for(;;) {
		size = (size << 5) | (*startBlock_ptr & 0x1F);
		if( !(*startBlock_ptr & (1<<6)) ) break; // Если бит следующего размерного блока не выставлен выходим
		startBlock_ptr--;
	}
	return size;
}

static BaseSize_t getNextBlockSize(byte_ptr startSize_ptr) {
	BaseSize_t size = 0;
	for(u08 i=0;;i++) {
		size |= (*startSize_ptr & 0x1F) << (5*i);
		if( !(*startSize_ptr & (1<<5)) ) break; // Если бит следующего размерного блока не выставлен выходим
		startSize_ptr++;
	}
	return size;
}

BaseSize_t getAllocateMemmorySize(const byte_ptr data) {
	if(data > heap &&
       data < heap + HEAP_SIZE) {  // Если мы передали валидный указатель
		if(!(*(data-1) & (1<<7))) return 0;
		return getCurrentBlockSize(data);
    }
	return 0;
}

static u08 calculateSize(BaseSize_t blockSize) {
	u08 sz = 0;
	while(blockSize) {
		sz++;
		blockSize>>=5;
	}
	return sz;
}

void clearAllMemmory(void){
	BaseSize_t i = 0;
    unlock_t unlock = lock(heap);
    while(i < HEAP_SIZE) {
    	BaseSize_t blockSize = getNextBlockSize(&heap[i]);
    	if(!blockSize) break;
    	BaseSize_t temp = blockSize;
    	while(temp) {
    		heap[i] &= ~(1<<7);
    		i++;
    		temp >>= 5;
    	}
    	i += blockSize;
    }
    unlock(heap);
}

static byte_ptr alloc(byte_ptr startSize, BaseSize_t size) {
	u08 i = 0;
	if(size < 0x1F) {
		*startSize = 1<<7;
		*startSize |= size;
		return (startSize+1);
	}
	while(size > 0) {
		if(!i) startSize[i] = 5<<5;
		else startSize[i] = 7<<5;
		startSize[i] |= size & 0x1F;
		size >>= 5;
		i++;
	}
	startSize[i] &= ~(1<<5);
	return (startSize+i+1);
}

static void free(byte_ptr startBlock) {
	while(*(--startBlock) & (1<<7)) {
		*startBlock &= ~(1<<7);
		if(!(*startBlock & (1<<6))) break;
	}
}

byte_ptr allocMem(const BaseSize_t size) {
	if(size > 0) {
		BaseSize_t i = 0;
		unlock_t unlock = lock(heap);
		while((i+size) > HEAP_SIZE) {
			BaseSize_t blockSize = getNextBlockSize(&heap[i]);
			if(!blockSize) {
				byte_ptr res = alloc(&heap[i],size);
				unlock(heap);
				return res;
			}
			if((heap[i] >> 7) ||     // Если этот блок занят (последний бит равен 1)
				blockSize < size) {  // или размер этого блока слишком маленький
				i += blockSize + calculateSize(blockSize); // Пропускаем этот блок
				continue;
			}
			if((blockSize-size) < 10) { // Для исключения дефрагментации если найденый блок близок к требуемому
				byte_ptr res = alloc(&heap[i],blockSize);  // Выделяем его
				unlock(heap);
				return res;
			}
			// Здесь если блок свободен и размер его больше требуемого
			BaseSize_t restSize = blockSize-size;  // Вычисляем размер оставшегося блока памяти
			byte_ptr result = alloc(&heap[i],size);
			i += size+calculateSize(size);
			free(alloc(&heap[i],restSize));
			return result;
		}
	}
	return NULL;
}

void freeMem(const byte_ptr data) {
    if(data > heap &&
       data < heap + HEAP_SIZE)  // Если мы передали валидный указатель
    {
        free(data);
    }
}

void defragmentation(void){
	BaseSize_t i = 0;
	BaseSize_t blockSize = 0;
    sizeAllFreeMemmory=HEAP_SIZE;
    while(i < HEAP_SIZE) {   // Пока не закончится куча
    	BaseSize_t currentBlockSize = getNextBlockSize(&heap[i]);
    	u08 blkSz = calculateSize(currentBlockSize);
        if(!currentBlockSize) break;   // Если размер нулевой, значит выделения памяти еще не было
        if(heap[i] & (1<<7)) {   // Если блок памяти занят
            blockSize = 0;
            i += currentBlockSize+blkSz;  // переходим к концу этого блока
            sizeAllFreeMemmory -= (currentBlockSize + blkSz);
            continue;
        }
        if(blockSize) { //Если блок памяти свободен
        	u08 prevBlkSz = calculateSize(blockSize);
        	BaseSize_t SumBlock = (BaseSize_t)(blockSize + currentBlockSize + blkSz + prevBlkSz);
           	unlock_t unlock = lock(heap);
           	byte_ptr startBlock = heap+i-blockSize-prevBlkSz-blkSz; // Находим стартовую позицию составного блока
           	free(alloc(startBlock,SumBlock));
            blockSize = SumBlock; // Тперь составной блок это предыдущий блок
            i += currentBlockSize + blkSz;
            unlock(heap);
            continue;
        }
        blockSize = currentBlockSize;
        i += blockSize + blkSz;
    }
}

#endif //ALLOC_MEM_LARGE
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
	if(size < 128 && size) {
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
    return NULL;
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

#endif


#ifdef __cplusplus
}
#endif
