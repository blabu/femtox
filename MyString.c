// Файл работы со строками
#include "MyString.h"

#define END_STRING '\0'
/*
command - строка, которую ищут
answer  - строка, в которой ищут
*/
s08 findStr(const string_t small, const string_t big){
    BaseSize_t small_len = strSize(small);
    BaseSize_t len = strSize(big);
    if(small_len>len) return -1;   // Если подстрока длинее строки возвращаем 0
    register BaseSize_t i = 0, j=0;
    for(; i<len; i++) // перебираем строку в которой ишем
    {
        if(big[i] == small[j])   // Если текущий символ исходной строки совпал с первым символом сравниваемой строки
        {
            if(j == small_len-1) return (i-j);    // Если все символы совпали и шли друг за другом
            j++;                      // Увеличиваем счетчик совпавших символов
        }
        else j=0;
    }
    return -1;
} 

bool_t str1_str2(const string_t small, const string_t big){ // Функция возвращает TRUE если command является подстрокой answer
  if(findStr(small,big)<0) return FALSE;
  return TRUE;
}
// Ищет символ symb в строке c_str (вернет позицию этого символа в строке) Если не нашло вернет отрицательный результат
s08 findSymb(const char symb, const string_t c_str){
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
  u08 i = 0;
  while(c_str[i] != END_STRING)
  {
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
  while(c_str1[i] != END_STRING) ++i; // Ищем конец строки 1
  while(c_str2[j] != END_STRING)
  {
    c_str1[i] = c_str2[j];
    ++j;
    ++i;
  }
  c_str1[i] = END_STRING;
}

void strCopy(string_t result, const string_t c_str, BaseSize_t numb, BaseSize_t pos){
    u08 i=0;
    u08 length = strSize(c_str);
    if(!numb) return;
    do{
        if(pos==length) break;    // Если исходная строка закончилась выходим
        result[i] = c_str[pos];  // Если исходная строка еще есть копируем
        pos++,numb--;i++;
    }while(numb);
    result[i]=END_STRING;
}

void toString(u32 n, string_t c_str){
    BaseSize_t size = (sizeof(n))<<1;
    BaseSize_t j = 0;
    for(;size != 0; size--)
    {
        unsigned char halfbyte = (n>>((size-1)<<2)) & 0x0F;
        if(halfbyte < 10)
        {
            if(halfbyte == 0 && j==0) continue;
            c_str[j] = (char)(halfbyte + '0');
            j++;
        }
        else
        {
            c_str[j] = (char)(halfbyte + 'A' - 10);
            j++;
        }
    }
    c_str[j]=END_STRING;
}

u32 toInt(const string_t c_str){
    u32 res = 0;
    s08 j = (sizeof(res))<<1;
    BaseSize_t i = 0;
    while(c_str[i] != END_STRING)
    {
        if(c_str[i] >= '0' && c_str[i] <= '9')
        {
            res <<= 4;
            res |= c_str[i] - '0';
            j--;
        }
        else if(c_str[i] >= 'A' && c_str[i] <= 'F')
        {
            res <<= 4;
            res |= c_str[i]-'A'+10;
            j--;
        }
        else if(res != 0) break;
        i++;
        if(j<0) break;
    }
    return res;
}

void shiftString(BaseSize_t poz, string_t c_str){
  BaseSize_t size = strSize(c_str);
  BaseSize_t i=0;
  while(poz<size){
    c_str[i]=c_str[poz];
    i++, poz++;
  }
  c_str[i] = '\0';
}