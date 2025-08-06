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
    _In_ UINT uDelay,			�Ժ���ָ���¼�������
    _In_ UINT uResolution,		�Ժ���ָ����ʱ�ľ��ȣ���ֵԽС����ʱ���¼��ֱ���Խ�ߡ�ȱʡֵΪ1ms
    _In_ LPTIMECALLBACK fptc,	ָ��һ���ص�����
    _In_ DWORD_PTR dwUser,		����û��ṩ�Ļص�����
    _In_ UINT fuEvent			ָ����ʱ���¼����ͣ�	TIME_ONESHOT���ں�ֻ����һ���¼�
														TIME_PERIODIC�����Բ����¼�
    );
	


	typedef void (CALLBACK TIMECALLBACK)
	(UINT uTimerID,			��ʱ��ID
	UINT uMsg,				Ԥ��
	DWORD_PTR dwUser,		�ص�����ָ�����/�ṹ���ָ�룬�������ݲ������ص�������
	DWORD_PTR dw1,			Ԥ��
	DWORD_PTR dw2			Ԥ��
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


