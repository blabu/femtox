# femtox
A simple operating system for microcontrollers.
This is a compilation with gcc.
I test it on microcontrollers AVR, MSP430 and STM32.
In TaskMngr.h, you can see the basic API for all modules in the system. Also you can configurate system there.
All platform specific file are concentrated in PlatformSpecific.c and PlatformSpecific.h.

<h1>Code example main.c </h1>

<code>
#include "TaskMng.h"<br>
<br>
void task(BaseSize_t arg_n, BaseParam_t arg_p) { <br>
    // Do some stuff<br>
    SetTimerTask(task,arg_n,arg_p,TICK_PER_SECOND);<br>
}<br>

int main(void) {<br>
  initFemtOS();<br>
  SetTask(task,0,0);<br>
  runFemtOS();<br>
  return 0;<br>
}<br>
</code>
