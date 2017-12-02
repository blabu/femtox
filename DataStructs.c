/*
 * dataStructs.c
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
//                                  РЕАЛИЗАЦИЯ СТРУКТУР ДАННЫХ
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
#ifdef DATA_STRUCT_MANAGER
#if ArraySize > 0xFE
#error "incompatible size"
#endif
typedef struct
{
    void* Data;               // Указатель на начало очереди
    BaseSize_t firstCount;    // Указатель на первый свободный элемент абстрактной структуры данных
    BaseSize_t lastCount;     // Указатель на последний фактический элемент в абстрактной структуре данных
    BaseSize_t sizeElement;   // Размер одного элемента абстрактной структуры данных
    BaseSize_t sizeAllElements;// Общий размер в количествах элементов в абстрактной структуре данных
} AbstractDataType;
static AbstractDataType Data_Array[ArraySize];   // Собственно сам массив абстрактных структур данных

/***************************/
/***************************/
void showAllDataStruct(void)
{
}

static inline u08 findNumberDataStruct(const void* const Data)
{
    register u08 i = 0;
    for(; i<ArraySize; i++) // находим абстрактную структуру данных
    {
        if(Data_Array[i].Data == Data) break;
    }
    return i;
}

void initDataStruct(void)  // Инициализация абстрактной структуры данных
{
    for(register u08 i = 0; i<ArraySize; i++)
    {
        Data_Array[i].firstCount = 0;
        Data_Array[i].lastCount = 0;
        Data_Array[i].Data = NULL;
        Data_Array[i].sizeElement = 0;
        Data_Array[i].sizeAllElements = 0;
    }
}

// Функция создает абстрактную структуру данных (резервирует место под нее в глобальном массиве)
// sizeElement - размер одного элемента в БАЙТАХ, sizeAll - размер очереди в ЭЛЕМЕНТАХ
u08 CreateDataStruct(const void* D, const BaseSize_t sizeElement, const BaseSize_t sizeAll){
    bool_t flag_int = FALSE;
    register u08 i = 0;
    for(; i<ArraySize; i++) // Ищем пустое место в списке для новой структуры данных
    {
        if(Data_Array[i].Data == D) return OTHER_ERROR; // Если такая структура уже есть
        if(Data_Array[i].Data == NULL) break;
    }
    if(i == ArraySize) return NOT_FAUND_DATA_STRUCT_ERROR;
    if(INTERRUPT_STATUS) {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    Data_Array[i].Data = (void*)D; // Адрес начала
    Data_Array[i].sizeElement = sizeElement; // размер одного элемента в байтах
    Data_Array[i].sizeAllElements = sizeAll; // Размер всей очереди в элементах
    Data_Array[i].firstCount= sizeAll;
    Data_Array[i].lastCount = sizeAll;
    if(flag_int) INTERRUPT_ENABLE;
    return EVERYTHING_IS_OK;
}

// Удаляем абстрактную структуру данных
u08 delDataStruct(const void* Data)  // Удаляем из массива абстрактную структуру данных с заданным идентификатором
{
    u08 i = findNumberDataStruct(Data);
    if(i == ArraySize) return NOT_FAUND_DATA_STRUCT_ERROR;  // Если такой не существует в массиве, выдаем ошибку
    Data_Array[i].Data = NULL;    // Если абстрактная структура данных есть удаляем ее
    return EVERYTHING_IS_OK;
}

u08 PutToCycleDataStruct(const void* Elem, const void* Array) {
    bool_t flag_int = FALSE;
    register u08 i = findNumberDataStruct(Array);
    if(i == ArraySize) return NOT_FAUND_DATA_STRUCT_ERROR;    // Если мы не нашли абстрактную структуру данных с указанным идентификтором выходим
    BaseSize_t frontCount = (Data_Array[i].firstCount < Data_Array[i].sizeAllElements)? Data_Array[i].firstCount+1:0;
    if(INTERRUPT_STATUS){
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    unsigned int offset = Data_Array[i].firstCount * Data_Array[i].sizeElement; //вычисляем смещение в байтах
    void* dst = (void*)((byte_ptr)Data_Array[i].Data + offset);     // Определяем адресс куда копировать
    memCpy(dst, Elem, Data_Array[i].sizeElement); // Вставляем наш элемент
    Data_Array[i].firstCount = frontCount;
    if(Data_Array[i].lastCount < Data_Array[i].sizeAllElements) Data_Array[i].lastCount++;
    if(flag_int) INTERRUPT_ENABLE;
    return EVERYTHING_IS_OK;
}

u08 GetFromCycleDataStruct(void* returnValue, const void* Array)
{
    bool_t flag_int = FALSE;
    register u08 i = findNumberDataStruct(Array);
    if(i == ArraySize) return NOT_FAUND_DATA_STRUCT_ERROR;    // Если в массиве нет искомой абстрактной структуры данных с заданным идентификатором
    if(Data_Array[i].lastCount > 0) { // Если есть какие либо данные
    	if(INTERRUPT_STATUS) {
    		flag_int = TRUE;
    		INTERRUPT_DISABLE;
    	}
    	Data_Array[i].lastCount--;
    	Data_Array[i].firstCount = (Data_Array[i].firstCount)? Data_Array[i].firstCount : Data_Array[i].sizeAllElements;
    	Data_Array[i].firstCount--;
    	unsigned int offset = Data_Array[i].firstCount * Data_Array[i].sizeElement;  // Определяем смещение на элемент, который надо достать
    	void* dst = (void*)((byte_ptr)Data_Array[i].Data + offset);     // Записываем адрес памяти свободной ячейки
    	memCpy(returnValue, dst, Data_Array[i].sizeElement);   // Если структура данных найдена, читаем от туда первый (самый старый) элемент
    	if(flag_int) INTERRUPT_ENABLE;  // Если все происходило не в прерывании восстанавливаем разрешение прерываний
    	return EVERYTHING_IS_OK;   // Если все впорядке возвращаем ноль
    }
    else {
    	*((byte_ptr)returnValue) = 0;
    	return OVERFLOW_OR_EMPTY_ERROR;
    }
}

//Положить элемент Elem в начало структуры данных Array
u08 PutToFrontDataStruct(const void* Elem, const void* Array)
{
    bool_t flag_int = FALSE;
    register u08 i = findNumberDataStruct(Array);
    if(i == ArraySize) return NOT_FAUND_DATA_STRUCT_ERROR;    // Если мы не нашли абстрактную структуру данных с указанным идентификтором выходим
    BaseSize_t frontCount = (Data_Array[i].firstCount < Data_Array[i].sizeAllElements)? Data_Array[i].firstCount+1:0;
    if(frontCount == Data_Array[i].lastCount) return OVERFLOW_OR_EMPTY_ERROR;  // Если после добавления мы догоним lastCount, значит структура заполнена
    if(INTERRUPT_STATUS)
    {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    unsigned int offset = Data_Array[i].firstCount * Data_Array[i].sizeElement; //вычисляем смещение в байтах
    void* dst = (void*)((byte_ptr)Data_Array[i].Data + offset);     // Определяем адресс куда копировать
    memCpy(dst, Elem, Data_Array[i].sizeElement); // Вставляем наш элемент
    Data_Array[i].firstCount = frontCount;
    if(flag_int) INTERRUPT_ENABLE;
    return EVERYTHING_IS_OK;
}

// Положить элемент Elem в конец абстрактной структуры данных Array
u08 PutToEndDataStruct(const void* Elem, const void* Array){
    bool_t flag_int = FALSE;
    register u08 i = findNumberDataStruct(Array);
    if(i == ArraySize) return NOT_FAUND_DATA_STRUCT_ERROR;    // Если мы не нашли абстрактную структуру данных с указанным идентификтором выходим
    BaseSize_t endCount = (Data_Array[i].lastCount)? Data_Array[i].lastCount:Data_Array[i].sizeAllElements;
    endCount--;
    if(endCount == Data_Array[i].firstCount) return OVERFLOW_OR_EMPTY_ERROR;  // Если она заполнена полностью писать некуда
    if(INTERRUPT_STATUS) {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    unsigned int offset = endCount * Data_Array[i].sizeElement;  // Определяем смещение на свободную позицию (количество байт)
    void* dst = (void*)((byte_ptr)Data_Array[i].Data + offset); // Записываем адрес памяти начала свободной ячейки
    memCpy(dst, Elem, Data_Array[i].sizeElement);  // Копируем все байты Elem в массив Array с заданным смещением
    Data_Array[i].lastCount = endCount;           // После копирования инкрементируем текущую позицию
    if(flag_int) INTERRUPT_ENABLE;
    return EVERYTHING_IS_OK;
}

u08 GetFromFrontDataStruct(void* returnValue, const void* Array) // Достаем элемент с начала структуры данных
{
    bool_t flag_int = FALSE;
    register u08 i = findNumberDataStruct(Array);
    if(i == ArraySize) return NOT_FAUND_DATA_STRUCT_ERROR;    // Если в массиве нет искомой абстрактной структуры данных с заданным идентификатором
    if(Data_Array[i].firstCount == Data_Array[i].lastCount) {return OVERFLOW_OR_EMPTY_ERROR;} // Если она пустая читать нечего
    if(INTERRUPT_STATUS)
    {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    Data_Array[i].firstCount = (Data_Array[i].firstCount)? Data_Array[i].firstCount : Data_Array[i].sizeAllElements;
    Data_Array[i].firstCount--;
    unsigned int offset = Data_Array[i].firstCount * Data_Array[i].sizeElement;  // Определяем смещение на элемент, который надо достать
    void* dst = (void*)((byte_ptr)Data_Array[i].Data + offset);     // Записываем адрес памяти свободной ячейки
    memCpy(returnValue, dst, Data_Array[i].sizeElement);   // Если структура данных найдена, читаем от туда первый (самый старый) элемент
    if(flag_int) INTERRUPT_ENABLE;  // Если все происходило не в прерывании восстанавливаем разрешение прерываний
    return EVERYTHING_IS_OK;   // Если все впорядке возвращаем ноль
}

u08 GetFromEndDataStruct(void* returnValue, const void* Array) // Достаем элемент с конца структуры данных
{
    bool_t flag_int = FALSE;
    register u08 i = findNumberDataStruct(Array);
    if(i == ArraySize) return NOT_FAUND_DATA_STRUCT_ERROR;    // Если в массиве нет искомой абстрактной структуры данных с заданным идентификатором
    if(Data_Array[i].lastCount == Data_Array[i].firstCount) return OVERFLOW_OR_EMPTY_ERROR; //Проверка пустая ли структура данных
    if(INTERRUPT_STATUS)
    {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    unsigned int offset = Data_Array[i].lastCount * Data_Array[i].sizeElement;
    void* src = (void*)((byte_ptr)Data_Array[i].Data+offset);
    memCpy(returnValue, src, Data_Array[i].sizeElement);   // Если структура данных найдена, читаем от туда первый (самый старый) элемент
    Data_Array[i].lastCount = (Data_Array[i].lastCount < Data_Array[i].sizeAllElements)? Data_Array[i].lastCount+1:0;
    if(flag_int) INTERRUPT_ENABLE;  // Если все происходило не в прерывании восстанавливаем разрешение прерываний
    return EVERYTHING_IS_OK;   // Если все впорядке возвращаем ноль
}

u08 delFromFrontDataStruct(const void* const Data)
{
    bool_t flag_int = FALSE;
    register u08 i = findNumberDataStruct(Data);
    if(i == ArraySize) return NOT_FAUND_DATA_STRUCT_ERROR;    // Если в массиве нет искомой абстрактной структуры данных с заданным идентификатором
    if(Data_Array[i].firstCount == Data_Array[i].lastCount) {return OVERFLOW_OR_EMPTY_ERROR;} // Если она пустая читать нечего
    if(INTERRUPT_STATUS)
    {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    Data_Array[i].firstCount = (Data_Array[i].firstCount)? Data_Array[i].firstCount : Data_Array[i].sizeAllElements; // Закольцовываем счетчики
    Data_Array[i].firstCount--;
    if(flag_int) INTERRUPT_ENABLE;  // Если все происходило не в прерывании восстанавливаем разрешение прерываний
    return EVERYTHING_IS_OK;   // Если все впорядке возвращаем ноль
}

u08 delFromEndDataStruct(const void* const Data)
{
    bool_t flag_int = FALSE;
    register u08 i = findNumberDataStruct(Data);
    if(i == ArraySize) return NOT_FAUND_DATA_STRUCT_ERROR;    // Если в массиве нет искомой абстрактной структуры данных с заданным идентификатором
    if(Data_Array[i].lastCount == Data_Array[i].firstCount) return OVERFLOW_OR_EMPTY_ERROR; //Проверка пустая ли структура данных
    if(INTERRUPT_STATUS)
    {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    Data_Array[i].lastCount = (Data_Array[i].lastCount < Data_Array[i].sizeAllElements)? Data_Array[i].lastCount+1:0;
    if(flag_int) INTERRUPT_ENABLE;  // Если все происходило не в прерывании восстанавливаем разрешение прерываний
    return EVERYTHING_IS_OK;   // Если все впорядке возвращаем ноль
}

u08 peekFromFrontData(void* returnValue, const void* Array) {
    bool_t flag_int = FALSE;
    register u08 i = findNumberDataStruct(Array);
    if(i == ArraySize) return NOT_FAUND_DATA_STRUCT_ERROR;    // Если в массиве нет искомой абстрактной структуры данных с заданным идентификатором
    if(Data_Array[i].firstCount == Data_Array[i].lastCount) {return OVERFLOW_OR_EMPTY_ERROR;} // Если она пустая читать нечего
    if(INTERRUPT_STATUS) {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    u08 count = (Data_Array[i].firstCount)? Data_Array[i].firstCount-1 : Data_Array[i].sizeAllElements-1;
    unsigned int offset = count * Data_Array[i].sizeElement;  // Определяем смещение на элемент, который надо достать
    void* dst = (void*)((byte_ptr)Data_Array[i].Data + offset); // Записываем адрес памяти свободной ячейки
    memCpy(returnValue, dst, Data_Array[i].sizeElement);   // Если структура данных найдена, читаем от туда первый (самый старый) элемент
    if(flag_int) INTERRUPT_ENABLE;  // Если все происходило не в прерывании восстанавливаем разрешение прерываний
    return EVERYTHING_IS_OK;   // Если все впорядке возвращаем ноль
}

u08 peekFromEndData(void* returnValue, const void* Array)
{
    bool_t flag_int = FALSE;
    register u08 i = findNumberDataStruct(Array);
    if(i == ArraySize) return NOT_FAUND_DATA_STRUCT_ERROR;    // Если в массиве нет искомой абстрактной структуры данных с заданным идентификатором
    if(Data_Array[i].lastCount == Data_Array[i].firstCount) return OVERFLOW_OR_EMPTY_ERROR; //Проверка пустая ли структура данных
    if(INTERRUPT_STATUS)
    {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    unsigned int offset = Data_Array[i].lastCount * Data_Array[i].sizeElement;
    void* src = (void*)((byte_ptr)Data_Array[i].Data+offset);
    memCpy(returnValue, src, Data_Array[i].sizeElement);   // Если структура данных найдена, читаем от туда первый (самый старый) элемент
    if(flag_int) INTERRUPT_ENABLE;  // Если все происходило не в прерывании восстанавливаем разрешение прерываний
    return EVERYTHING_IS_OK;   // Если все впорядке возвращаем ноль
}

void clearDataStruct(const void * const Data){
    bool_t flag_int = FALSE;
    register u08 i = findNumberDataStruct(Data);
    if(i == ArraySize) return;
    if(INTERRUPT_STATUS) {
        flag_int = TRUE;
        INTERRUPT_DISABLE;
    }
    Data_Array[i].firstCount = Data_Array[i].sizeAllElements; // Очищаем от данных наш массив
    Data_Array[i].lastCount = Data_Array[i].sizeAllElements;
    if(flag_int) INTERRUPT_ENABLE;  // Если все происходило не в прерывании восстанавливаем разрешение прерываний
}

bool_t isEmptyDataStruct(const void* const Data){
    register u08 i = findNumberDataStruct(Data);
    if(i == ArraySize) return TRUE; // Если такой структуры нет она точно пустая
    bool_t res = (Data_Array[i].firstCount == Data_Array[i].lastCount); // Если они равны друг другу значит пустая
    return res;
}

BaseSize_t getCurrentSizeDataStruct(const void* const Data) {
	register u08 i = findNumberDataStruct(Data);
	if(i == ArraySize) return 0;
	BaseSize_t first = 0;
	BaseSize_t last = 0;
	while(first != Data_Array[i].firstCount) first = Data_Array[i].firstCount;
	while(last != Data_Array[i].lastCount) last = Data_Array[i].lastCount;
	if(last > first) return (first + (Data_Array[i].sizeAllElements-last) + 1);
	if(first > last) return (first - last);
	return 0;
}

void for_each(const void* const Array, TaskMng tsk) {
    register u08 i = findNumberDataStruct(Array);
    if(i == ArraySize) return;
    for(BaseSize_t j=Data_Array[i].firstCount; j!=Data_Array[i].lastCount;)
    {
      BaseParam_t ptr = (BaseParam_t)((byte_ptr)Data_Array[i].Data + j*Data_Array[i].sizeElement);
      tsk(0,ptr);
      j=(j < Data_Array[i].sizeAllElements)? j+1:0;
    }
}

#endif //DATA_STRUCT_MANAGER


#ifdef __cplusplus
}
#endif
