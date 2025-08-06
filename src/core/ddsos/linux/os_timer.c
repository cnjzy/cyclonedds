#include "os_timer.h"
#include <string.h>
#include <stdio.h>

signed int os_timer_init(timer_t* m_timer,unsigned int interval, os_timer_route timerhandle, void *arg)
{
	struct sigevent sigEvent; /*信号事件结构体*/
	struct itimerspec ts;
	
	
	memset(&sigEvent, 0, sizeof(sigEvent));

	sigEvent.sigev_notify = SIGEV_THREAD;
	sigEvent.sigev_value.sival_ptr = arg;
	sigEvent.sigev_notify_function = (void *)timerhandle;


	signed int ret =timer_create(CLOCK_REALTIME, &sigEvent, m_timer);
	

	if (0 != ret)
	{
		printf("timer create error!\n"); 
		return ret;
	}
	
	unsigned int interval_s = interval/1000;
	unsigned int interval_ms = interval%1000;

	
	ts.it_value.tv_sec = interval_s;
	ts.it_value.tv_nsec = interval_ms;
	ts.it_interval.tv_sec = interval_s;
	ts.it_interval.tv_nsec =interval_ms;
	
	ret = timer_settime(*m_timer, 0, &ts, NULL);

	if (0 != ret)
	{
		printf("timer set error!\n");
		return ret;
	}

	return ret;
}

void os_timer_kill(os_timer_ID timerID)
{
	int ret = timer_delete(timerID);
	if (0 != ret) 
	{
		printf("timer delete error!\n");
	}
}
