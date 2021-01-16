#ifndef MY_STRING_H
#define _STRING_H
#include "FemtoxTypes.h"

#define END_STRING '\0'

// command - строка, которую ищут
// answer  - строка, в которой ищут
// Функция возвращает TRUE если найдено
// Если не нашли FALSE
bool_t str1_str2(const string_t small, const string_t big);

bool_t startWith(const string_t str, const string_t starts);

bool_t strCompare(const string_t str1, const string_t str2);

// small - строка, которую ищут
// big  - строка, в которой ищут
// Функция возвращает адресс начала входа подстроки в строку
// Если не нашли вернем отрицательное число
s16 findStr(const string_t small, const string_t big);

// Ищет символ symb в строке c_str
// Если не найдет вернет отрицательное число
s16 findSymb(const char symb, const string_t c_str);

// Добавляет к строке str1 строку str2
// Размер строки str1 должен уместить str2
void strCat(string_t c_str1, const string_t c_str2);

//Копирует numb байт строки c_str начиная с позиции pos в строку result.
void strCopy(string_t result, const string_t c_str, BaseSize_t numb, BaseSize_t pos);
char* myStrcpy(string_t destination, const string_t source);
void strClear(string_t str);

// Вернет кол-во замен (в строке c_str, но не больше размера size, если size==0 тогда до конца строки)
BaseSize_t replaceAllSymbols(string_t c_str,const char origin, const char replaced, BaseSize_t size);

/*
Разбивает строку на подстроки. Заменяет символ delim концом ситроки. Вернет кол-во подстрок в строке
EXAMPLE
    char str[] = "1122312;2;2;0;0;0;600.00;130.00;600.00;50.00;0.00;-20.00;50.00;3.00;12.50;250.00;1.00;600.00;-20.00";
    BaseSize_t n = strSplit(';',str);
    string_t tempStr = str;
    for(BaseSize_t i = 0; i<n; i++) {
        writeLogStr(tempStr);
        u08 sz = strSize(tempStr);
        tempStr = &tempStr[sz+1];
    }
*/
BaseSize_t strSplit(char delim, const string_t c_str);

//Вернет размер строки
BaseSize_t toUpperCase(const string_t str);

//Вернет размер строки
BaseSize_t toLowerCase(const string_t str);

// razryad - количество знаков, например для чисел от 10 до 99 razryad = 2.
s64 toIntDec(const string_t c_str);

s64 toInt64(const string_t c_str);
// str - строка с символьной шестнадцатиричной переменной
s32 toInt32(const string_t c_str);

s08 toInt08(const string_t c_str);

s16 toInt16(const string_t c_str);

double toDouble(const string_t c_str);

// Переводит число в строку шестнадцатиричного формата
void toString(u08 capacity, s64 data, string_t c_str);
void toStringUnsign(u08 capacity, u64 data, string_t c_str);

void toStringDec(s64 data, string_t c_str);

bool_t isDigitDec(const char symb);
bool_t isDigit(const char symb);
bool_t isAsciiOrNumb(const char symb);
#ifndef _X86
//#define isalnum(cczz) isAsciiOrNumb((const char)cczz)
#endif

void toStringUnsignDec(u64 data, string_t c_str);

void doubleToString(double data, string_t c_str, u08 precision);

void shiftStringLeft(BaseSize_t poz, const string_t c_str);
void shiftStringRight(BaseSize_t poz, const string_t c_str);

void fillRightStr(u16 size, const string_t str, char symb);

//Вернет размер строки
BaseSize_t strSize(const string_t c_str);

/*
 * Пока поддерживаются
 * %B, unsigned u08
 * %I, unsigned u16
 * %D, unsigned u32
 * %L, unsigned u64
 * %b, signed s08
 * %i, signed s16
 * %d, signed s32
 * %l, signed s64
 * %x, hex unsigned (u08)
 * %X, hex unsigned (u32)
 * %c, symb
 * %s, string
 * %F, float,
 * */
void Sprintf(const string_t result, const string_t paternStr, void** params);

#endif // MY_STRING_H
