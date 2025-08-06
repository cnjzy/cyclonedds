#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "os_timer.h"
#include <mmiscapi2.h>

typedef struct {
	UINT uTimerID;
	os_timer_route func;
	void * arg;
} timerHandle;

timerHandle timer_handle;

void*  win_handle(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	timer_handle.uTimerID = uTimerID;
	timer_handle.func(timer_handle.arg);
	return 0;
}

signed int os_timer_init(os_timer_ID* timerID, unsigned int interval, os_timer_route timerhandle, void *arg)
{
	timer_handle.func = timerhandle;
	timer_handle.arg = arg;
/*
	timeSetEvent(
    _In_ UINT uDelay,			以毫秒指定事件的周期
    _In_ UINT uResolution,		以毫秒指定延时的精度，数值越小，定时器事件分辨率越高。缺省值为1ms
    _In_ LPTIMECALLBACK fptc,	指向一个回调函数
    _In_ DWORD_PTR dwUser,		存放用户提供的回调数据
    _In_ UINT fuEvent			指定定时器事件类型：	TIME_ONESHOT周期后只产生一次事件
														TIME_PERIODIC周期性产生事件
    );
	


	typedef void (CALLBACK TIMECALLBACK)
	(UINT uTimerID,			定时器ID
	UINT uMsg,				预留
	DWORD_PTR dwUser,		回调函数指向变量/结构体的指针，用来传递参数到回调函数内
	DWORD_PTR dw1,			预留
	DWORD_PTR dw2			预留
	);
	typedef TIMECALLBACK FAR *LPTIMECALLBACK;
	*/

	unsigned int ret = timeSetEvent(interval, 1, (LPTIMECALLBACK)win_handle,(DWORD_PTR)&timer_handle, TIME_PERIODIC);
	*timerID = ret;

	//if (0 != ret)
	//{
	//	printf("timer create error!\n");
	//	return ret;
	//}
	return (signed int)ret;
}

void os_timer_kill(os_timer_ID timerID)
{
	timeKillEvent(timerID);
}


