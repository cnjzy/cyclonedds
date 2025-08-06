#include<os_thread.h>


void os_thread_attr_init(os_thread_attr_t *thread_attr)
{
	thread_attr->sched_class = OS_SCHED_DEFAULT;
	thread_attr->priority = 0;
	thread_attr->stack_size = 1024 * 1024 * 3;	/*默认线程栈大小为 1MB*/
}

RET_CODE_T os_thread_create(os_thread_t *pthread, const signed char* name,
	const os_thread_attr_t *thread_attr, os_thread_route thread_route, void* arg)
{
	PROCESS_ATTRIBUTE_TYPE attribute;
	memcpy(attribute.NAME,name,MAX_NAME_LENGTH);
	attribute.ENTRY_POINT = thread_route;
	attribute.STACK_SIZE=thread_attr->stack_size;
	attribute.BASE_PRIORITY = 2;
	attribute.PERIOD =0;
	attribute.TIME_CAPACITY =-1;
	attribute.DEADLINE =SOFT;

	RETURN_CODE_TYPE retCode;

	CREATE_PROCESS(&attribute,pthread,&retCode);

	if(retCode != NO_ERROR)
	{
		printf("timerFuntion thread create  failure,retCode:%d\n",retCode);
	}

	START(*pthread,&retCode);
	if (*pthread) {
		return RET_NO_ERR;
	}
	else {
		return RET_ERR;
	}
}

RET_CODE_T os_thread_wait_exit(os_thread_t thread, void* thread_ret)
{
	RETURN_CODE_TYPE retCode = NO_ERROR;
	STOP (thread,&retCode);
	if(retCode != NO_ERROR)
	{
		printf("timerFuntion thread kill  failure,retCode:%d\n",retCode);
	}
}


OS_API os_thread_t os_thread_self(void)
{
	RETURN_CODE_TYPE retCode = NO_ERROR;
	PROCESS_ID_TYPE process_id=0;
	GET_MY_ID(&process_id,&retCode);
	return process_id;
}

OS_API unsigned long os_thread_to_integer(os_thread_t thread)
{
	return thread;
}
