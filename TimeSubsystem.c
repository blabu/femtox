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
 * TimeSubsystem.c
 *
 *  Created on: 28 квіт. 2017 р.
 *      Author: oleksiy.khanin
 */
#include "TaskMngr.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CLOCK_SERVICE

volatile Time_t __systemSeconds = 0;

#define SECONDS_2000 946684800UL  /*Колличество секунд с 1970 по 2000*/
#define SECONDS_IN_YEAR 31536000UL
#define SECONDS_IN_DAY 86400UL

#if TIME_INDEX!=0
static volatile s08 timeCorrectSummer = TIME_INDEX;
#endif

#define JANUARY   31
#define FEBRUARY  28
#define MARCH	  31
#define APRIL     30
#define MAY	  	  31
#define JUNE      30
#define JULY	  31
#define AUGUST	  31
#define SEPTEMBER 30
#define OCTOBOR	  31
#define NOVEMBER  30
#define DECEMBER  31
const u08 daysInMounth[12] = {JANUARY,FEBRUARY,MARCH,APRIL,MAY,JUNE,JULY,AUGUST,SEPTEMBER,OCTOBOR,NOVEMBER,DECEMBER};

static u16 toStandartYear(u16 year) {
	u16 result = year;
	if(year > 70 && year<100)
		return result+1900;
	else if(year < 70) return result+2000;
	return result;
}

static  u08 getMinutesFromSec(Time_t sec) {
	sec = sec/60; // Определяем сколько всего минут прошло
	sec %= 60; // от 0 до 59 минут
	return (u08)sec;
}

static u08 getHourFromSec(Time_t sec) {
	sec = (sec/3600UL); // Определяем сколько всего часов прошло
	sec %= 24; // от 0 до 23 часов
	return (u08)sec;
}

static bool_t isUpYear(u16 year) { // Высокосный ли год
	if(year & 0x03) return FALSE;
	return TRUE;
}

static u08 getDayOffset(u16 year) {
	u08 dataOffset = 0;
	if(year > 1972) // Больше первого высокосного года
		dataOffset = ((year - 1972)>>2);
	return dataOffset;
}

static u16 getDayInYearFromSec(Time_t sec) { // День в году
	u16 days = (u16)(sec/SECONDS_IN_DAY); // Кол-во дней с 1970 года
	u16 year = 1970;
	while(days > 365) {
		days-=365; year++;
	}
	u08 dataOffset = getDayOffset(year);
	if(days <= dataOffset) {
		year--;
		if(!isUpYear(year)) days += 365;
		else days += 366;
	}
	days -= dataOffset;
	return days;
}

static u16 getDayAndMonthFromDayInYear(u16 dayInYear) { //LSB - day, MSB - mounth
	u08 month=0;
	for(; month<12; month++) {
		if(dayInYear > daysInMounth[month]) {
			dayInYear -= ( daysInMounth[month] );
		}
		else break;
	}
	u08 day = dayInYear & 0x1F; // 32 (Обнуляем все старшие биты) на всякий случай
	month+=1;	// Чтобы год начинался с 1-го месяца
#if TIME_INDEX!=0
#ifdef SUMMER_TIME
	//Если месяц больше марта (т.е. апрель или дальше) и меньше ноября (т.е. окябрь или меньше)
	if(month > 3 && month < 11) timeCorrectSummer = TIME_INDEX+1;
	else timeCorrectSummer = TIME_INDEX;
#else
	timeCorrectSummer = TIME_INDEX;
#endif
#endif
	return ((u16)month<<8) | day;
}

static u16 getDayInYearFromDate(const Date_t*const d) {
	u16 day = 1;
	u08 mounth = d->mon;
	while(--mounth) {
		day += daysInMounth[mounth];
	}
	if(day) day--;
	day += d->day;
	return day;
}


Time_t getAllSeconds(void){
	Time_t temp = 0;
	while(temp != __systemSeconds) temp = __systemSeconds;
	return temp;
}

u08 getMinutes(void){
	return getMinutesFromSec(getAllSeconds());
}

u08 getHour(void){
    Time_t sec = getAllSeconds();
	u08 nowHour = getHourFromSec(sec);
#if TIME_INDEX!=0
	nowHour += timeCorrectSummer;
#endif
	if(nowHour > 23) nowHour -= 24; // Если часы равны 24 и более
	return nowHour;
}

u16 getDayInYear(void) { // День в году
	return getDayInYearFromSec(getAllSeconds());
}

//LSB - day, MSB - mounth
u16 getDayAndMonth(void) {
	return getDayAndMonthFromDayInYear(getDayInYear());
}

u08 getDaysInMonth(u08 month) {
	if(month) month--;
	return daysInMounth[month];
}

void setSeconds(const u32 sec) {
	while(__systemSeconds != sec) __systemSeconds = sec;
#if TIME_INDEX!=0
#ifdef SUMMER_TIME
	getDayAndMonth();
#endif
#endif
}

Date_t getDateFromSeconds(Time_t sec, bool_t toLocalTimeZone){
	Date_t res;
	u32 minutes, hours, days, year, month;
	minutes  = sec / 60;
	sec -= minutes * 60;
	hours    = minutes / 60;
	minutes -= hours   * 60;
	days     = hours   / 24;
	hours   -= days    * 24;
	year     = 1970;
	while(1) {
		bool_t leapYear = isUpYear(year);
		u16 daysInYear = leapYear ? 366 : 365;
		if (days >= daysInYear) {
			days -= daysInYear;
			++year;
		}
		else {
			for(month = 0; month < 12; ++month) {
				u08 dim = daysInMounth[month];
				if (month==1 && leapYear) ++dim;
				if (days >= dim) days -= dim;
				else
					break;
			}
			break;
		}
	}
	res.sec = (u08)sec; res.min = (u08)minutes; res.hour = (u08)hours;
	res.day = (u08)days+1;  res.mon = (u08)month+1;   res.year = (u16)year;
	if(toLocalTimeZone) {
#if TIME_INDEX!=0
#ifdef SUMMER_TIME
	if(res.mon > 3 && res.mon < 11) timeCorrectSummer = TIME_INDEX+1;
	else timeCorrectSummer = TIME_INDEX;
#else
	timeCorrectSummer = TIME_INDEX;
#endif
	for(u08 i = timeCorrectSummer; i; i--) addOneHourToDate(&res);
#endif
	}
	return res;
}

Time_t getSecondsFromDate(const Date_t*const date) {
	u16 year = toStandartYear(date->year);
	u08 dayOffset = getDayOffset(year); // Поправка в днях на высокосные годы
#if TIME_INDEX!=0
	const u08 tCorrectSummer = timeCorrectSummer;
#else
	const u08 tCorrectSummer = 0;
#endif
	year -= 1970;
	Time_t tempSeconds = (Time_t)year*SECONDS_IN_YEAR +
						 ((u32)getDayInYearFromDate(date) + dayOffset)*SECONDS_IN_DAY +
						 (u32)(date->hour)*3600UL + (u32)(date->min)*60UL + date->sec -
/*Корректировка времени*/tCorrectSummer*3600UL;
	return tempSeconds;
}

void addOneDayToDate(Date_t* date) {
	if(date->day < daysInMounth[date->mon-1]) date->day++;
	else {date->day = 1; if(date->mon < 12) date->mon++;
	else {date->mon = 1;  date->year++;}}
}

void addOneHourToDate(Date_t* date){
	if(date->hour < 23) date->hour++;
	else {date->hour = 0; addOneDayToDate(date);}
}

void addOneMinutesToDate(Date_t* date){
	if(date->min < 59) date->min++;
	else {date->min = 0;  addOneHourToDate(date);}
}

void addOneSecondToDate(Date_t* date){
	if(date->sec < 59) date->sec++;
	else {date->sec = 0; addOneMinutesToDate(date);}
}

void subOneDayFromDate(Date_t * date) {
	if(date->day > 1) date->day--;
	else if(date->mon > 1) {
		date->mon--;
		date->day = daysInMounth[date->mon];
	}
	else {date->mon = 12;  date->year--;}
}

/*
 * return >0 if date1 > date2
 * return 0 if date = date2
 * return <0 if date1 < date2
 */
s08 compareDates(const Date_t*const date1, const Date_t*const date2){
	if(date1 == NULL || date2 == NULL) return 0;
	u16 yearDate1 = toStandartYear(date1->year);
	u16 yearDate2 = toStandartYear(date2->year);
	s16 different = yearDate1 - yearDate2;
	if(different) return (s08)(different);
	different = date1->mon - date2->mon;
	if(different) return (s08)(different);
	different = date1->day - date2->day;
	if(different) return (s08)(different);
	different = date1->hour - date2->hour;
	if(different) return (s08)(different);
	different = date1->min - date2->min;
	if(different) return (s08)(different);
	different = date1->sec - date2->sec;
	if(different) return (s08)(different);
	return 0;
}

#include "MyString.h"
// input date must have format YY.MM.DD hh:mm:ss
void setDate(string_t date) {
	char tempStr[4];
	Date_t resultData;
	if(strSize(date) < 17) { return;}
	strCopy(tempStr,date,2,0);
	resultData.year = (u16)toIntDec(tempStr);
	resultData.year = toStandartYear(resultData.year);
	strClear(tempStr);
	strCopy(tempStr,date,2,3);
	resultData.mon = (u08)toIntDec(tempStr);
	strClear(tempStr);
	strCopy(tempStr,date,2,6);
	resultData.day = (u08)toIntDec(tempStr);
	strClear(tempStr);
	strCopy(tempStr,date,2,9);
	resultData.hour = (u08)toIntDec(tempStr);
	strClear(tempStr);
	strCopy(tempStr,date,2,12);
	resultData.min = (u08)toIntDec(tempStr);
	strClear(tempStr);
	strCopy(tempStr,date,2,15);
	resultData.sec = (u08)toIntDec(tempStr);
	Time_t tempSeconds = getSecondsFromDate(&resultData);
	setSeconds(tempSeconds);
}

void dateToString(string_t out, Date_t* date) {
	if(out == NULL) return;
	char temp[8]; temp[0] = 0;
	u16 year = date->year%100;
	toStringUnsignDec(year,temp);
	while(strSize(temp) < 2) { temp[2] = 0; temp[1] = temp[0]; temp[0] = '0';}
	strCat(out,temp);
	strCat(out,".");
	temp[0] = 0;
	toStringUnsignDec(date->mon,temp);
	while(strSize(temp) < 2) { temp[2] = 0; temp[1] = temp[0]; temp[0] = '0';}
	strCat(out,temp);
	strCat(out,".");
	temp[0] = 0;
	toStringUnsignDec(date->day,temp);
	while(strSize(temp) < 2) { temp[2] = 0; temp[1] = temp[0]; temp[0] = '0';}
	strCat(out,temp);
	strCat(out," ");
	temp[0] = 0;
	toStringUnsignDec(date->hour,temp);
	while(strSize(temp) < 2) { temp[2] = 0; temp[1] = temp[0]; temp[0] = '0';}
	strCat(out,temp);
	strCat(out,":");
	temp[0] = 0;
	toStringUnsignDec(date->min,temp);
	while(strSize(temp) < 2) { temp[2] = 0; temp[1] = temp[0]; temp[0] = '0';}
	strCat(out,temp);
	strCat(out,":");
	temp[0] = 0;
	toStringUnsignDec(date->sec,temp);
	while(strSize(temp) < 2) { temp[2] = 0; temp[1] = temp[0]; temp[0] = '0';}
	strCat(out,temp);
}

#endif
#ifdef __cplusplus
}
#endif
