// Файл работы со строками
#include "MyString.h"
#include "TaskMngr.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
command - строка, которую ищут
answer  - строка, в которой ищут
*/
s16 findStr(const string_t small, const string_t big){
	if(small != NULL && big != NULL) {
		BaseSize_t small_len = strSize(small);
		BaseSize_t len = strSize(big);
		if(small_len>len) return -1;   // Если подстрока длинее строки возвращаем 0
		register BaseSize_t i = 0, j=0;
		for(; i<len; i++) // перебираем строку в которой ишем
		{
			if(big[i] == small[j]){   // Если текущий символ исходной строки совпал с первым символом сравниваемой строки
				if(j == small_len-1) return (i-j);    // Если все символы совпали и шли друг за другом
				j++;                      // Увеличиваем счетчик совпавших символов
			}
			else if(j){
				j=0; i--;
			}
		}
	}
    return -1;
}

bool_t str1_str2(const string_t small, const string_t big){ // Функция возвращает TRUE если small является подстрокой big
  if(findStr(small,big)<0) return FALSE;
  return TRUE;
}

bool_t strCompare(const string_t str1, const string_t str2){
	u08 size1 = strSize(str1);
	u08 size2 = strSize(str2);
	if(size1 != size2) return FALSE;
	for(u08 i = 0; i< size1; i++) {
		if(str1[i] != str2[i]) return FALSE;
	}
	return TRUE;
}

// Ищет символ symb в строке c_str (вернет позицию этого символа в строке) Если не нашло вернет отрицательный результат
s16 findSymb(const char symb, const string_t c_str){
	  if(c_str == NULL) return -1;
      BaseSize_t i= 0;
      while(c_str[i] != END_STRING)
      {
        if(c_str[i] == symb) return i;
        i++;
      }
      return -1;
}

/*
Вернет размер строки
*/
BaseSize_t strSize(const string_t c_str){
	if(c_str == NULL) return 0;
	BaseSize_t i = 0;
	while(c_str[i] != END_STRING){
		i++;
		if(!i) break; // Если переполнился выходим из функции с результатом 0 (не нашли конца строки)
	}
	return i;
}

/*
Добавляет к строке str1 строку str2
Размер строки str1 должен уместить str2
*/
void strCat(string_t c_str1, const string_t c_str2){
  BaseSize_t i = 0, j=0;
  if(c_str1 == NULL || c_str2 == NULL) return;
  while(c_str1[i] != END_STRING) ++i; // Ищем конец строки 1
  while(c_str2[j] != END_STRING){
    c_str1[i] = c_str2[j];
    ++j;
    ++i;
  }
  c_str1[i] = END_STRING;
}

void strCopy(string_t result, const string_t c_str, BaseSize_t numb, BaseSize_t pos){
    BaseSize_t i=0;
    if(result == NULL || c_str == NULL) return;
    if(!numb) return;
    do{
        result[i] = c_str[pos];  // Если исходная строка еще есть копируем
        if(c_str[pos] == END_STRING) return;
        pos++,numb--;i++;
    }while(numb);
    result[i]=END_STRING;
}

char* strcpy (string_t destination, const string_t source) {
	BaseSize_t size = strSize(source) + 1; // End byte too
	memCpy(destination, source, size);
	return destination;
}

void strClear(string_t str){
	if(str == NULL) return;
	str[0]='\0';
}

// Вернет кол-во замен (в строке c_str, но не больше размера size, если size==0 тогда до конца строки)
BaseSize_t replaceAllSymbols(string_t c_str,const char origin, const char replaced, BaseSize_t size) {
	if(c_str != NULL) {
		BaseSize_t i = 0;
		BaseSize_t numb = 0;
	    while(c_str[i] != END_STRING) {
	    	size--;
	        if(c_str[i] == origin) {
	                c_str[i] = replaced;
	                numb++;
	        }
	        i++;
	        if(!i) return 0; // Произошло переполнения (строка слишком длинная)
	        if(!size) break;
	    }
	    return numb;
	}
	return 0;
}

// Разбивает строку на подстроки. Заменяет символ delim концом ситроки. Вернет кол-во подстрок в строке
BaseSize_t strSplit(char delim, string_t c_str) {
    BaseSize_t i = 0;
    if(c_str == NULL) return 0;
    BaseSize_t numb = replaceAllSymbols(c_str,delim,END_STRING,0) + 1;
    return numb;
}

void toStringUnsignDec(u64 data, string_t c_str){
	u08 size = 0;
	u64 offset = 0;
	if(data<10) size = 1;
	else if(data<100) { size = 2; offset = 10; }
	else if(data<1000UL){ size = 3; offset = 100; }
	else if(data<10000UL){ size = 4; offset = 1000UL; }
	else if(data<100000UL){ size = 5; offset = 10000UL; }
	else if(data<1000000UL){ size = 6; offset = 100000UL; }
	else if(data<10000000UL){ size = 7; offset = 1000000UL; }
	else if(data<100000000UL){ size = 8; offset = 10000000UL; }
	else if(data<1000000000ULL){ size = 9; offset = 100000000ULL; }
	else if(data<10000000000ULL){ size = 10; offset = 1000000000ULL; }
	else if(data<100000000000ULL){ size = 11; offset = 10000000000ULL; }
	else if(data<1000000000000ULL){ size = 12; offset = 100000000000ULL; }
	else if(data<10000000000000ULL){ size = 13; offset = 1000000000000ULL; }
	else if(data<100000000000000ULL){ size = 14; offset = 10000000000000ULL; }
	else if(data<1000000000000000ULL){ size = 15; offset = 100000000000000ULL; }
	else if(data<10000000000000000ULL){ size = 16; offset = 1000000000000000ULL; }
	else {MaximizeErrorHandler("toStringUnsignDec size is too long"); return;}
	u08 i = 0;
	while(size) {
		if(size != 1){
				 c_str[i] = (data/offset) + 0x30;
				 data %= offset;
				 offset /= 10;
		}
		else c_str[i] = (data+0x30)&0xFF;
		size--;
		i++;
	}
	c_str[i] = END_STRING;
}

void toStringDec(s64 data, string_t c_str) {
	if(data<0) {
		c_str[0] ='-';
		data = -data;
		toStringUnsignDec(data, c_str+1);
		return;
	}
	toStringUnsignDec(data,c_str);
}

void toStringUnsign(u08 capacity, u64 data, string_t c_str){
    u08 size = (capacity)<<1; // Размер выходной строки
    u08 j = 0;
    for(;size != 0; size--) {
    	u08 offset = (size-1)<<2; // Расчитываем смещение
        unsigned char halfbyte = ((data>>offset) & 0x0F); // Берем старшие четыре бита
        if(halfbyte < 10) {
        	if(halfbyte == 0 && j == 0) continue;
            c_str[j] = (char)(halfbyte + '0');
            j++;
        }
        else {
            c_str[j] = (char)(halfbyte + 'A' - 10);
            j++;
        }
    }
    if(j==0) {c_str[j]='0'; j++;}
    c_str[j]=END_STRING;
}

//Вернет размер строки
BaseSize_t toUpperCase(string_t str) {
	if(str != NULL) {
		BaseSize_t i = 0;
		while(str[i] != END_STRING) {
			if(str[i] >= 'a' &&
					str[i] <= 'z') {
				str[i] = str[i]-'a'+'A';
			}
			i++;
			if(!i) break;  // Строка слишком большая
		}
		return i;
	}
	return 0;
}

//Вернет размер строки
BaseSize_t toLowerCase(string_t str) {
	if(str != NULL) {
	BaseSize_t i = 0;
	while(str[i] != END_STRING) {
		if(str[i] >= 'A' &&
		   str[i] <= 'Z') {
			str[i] = str[i]-'A'+'a';
		}
		i++;
		if(!i) break; // Строка слишком большая
	}
	return i;
	}
	return 0;
}

void toString(u08 capacity, s64 data, string_t c_str){
	if(data<0) {
		 c_str[0] = '-';
		 data=-data;
		 toStringUnsign(capacity,data,c_str+1);
	}else toStringUnsign(capacity,data,c_str);
}

static s64 toInt(s08 razryad, const string_t c_str){
    s64 res = 0;
    if(c_str == NULL) return res;
    bool_t sign = FALSE;
    BaseSize_t i = 0;
    if(c_str[0] == '-') {sign=TRUE; i = 1;}
    razryad <<= 1; // В каждом разряде по две цифры 0хFF
    while(c_str[i] != END_STRING) {
        if(c_str[i] >= '0' && c_str[i] <= '9') {
            res <<= 4;
            res |= c_str[i] - '0';
            razryad--;
        }else if(c_str[i] >= 'A' && c_str[i] <= 'F') {
            res <<= 4;
            res |= c_str[i]-'A'+10;
            razryad--;
        }else if(c_str[i] >= 'a' && c_str[i] <= 'f') {
        	res <<= 4;
        	res |= c_str[i]-'a'+10;
        	razryad--;
        }
        else if(res != 0) break;
        if(razryad <= 0) break;
        i++;
    }
    if(sign) res = -res;
    return res;
}

s64 toIntDec(const string_t c_str) {
	s64 res = 0;
	if(c_str == NULL) return res;  // В каждом разряде по две цифры
	s08 razryad = strSize(c_str);
    bool_t sign = FALSE;
    BaseSize_t i = 0;
    if(c_str[0] == '-') {sign=TRUE; i = 1;}
	razryad <<= 1;
	while(c_str[i] != END_STRING) {
		if(c_str[i] >= '0' && c_str[i] <= '9') { // Значит валидный знак
			res *= 10;
			res += (c_str[i]-'0');
			razryad--;
		}
		else if(res != 0) break;
        if(razryad <= 0) break;
        i++;
	}
	if(sign) res=-res;
	return res;
}


s64 toInt64(const string_t c_str){
    s64 res = strSize(c_str);
    if(res<1) return 0;
    res = toInt(8,c_str);
    return res;
}


s32 toInt32(const string_t c_str){
    s32 res = strSize(c_str);
    if(res<1) return 0;
    res = toInt(4,c_str);
    return res;
}

s16 toInt16(const string_t c_str){
	s16 res = strSize(c_str);
	if(res<1) return 0;
	res = toInt(2,c_str);
	return res;
}

s08 toInt08(const string_t c_str){
	s08 res = strSize(c_str);
	if(res<1) return 0;
	res = toInt(1,c_str);
	return res;
}

double toDouble(const string_t c_str) {
	s32 whole = toIntDec(c_str);
	s16 poz = findSymb('.',c_str);
	if(poz < 0) {
		poz = findSymb(',',c_str);
		if(poz < 0) return (double)whole;
	}
	double fract = (double)toIntDec(c_str+poz+1);
	while(fract > 1) {
		fract /= 10;
	}
	return (double)(whole+fract);
}

bool_t isDigit(const char symb) {
	if(symb < '0') return FALSE;
	if(symb > '9' && symb < 'A') return FALSE;
	if(symb > 'F') return FALSE;
	return TRUE;
}

bool_t isAsciiOrNumb(const char symb) {
	if(symb >= '0' && symb <= '9') return TRUE;
	if(symb >= 'A' && symb <= 'Z') return TRUE;
	if(symb >= 'a' && symb <= 'z') return TRUE;
	return FALSE;
}

void shiftStringLeft(BaseSize_t poz, string_t c_str){
  BaseSize_t size = strSize(c_str);
  BaseSize_t i=0;
  while(poz<size){
    c_str[i]=c_str[poz];
    i++, poz++;
  }
  c_str[i] = END_STRING;
}

void shiftStringRight(BaseSize_t poz, string_t c_str) {
	BaseSize_t size = strSize(c_str)+2; // With END_STRING
	poz += size;
	while(size) {
        poz--; size--;
		c_str[poz] = c_str[size];
	}
}


void doubleToString(double data, string_t c_str, u08 precision) {
	if(c_str == NULL) return;
	s32 wholePart = (s32)data; // выделяем целую часть
	double fraction = data - wholePart;	// выделяем дробную часть
	toStringDec(wholePart,c_str);
	if(fraction < 0) { fraction = -fraction; }
	if (precision > 0) {
		string_t endString = c_str;
		while(*endString != END_STRING) endString++;
		*endString = '.';
		endString++;
		while(precision > 0) {
			precision--;
			fraction *= 10;
			u08 d1 = ((u08)fraction) & 0x0F; // 0 - 9
			if(d1 < 10) *endString++ = '0' + d1; // '0' - '9'
			fraction -= d1;
		}
		*endString = END_STRING;
	}
}

// Заполняет строку одним символом справа
// Например: исходная строка "113" после выполнения этой функции строка может быть "00113"
void fillRightStr(u16 size, string_t str, char symb) {
	s16 s = size - strSize(str);
	if(s <= 0) return;
	shiftStringRight(s,str);
	for(s16 i = 0; i<s; i++) {
		str[i] = symb;
	}
}

/* Печать в строку
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
 * %F, float,
 * %c, symb
 * %s, string
 * */
void Sprintf(string_t result, const string_t paternStr, void** params) {
	if(result == NULL || paternStr == NULL) return;
	if(params == NULL) {
		u08 size = strSize(paternStr);
		strCopy(result, paternStr, size, 0);
		return;
	}
	#undef TEMP_BUF_SIZE
	#define TEMP_BUF_SIZE 12
	result[0] = END_STRING;
	char tempBuf[TEMP_BUF_SIZE];
	u08 currentPozTempBuf = 0;
	u08 i = 0;
	while(paternStr[i] != END_STRING) {
		if(paternStr[i] == '%') {
			if(currentPozTempBuf > 0) {
				tempBuf[currentPozTempBuf] = END_STRING;
				strCat(result,tempBuf);
				currentPozTempBuf = 0;
			}
			i++;
			switch(paternStr[i]) {
			case 'B':
				toStringDec(**((u08**)params), tempBuf);
				strCat(result, tempBuf);
				break;
			case 'I':
				toStringDec(**((u16**)params), tempBuf);
				strCat(result, tempBuf);
				break;
			case 'D':
				toStringDec(**((u32**)params), tempBuf);
				strCat(result, tempBuf);
				break;
			case 'L':
				toStringDec(**((u64**)params), tempBuf);
				strCat(result, tempBuf);
				break;
			case 'b':
				toStringDec(**((s08**)params), tempBuf);
				strCat(result, tempBuf);
				break;
			case 'i':
				toStringDec(**((s16**)params), tempBuf);
				strCat(result, tempBuf);
				break;
			case 'd':
				toStringDec(**((s32**)params), tempBuf);
				strCat(result, tempBuf);
				break;
			case 'l':
				toStringDec(**((s64**)params), tempBuf);
				strCat(result, tempBuf);
				break;
			case 'x':
				toString(4, **((u08**)params), tempBuf);
				strCat(result, tempBuf);
				break;
			case 'X':
				toString(4, **((u32**)params), tempBuf);
				strCat(result, tempBuf);
				break;
			case 's':
				strCat(result, (string_t)(*params));
				break;
			case 'c':
				tempBuf[currentPozTempBuf] = **(char**)params;
				currentPozTempBuf++;
				break;
			case 'F':
			    doubleToString(**((float**)params), tempBuf, 2);
			    strCat(result, tempBuf);
			    break;
			default:
				strCat(result," !unsupported! ");
			}
			params++;
		} else
		{
			if(currentPozTempBuf < TEMP_BUF_SIZE-2) {
				tempBuf[currentPozTempBuf] = paternStr[i];
				currentPozTempBuf++;
			} else {
				tempBuf[currentPozTempBuf] = paternStr[i];
				tempBuf[currentPozTempBuf+1] = END_STRING;
				strCat(result,tempBuf);
				currentPozTempBuf = 0;
			}
		}
		i++;
	}
	if(currentPozTempBuf>0) {
		tempBuf[currentPozTempBuf] = END_STRING;
		strCat(result,tempBuf);
	}
}

#ifdef __cplusplus
}
#endif
