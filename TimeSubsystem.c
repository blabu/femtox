/*
 * TimeSubsystem.c
 *
 *  Created on: 28 квіт. 2017 р.
 *      Author: oleksiy.khanin
 */

#include "TaskMngr.h"
#include "PlatformSpecific.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CLOCK_SERVICE

volatile u32 seconds = 0;

#define SECONDS_2000 946684800UL  /*Колличество секунд с 1970 по 2000*/
#define SECONDS_IN_YEAR 31536000UL
#define SECONDS_IN_DAY 86400UL

#if TIME_INDEX>1
static u08 timeCorrectSummer = TIME_INDEX;
#endif

#define JANUARY   31
#define FEBRUARY  28
#define MARCH	  31
#define APRIL     30
#define MAY		  31
#define JUNE      30
#define JULY	  31
#define AUGUST	  31
#define SEPTEMBER 30
#define OCTOBOR	  31
#define NOVEMBER  30
#define DECEMBER  31
const u08 daysInYear[12] = {JANUARY,FEBRUARY,MARCH,APRIL,MAY,JUNE,JULY,AUGUST,SEPTEMBER,OCTOBOR,NOVEMBER,DECEMBER};

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

static u16 getYearFromSec(Time_t sec) {
	return (sec/SECONDS_IN_YEAR) + 1970;
}

static u16 getDayInYearFromSec(Time_t sec) { // День в году
	u16 result = (u16)(sec/SECONDS_IN_DAY); // Кол-во дней с 1970 года
	result %= 365;
	// Поправка на высокосные годы
	u16 year  = getYearFromSec(sec);
	u08 dataOffset = 0;
	if(year > 1972) // Больше первого высокосного года
	 	dataOffset = ((year - 1972)>>2);
	result -= dataOffset;
	return result;
}

static u16 getDayAndMonthFromDay(u16 dayInYear) { //LSB - day, MSB - mounth
	u08 month=0;
	u08 day = 0;
	for(; month<12; month++) {
		if(dayInYear > daysInYear[month]) {
			dayInYear -= ( daysInYear[month] );  // Учитываем что день в месяце начинается с 0
		}
		else break;
	}
	day = dayInYear & 0x1F; // 32 (Обнуляем все старшие биты) на всякий случай
	month+=1;	// Чтобы год начинался с 1-го месяца
#if TIME_INDEX>1
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

Time_t getAllSeconds(){
	u32 temp = 0;
	while(temp != seconds) temp = seconds;
	return temp;
}

u08 getMinutes(){
	return getMinutesFromSec(getAllSeconds());
}

u08 getHour(){
	u08 nowHour = getHourFromSec(getAllSeconds());
#ifdef TIME_INDEX
	nowHour += timeCorrectSummer;
    if(nowHour > 23) nowHour -= 24;
#endif
	return nowHour;
}

u16 getYear(){
	return getYearFromSec(getAllSeconds());
}

u16 getDayInYear() { // День в году
	return getDayInYearFromSec(getAllSeconds());
}

//LSB - day, MSB - mounth
u16 getDayAndMonth() {
	return getDayAndMonthFromDay(getDayInYear());
}

u08 getDaysInMonth(u08 month) {
	return daysInYear[month];
}

void setSeconds(u32 sec) {
	while(seconds != sec) seconds = sec;
}

Date_t getDateFromSeconds(Time_t sec){
	Date_t res;
	res.sec = sec%60;
	res.min = getMinutesFromSec(sec);
	res.hour = getHourFromSec(sec);
	u16 temp = getDayAndMonthFromDay(getDayInYearFromSec(sec));
	res.day = (u08)(temp & 0xFF);
	res.mon = (u08)(temp>>8);
	res.year = (sec/SECONDS_IN_YEAR) + 1970;
	return res;
}


void addOneSecondToDate(Date_t* date){
	if(date->sec < 59) date->sec++;
	else if(date->min < 59) date->min++;
	else if(date->hour < 23) date->hour++;
	else if(date->day < getDaysInMonth(date->mon)) date->day++;
	else if(date->mon < 12) date->mon++;
	else date->year++;
}

void addOneMinutesToDate(Date_t* date){
	if(date->min < 59) date->min++;
	else if(date->hour < 23) date->hour++;
	else if(date->day < getDaysInMonth(date->mon)) date->day++;
	else if(date->mon < 12) date->mon++;
	else date->year++;
}

void addOneHourToDate(Date_t* date){
	if(date->hour < 23) date->hour++;
	else if(date->day < getDaysInMonth(date->mon)) date->day++;
	else if(date->mon < 12) date->mon++;
	else date->year++;
}

void addOneDayToDate(Date_t* date){
	if(date->day < getDaysInMonth(date->mon)) date->day++;
	else if(date->mon < 12) date->mon++;
	else date->year++;
}

/*
 * return >0 if date1 > date2
 * return 0 if date = date2
 * return <0 if date1 < date2
 */
s08 compareDates(Date_t* date1, Date_t* date2){
	if(date1 == NULL || date2 == NULL) return 0;
	s16 different = date1->year - date2->year;
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
	if(strSize(date) < 17) { return;}
	strCopy(tempStr,date,2,0);
	u08 year = toIntDec(tempStr);
	if(year < 70) { // значит введена дата после 2000-го
		year += 30;
	}
	u08 dayOffset = (year+2)>>2; // Поправка в днях на высокосные годы
	strClear(tempStr);
	strCopy(tempStr,date,2,3);
	u08 mounth = toIntDec(tempStr);
	strClear(tempStr);
	strCopy(tempStr,date,2,6);
	u08 day = toIntDec(tempStr);
	strClear(tempStr);
	strCopy(tempStr,date,2,9);
	u08 hour = toIntDec(tempStr);
	strClear(tempStr);
	strCopy(tempStr,date,2,12);
	u08 minutes = toIntDec(tempStr);
	strClear(tempStr);
	strCopy(tempStr,date,2,15);
	u08 sec = toIntDec(tempStr);
	Time_t tempSeconds = year*SECONDS_IN_YEAR + (daysInYear[mounth-1] + day + dayOffset)*SECONDS_IN_DAY + hour*3600 + minutes*60 + sec;
	setSeconds(tempSeconds);
}
#endif

#ifdef __cplusplus
}
#endif
