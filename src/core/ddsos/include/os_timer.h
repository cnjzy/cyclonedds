#ifndef OS_TIMER_H
#define OS_TIMER_H

#if defined(__cplusplus)
extern "C" {
#endif


#include "base_types.h"
#include "os_types.h"

#if defined(_WIN32)
#include<Windows.h>
#pragma comment(lib,"Winmm.lib")
typedef UINT os_timer_ID;

#elif defined(__linux__)
#include <signal.h>
#include <time.h>
typedef timer_t os_timer_ID ;/*定时器的进程ID*/

#else
#include <os/pos/apex/apexLib.h>
#include <stdlib.h>
#include <stdio.h>
typedef PROCESS_ID_TYPE os_timer_ID;

#endif

typedef void* (*os_timer_route)(void* obj);

/*以毫秒作为间隔，运行函数,arg为句柄携带参数*/
signed int os_timer_init(os_timer_ID* timerID, unsigned int interval, os_timer_route timerhandle, void *arg);

void os_timer_kill(os_timer_ID timerID);

#if defined(__cplusplus)
}
#endif

#endif	/* OS_TIMER_H*/
