# femtox
A simple operating system for microcontrollers.
This is a compilation with gcc.
I test it on microcontrollers AVR, MSP430 and STM32.
In TaskMngr.h, you can see the basic API for all modules in the system. Also you can configurate system there.
All platform specific file are concentrated in PlatformSpecific.c and PlatformSpecific.h.

###Usage
<h1 align='center'>Code example main.c </h1>

```c
#include "TaskMng.h"

void task(BaseSize_t arg_n, BaseParam_t arg_p) {
    // Do some stuff
    SetTimerTask(task,arg_n,arg_p,TICK_PER_SECOND);
}

int main(void) {
  initFemtOS();
  SetTask(task,0,0);
  runFemtOS();
  return 0;
}
```