#include "os_timer.h"
#include <os/pos/apex/apexTime.h>


static unsigned int index =1;

typedef struct{
	void *arg;
	os_timer_route func;
	unsigned int interval_ms;
}TimerAttirbute;

TimerAttirbute timer_attribute;

void timerFunc(void)
{
	RETURN_CODE_TYPE returnCode;
	//SYSTEM_TIME_TYPE sys_time_ns;
 	while(1)
	{
		//GET_TIME(&sys_time_ns,&returnCode);
 		//if(0ll != ( (sys_time_ns/1000000ll) % (signed long long)(timer_attribute.interval_ms)))
 		//{continue;}
		timer_attribute.func(timer_attribute.arg);

		SUSPEND_SELF( ((signed long long)timer_attribute.interval_ms * (signed long long)1000000 ),&returnCode);

	}

}


signed int os_timer_init(os_timer_ID* timerID, unsigned int interval, os_timer_route timerhandle, void *arg)
{
	RETURN_CODE_TYPE retCode = NO_ERROR;
	timer_attribute.interval_ms =interval;
	timer_attribute.func = timerhandle;
	timer_attribute.arg = arg;
	PROCESS_NAME_TYPE thread_name;
	sprintf(thread_name,"timer_%d",(unsigned int)index);

	PROCESS_ATTRIBUTE_TYPE attribute;
	memcpy(attribute.NAME,thread_name,MAX_NAME_LENGTH);
	attribute.ENTRY_POINT = timerFunc;
	attribute.STACK_SIZE=8192;
	attribute.BASE_PRIORITY = 2;
	attribute.PERIOD =0;
	attribute.TIME_CAPACITY =-1;
	attribute.DEADLINE =SOFT;


	CREATE_PROCESS(&attribute,timerID,&retCode);

	if(retCode != NO_ERROR)
	{
		printf("timerFuntion thread create  failure,retCode:%d\n",retCode);
	}

	START(*timerID,&retCode);

	return retCode;
}


void os_timer_kill(os_timer_ID timerID)
{
	RETURN_CODE_TYPE retCode = NO_ERROR;
	STOP (timerID,&retCode);
	if(retCode != NO_ERROR)
	{
		printf("timerFuntion thread kill  failure,retCode:%d\n",retCode);
	}
}
