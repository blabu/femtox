# Femtox

**
Мини фреймворк написаннный на ANCII C для работы с микроконтроллерами, соответсвует стандарту С99
**

Возможности
=============
Фреймворк представляет собой прототип кооперативной операционной системы. В котором реализваны следующие возможности:
1. Очередь задач
2. Очередь таймеров
3. Подсистема сигналов и слотов
4. Подсистема управления колбек функциями
5. Система управления и выделения динамической памяти из заранее выделеной кучи
6. Структуры данных стек, очередь (FIFO, FILO) и кольцевой буфер, связанный список
7. Подсистема циклически выполняющихся задач

Введение
=============
Для использования фреймворка необходимо define-ми выбрать архитектру в файле platform.h, для которой будет осуществлятся компиляция или, если необходимой архитектуры найдено не было, реализовать платфомо-зависимые функции по примеру файла PlatformSpecific?????.h.
Поддерживаемые архитектуры:
1. AVR
2. MSP430
3. STM32
4. X86 (из под Linux) - может использоватся как мини фреймворк
5. X86 (из под Windows)  - может использоватся как мини фреймворк

Для тонкой настройки фреймворка под заданные нужды существуют файлы FemtoxConf????.h.
В файлах FemtoxTypes???.h - определены типы данных используемые в библиотеки.

Пример использования: минимальный рабочий код.
-------------

```c
#include "TaskMng.h"
#include "PlatformSpecific.h"

int main() {
initFemtOS();
runFemtOS();
return 0;
}
```
<code>initFemtOS()</code> - инициализирует все подсистемы настроенные в файле FemtoxConf????.h. 
<code>runFemtOS()</code> - запустит цикл обработки всех подсистем.
Указанный выше код не выполняет никаких полезных функций.
Расмотрим еще пример:

Пример использования: выполняем одну задачу.
-------------
Задачей является любая функция со следующей сигнатурой:
<code>void taskName(BaseSize_t arg_n, BaseParam_t arg_p);</code>

```c
#include "TaskMng.h"
#include "PlatformSpecific.h"

void someVeryImportantTask(BaseSize_t arg_n, BaseParam_t arg_p) {
	// Do many important things
	SetTimerTask(someVeryImportantTask,arg_n, arg_p,10);
}

int main() {
initFemtOS();
SetTask(someVeryImportantTask,arg_n, arg_p);
runFemtOS();
return 0;
}
```
В коде выше функция <code>void someVeryImportantTask(BaseSize_t arg_n, BaseParam_t arg_p)</code>
будет выполнена сразу после вызова <code>runFemtOS()</code>
По завершению выполнения этой функции будет запущен таймер для запуска ее же через 10 тиков операционной системы.
После чего, процесс повторится, и так до бесконечности.
Код выше можно упростить с приминением подсистема циклически выполняющихся задач:

```c
#include "TaskMng.h"
#include "PlatformSpecific.h"

void someVeryImportantTask() {
	// Do many important things
}

int main() {
initFemtOS();
SetCycleTask(10, someVeryImportantTask,TRUE);
runFemtOS();
return 0;
}
```
Аналогично предыдущему, функция <code>void someVeryImportantTask()</code> будет вызыватся каждые 10 тактов операционной системы.

Пример использования: подсистема колбек-ов.
-------------

Колбэк - это отложеный вызов функции по завершению какой-либо задачи.
На одну регистрацию колбека будет вызван один раз колбэк.
```c
#include "TaskMng.h"
#include "PlatformSpecific.h"

void someVeryImportantTask(BaseSize_t count, BaseParam_t argPtr) {
	switch(count) {
	case 0:
		// Do some prepare stuff
		count++;
		SetTimerTask(someVeryImportantTask,count,argPtr,20); // Back here later
		return;
	case 1:
		// Do some another stuff
		if(argPtr == NULL) count=10;
		else count = 20;
		SetTimerTask(someVeryImportantTask,count,argPtr,100); // Back here later
		return;
	case 10:
		// Do some stuff
		count = 2;
		SetTask(someVeryImportantTask,count,argPtr); 
	case 20:
		// Do some stuff
		count = 2;
		SetTask(someVeryImportantTask,count,argPtr); 
	case 2:
		// Finish some
		count = 0xFF;
		SetTask(someVeryImportantTask,count,argPtr); //Call task as quickly as you can
		return;
	default:
		execCallBack((void*)someVeryImportantTask); // Задача завершилась, сообщим об этом всем
		return;
	}
}

void waitSomeTask(BaseSize_t argN, BaseParam_t argPtr) {
	// Do some
}

int main() {
initFemtOS();
registerCallBack(waitSomeTask,0,NULL,someVeryImportantTask);
// По завершению этой задачи будет вызван waitSomeTask с указанными выше параметрами
SetTask(someVeryImportantTask,0,NULL);
// По заверению этой задачи ничего вызвано не будет так как новый колбэк не зарегистрирован
SetTimerTask(someVeryImportantTask,0,NULL,BIG_TIME_DELAY);
runFemtOS();
return 0;
}
```
Пример использования: подсистема сигналов.
-------------

Для реализации патерна издатель-подписчик хорошо подойдет подсистема сигналов

```c
#include "TaskMng.h"
#include "PlatformSpecific.h"
void* someVeryImportantSignal = (void*)10; // Здесь может быть что угодно (это лишь метка)
void someVeryImportantTask(BaseSize_t count, BaseParam_t argPtr) {
	switch(count) {
	case 0:
		// Do some prepare stuff
		count++;
		SetTimerTask(someVeryImportantTask,count,argPtr,20); // Back here later
		return;
	case 1:
		// Do some another stuff
		if(argPtr == NULL) count=10;
		else count = 20;
		SetTimerTask(someVeryImportantTask,count,argPtr,100); // Back here later
		return;
	case 10:
		// Do some stuff
		count = 2;
		SetTask(someVeryImportantTask,count,argPtr); 
	case 20:
		// Do some stuff
		count = 2;
		SetTask(someVeryImportantTask,count,argPtr); 
	case 2:
		// Finish some
		count = 0xFF;
		SetTask(someVeryImportantTask,count,argPtr); //Call task as quickly as you can
		return;
	default:
		emitSignal(someVeryImportantSignal, count, argPtr); // Генерируем сигнал с параметрами
		return;
	}
}

void waitSomeTask(BaseSize_t argN, BaseParam_t argPtr) {
	// Do some
}
#define BIG_TIME_DELAY 0xFFFF0
int main() {
initFemtOS();
connectTaskToSignal(waitSomeTask, someVeryImportantSignal);
 // По завершению задачи будет создан сигнал и вызвана функция waitSomeTask
SetTask(someVeryImportantTask,0,NULL);
// Через заданное время сново будет создан сигнал и вызвана функция waitSomeTask
SetTimerTask(someVeryImportantTask,0,NULL,BIG_TIME_DELAY);
runFemtOS();
return 0;
}
```
Лицензия
-------------
MIT License