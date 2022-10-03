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

#ifdef DEBUG_CHEK_ALLOCATED_MOMORY
#include "logging.h"
typedef struct {
	byte_ptr ptr;
	BaseSize_t size;
	string_t comment;
} allocateDescriptor_t;
#define MAX_DESCRIPTORS 3000
static allocateDescriptor_t descriptor[MAX_DESCRIPTORS];
static u16 findDescriptor(const byte_ptr pointer) {
	u16 i = 0;
	for(;i<MAX_DESCRIPTORS;i++) {
		if(descriptor[i].ptr == pointer) break;
	}
	return i;
}
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
	heap[0] = 0;
#ifdef DEBUG_CHEK_ALLOCATED_MOMORY
	for(u16 i = 0; i<MAX_DESCRIPTORS; i++) {
		descriptor[i].ptr = NULL;
	}
#endif
}

BaseSize_t getFreeMemmorySize(void){
	BaseSize_t s = sizeAllFreeMemmory;
	while(s != sizeAllFreeMemmory) s = sizeAllFreeMemmory;
	if(s >= HEAP_SIZE) {
		defragmentation();
	}
    return s;
}

static BaseSize_t getCurrentBlockSize(byte_ptr startBlock_ptr) {
	BaseSize_t size = 0;
	for(;;) {
		startBlock_ptr--;
		size = (size << 5) | (*startBlock_ptr & 0x1F);
		if( !(*startBlock_ptr & (1<<6)) ) break; // Если бит следующего размерного блока не выставлен выходим
	}
	return size;
}

static BaseSize_t getNextBlockSize(byte_ptr startSize_ptr) {
	BaseSize_t size = 0;
	for(u08 i=0;;i++) {
		size |= (*startSize_ptr & 0x1F) << (5*i);
		if(!(*startSize_ptr & (1<<5))) break; // Если бит следующего размерного блока не выставлен выходим
		startSize_ptr++;
	}
	return size;
}

BaseSize_t getAllocateMemmorySize(const byte_ptr data) {
	if(data > heap &&
       data < heap + HEAP_SIZE) {  // Если мы передали валидный указатель
		if( !(*(data-1) & (1<<7)) ) {
			return 0;
		}
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
    const unlock_t unlock = lock(heap);
    while(i < HEAP_SIZE) {
    	const BaseSize_t blockSize = getNextBlockSize(&heap[i]);
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
	#ifdef DEBUG_CHEK_ALLOCATED_MOMORY
    for(u16 i = 0; i<MAX_DESCRIPTORS; i++) descriptor[i].ptr = NULL;
	#endif
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
	startSize[i-1] &= ~(1<<5);
	return (startSize+i);
}

static void free(byte_ptr startBlock) {
	while(*(--startBlock) & (1<<7)) {
		*startBlock &= ~(1<<7);
		if(!(*startBlock & (1<<6))) break;
	}
}

static byte_ptr _allocMem(const BaseSize_t size) {
	if(size > 0) {
		BaseSize_t i = 0;
		const unlock_t unlock = lock(heap);
		while((i+size) < HEAP_SIZE) {
			const BaseSize_t blockSize = getNextBlockSize(&heap[i]);
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
            const BaseSize_t newSizeBlck = size + calculateSize(size); // Вычисляем размер нового куска с учетом его размера
            const BaseSize_t restSize = blockSize + calculateSize(blockSize) - newSizeBlck;  // Вычисляем размер оставшегося СВОБОДНОГО блока памяти
            BaseSize_t freeSizeBlck = restSize - calculateSize(restSize); // Из него следует предусмотреть место для хранения самого размера будущего свободного блока
            while(freeSizeBlck < restSize) {
                if((calculateSize(freeSizeBlck) + freeSizeBlck) == restSize) {
                    byte_ptr result = alloc(&heap[i],size); // Выделяем нужный блока памяти
                    i += newSizeBlck; // Вычисляем конец новго выделенного блока
                    free(alloc(&heap[i],freeSizeBlck)); // Освобождаем выделенный кусок
                    unlock(heap);
                    return result;
                }
                freeSizeBlck++;
            }
            i += blockSize + calculateSize(blockSize); // Пропускаем этот блок
		}
		unlock(heap);
	}
	return NULL;
}

#ifdef DEBUG_CHEK_ALLOCATED_MOMORY
byte_ptr allocMemComment(const BaseSize_t size, string_t comment) {
    byte_ptr res = _allocMem(size);
    if(res == NULL) {
        defragmentation();
        res = _allocMem(size);
    }
   	const u16 i = findDescriptor(NULL);
   	if(i < MAX_DESCRIPTORS) {descriptor[i].ptr = res; descriptor[i].size = size; descriptor[i].comment = comment;}
   	else {
   		MaximizeErrorHandler((const string_t)"Overflow descriptor list in allocMem");
   		showAllBlocks();
   	}
    return res;
}

void showAllBlocks() {
	for(u16 i = 0; i<MAX_DESCRIPTORS; i++) {
		if(descriptor[i].ptr != NULL) {
			writeLog2Str((string_t)"Descriptor is busy ", (string_t)descriptor[i].comment);
			writeLogWithStr((string_t)"Descriptor pointer is ", (u32)descriptor[i].ptr);
			writeLogWithStr((string_t)"Allocated size ", (u32)descriptor[i].size);
		}
	}
}
#endif

byte_ptr allocMem(const BaseSize_t size) {
    byte_ptr res = _allocMem(size);
    if(res == NULL) {
        defragmentation();
        res = _allocMem(size);
    }
#ifdef DEBUG_CHEK_ALLOCATED_MOMORY
	const u16 i = findDescriptor(NULL);
	if(i < MAX_DESCRIPTORS) {descriptor[i].ptr = res; descriptor[i].size = size;}
	else MaximizeErrorHandler("Overflow descriptor list in allocMem");
#endif
    return res;
}

#ifdef DEBUG_CHEK_ALLOCATED_MOMORY
void writeLogWithStr(const string_t c_str, u32 n);
#endif

bool_t validateMemory() {
    BaseSize_t i = 0;
    while(i < HEAP_SIZE) {
        const BaseSize_t blocSize = getNextBlockSize(&heap[i]);
        if(blocSize) return TRUE;
        i+=blocSize + calculateSize(blocSize);
        if(i > HEAP_SIZE) {
			#ifdef DEBUG_CHEK_ALLOCATED_MOMORY
            writeLogWithStr("ERROR: invalid memory block in ", i-blocSize);
			#endif
            return FALSE;
        }
    }
    return TRUE;
}

void freeMem(const byte_ptr data) {
    if(data > heap &&
       data < heap + HEAP_SIZE)  // Если мы передали валидный указатель
    {
		#ifdef DEBUG_CHEK_ALLOCATED_MOMORY
    		const u16 i = findDescriptor(data);
    		if(i < MAX_DESCRIPTORS) {
    			const BaseSize_t sz = getCurrentBlockSize(data);
    			if(sz >= descriptor[i].size && sz - descriptor[i].size < 10) {
    				descriptor[i].ptr = NULL;
    			}
    			else {
    				writeLog2Str("ERROR in freeMem: ", descriptor[i].comment);
    				writeLogWithStr("ERROR: allocated memory", descriptor[i].size);
    				writeLogWithStr("ERROR: try free memory", sz);
    				MaximizeErrorHandler("Incorrect memory size");
    			}
    		}
    		else {
    			writeLogWithStr("ERROR:Try freeMem by ptr ", (u32)data);
    			MaximizeErrorHandler("Undefine descriptor in list for freeMem");

    		}
		#endif
    	const unlock_t unlock = lock(heap);
        free(data);
        unlock(heap);
    } else if(data != NULL) {
    	writeLogWithStr("ERROR: Incorrect ptr in freeMem ", (u32)data);
    }
}

void defragmentation(void){
	BaseSize_t i = 0;
	BaseSize_t blockSize = 0;
    sizeAllFreeMemmory=HEAP_SIZE;
    const unlock_t unlock = lock(heap);
    while(i < HEAP_SIZE) {   // Пока не закончится куча
    	const BaseSize_t currentBlockSize = getNextBlockSize(&heap[i]);
    	const u08 blkSz = calculateSize(currentBlockSize);
        if(!currentBlockSize) break;   // Если размер нулевой, значит выделения памяти еще не было
        if(heap[i] & (1<<7)) {   // Если блок памяти занят
            blockSize = 0;
            i += currentBlockSize+blkSz;  // переходим к концу этого блока
            sizeAllFreeMemmory -= (currentBlockSize + blkSz);
            continue;
        }
        if(blockSize) { //Если уже были освобождения подряд
        	const u08 prevBlkSz = calculateSize(blockSize);
        	const BaseSize_t SumBlockSize = (BaseSize_t)(blockSize + prevBlkSz) + (BaseSize_t)(currentBlockSize + blkSz);
        	BaseSize_t  newSumBlocKSize = SumBlockSize - calculateSize(SumBlockSize);
        	while(newSumBlocKSize < SumBlockSize) {
        	    if(newSumBlocKSize + calculateSize(newSumBlocKSize) == SumBlockSize) {
                    byte_ptr startBlock = heap+i-blockSize-prevBlkSz; // Находим стартовую позицию составного блока
                    free(alloc(startBlock,newSumBlocKSize));
                    blockSize = newSumBlocKSize; // Тперь составной блок это предыдущий блок
                    i += currentBlockSize + blkSz;
                    break;
                }
                newSumBlocKSize++;
        	}
        	if(newSumBlocKSize < SumBlockSize) continue;
        }
        i += currentBlockSize + blkSz;
        blockSize = currentBlockSize;
    }
    unlock(heap);
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
	heap[0] = 0;
#if REGISTRY_ALLOCATE_MEMMORY_SIZE > 0
	clearAllRegister();
#endif
}

u16 getFreeMemmorySize(void){
	if(sizeAllFreeMemmory >= HEAP_SIZE) {
		defragmentation();
	}
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
    const unlock_t unlock = lock(heap);
    while(i < HEAP_SIZE) {
    	u08 blockSize = heap[i] & 0x7F;
    	if(!blockSize) break;
    	heap[i] &= ~(1<<7);
    	i+=blockSize+1;
    }
    unlock(heap);
#if REGISTRY_ALLOCATE_MEMMORY_SIZE > 0
    clearAllRegister();
#endif
}

static byte_ptr _allocMem(const u08 size) { //size - до 127 размер блока выделяемой памяти
	if(size < 128 && size) {
		u16 i = 0;  // Поиск свободного места начнем с нулевого элемента, максимум определен размером u16
		const unlock_t unlock = lock(heap);
		while((i+size) < HEAP_SIZE) // Пока мы можем выделить тот объем памяти который у нас попросили
		{
			const u08 blockSize = heap[i] & 0x7F;  // Вычисляем размер следующего блока памяти
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
		if((i+size) < HEAP_SIZE) { // If we break the loop before end
			return (heap + i + 1); // вернем валидный указатель на начало массива
		}
		return NULL; // Иначе мы вышли из цикла по причине окончания кучи, вернем ноль
	}
    return NULL;
}

byte_ptr allocMem(const u08 size) {
    byte_ptr res = _allocMem(size);
    if(res == NULL) {
        defragmentation();
        return _allocMem(size);
    }
	#ifdef DEBUG_CHEK_ALLOCATED_MOMORY
		u16 i = findDescriptor(NULL);
		if(i < MAX_DESCRIPTORS) {descriptor[i].ptr = res; descriptor[i].size = size;}
		else MaximizeErrorHandler("Overflow descriptor list in allocMem");
	#endif
    return res;
}

void freeMem(const byte_ptr data) {
    if(data > heap &&
       data < heap + HEAP_SIZE)  // Если мы передали валидный указатель
    {
    	const unlock_t unlock = lock(heap);
        *(data-1) &= ~(1<<7); // Очистим флаг занятости данных (не трогая при этом сами данные и их размер)
        unlock(heap);
    }
}

void defragmentation(void){
    u16 i = 0;
    u08 blockSize = 0;
    sizeAllFreeMemmory=HEAP_SIZE;
    const unlock_t unlock = lock(heap);
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
            if(SumBlock <= 127) { //Если SumBlock == 127 нет смысла его перезаписывать
            	heap[i - (blockSize+1)] = SumBlock;
            	blockSize = SumBlock;
            	i += currentBlockSize + 1;
                continue;
            } else {
                i -= blockSize+1;
                heap[i] = 127;
                i += (127+1);
                currentBlockSize = SumBlock - (127+1);
                heap[i] = currentBlockSize;
            }
        }
        blockSize = currentBlockSize;
        i += blockSize + 1;
    }
    unlock(heap);
}

#endif

void freeMemTask(BaseSize_t count, BaseParam_t pointer) {
	freeMem((byte_ptr)pointer);
	execCallBack(freeMemTask);
}

#ifdef __cplusplus
}
#endif
