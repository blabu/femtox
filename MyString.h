#ifndef MY_STRING_H
#define MY_STRING_H
#include "TaskMngr.h"

// command - строка, которую ищут
// answer  - строка, в которой ищут
// Функция возвращает TRUE если найдено
// Если не нашли FALSE
bool_t str1_str2(const string_t small, const string_t big);

bool_t strCompare(const string_t str1, const string_t str2);

// command - строка, которую ищут
// answer  - строка, в которой ищут
// Функция возвращает адресс начала входа подстроки в строку
// Если не нашли вернем отрицательное число
s08 findStr(const string_t small, const string_t big);

// Ищет символ symb в строке c_str
// Если не найдет вернет отрицательное число
s08 findSymb(const char symb, const string_t c_str);

// Добавляет к строке str1 строку str2
// Размер строки str1 должен уместить str2
void strCat(string_t c_str1, const string_t c_str2);

//Копирует numb байт строки c_str начиная с позиции pos в строку result.
void strCopy(string_t result, const string_t c_str, BaseSize_t numb, BaseSize_t pos);

void strClear(string_t str);


// razryad - количество знаков, например для чисел от 10 до 99 razryad = 2.
s32 toIntDec(const string_t c_str);

// str - строка с символьной шестнадцатиричной переменной
s32 toInt32(const string_t c_str);

s08 toInt08(const string_t c_str);

s16 toInt16(const string_t c_str);

// Переводит число в строку шестнадцатиричного формата
void toString(u08 capacity, s32 data, string_t c_str);
void toStringUnsign(u08 capacity, u32 data, string_t c_str);

void toStringDec(s32 data, string_t c_str);

bool_t isDigit(const char symb);

void toStringUnsignDec(u32 data, string_t c_str);

void doubleToString(double data, string_t c_str, u08 precision);

void shiftStringLeft(BaseSize_t poz, string_t c_str);
void shiftStringRight(BaseSize_t poz, string_t c_str);

// Заменяет все символы в строке длиной size (если size==0 определит длину сама)
void replaceAllSymbols(string_t c_str, const char symbolOrigin, const char symbolReplacement, BaseSize_t size);

//Вернет размер строки
BaseSize_t strSize(const string_t c_str);

#endif // MY_STRING_H
