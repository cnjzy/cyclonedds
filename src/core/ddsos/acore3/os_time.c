#include"os_time.h"
#include <os/pos/apex/apexTime.h>

	OS_TIME os_getCurrentTime()
	{
	    RETURN_CODE_TYPE returnCode;
		SYSTEM_TIME_TYPE sys_time;
		GET_TIME(&sys_time,&returnCode);
		OS_TIME os_time = {0};

		os_time.os_millisecond = (sys_time/1000/1000)%1000;
		os_time.os_second = (sys_time/1000/1000/1000)%60;
		os_time.os_minute = (sys_time/1000/1000/1000/60)%60;
		os_time.os_hour = (sys_time/1000/1000/1000/60/60)%24;
		os_time.os_day = 0;
		os_time.os_month = 0;
		os_time.os_year = 0;

		return os_time;
	}

	unsigned long long os_getCurrentEpochTime(void)
{
	RETURN_CODE_TYPE returnCode;
		SYSTEM_TIME_TYPE sys_time;
		GET_TIME(&sys_time,&returnCode);

	return sys_time/1000/1000;
}