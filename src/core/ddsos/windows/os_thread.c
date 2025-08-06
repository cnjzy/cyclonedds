#include "os_thread.h"
#include <assert.h>


void os_thread_attr_init(os_thread_attr_t *thread_attr)
{
	assert(thread_attr != NULL);
	thread_attr->sched_class = OS_SCHED_DEFAULT;
	thread_attr->priority = 0;
	thread_attr->stack_size = 1024 * 1024 * 2;	/*默认线程栈大小为 2MB*/
}

RET_CODE_T os_thread_create(os_thread_t *pthread, const char* name,
	const os_thread_attr_t *thread_attr, os_thread_route thread_route, void* arg)
{
	os_thread_attr_t attr;
	if (thread_attr == NULL) {
		os_thread_attr_init(&attr);
	}
	else {
		memcpy(&attr, thread_attr, sizeof(attr));
	}
	(*pthread) = CreateThread(NULL, thread_attr->stack_size, (LPTHREAD_START_ROUTINE)thread_route, arg, 0, NULL);
	if (*pthread) {
		return RET_NO_ERR;
	}
	else {
		return RET_ERR;
	}
}

RET_CODE_T os_thread_wait_exit(os_thread_t thread, void* thread_ret)
{
	WaitForSingleObject(thread, INFINITE);
	unsigned char ret = CloseHandle(thread);
	if (ret) {
		return RET_NO_ERR;
	}
	else {
		return RET_ERR;
	}
}

os_thread_t os_thread_self(void)
{
	return GetCurrentThread();
}

unsigned long os_thread_to_integer(os_thread_t thread)
{
	return GetThreadId(thread);
}

