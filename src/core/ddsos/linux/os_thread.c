#include "os_thread.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>

void os_thread_attr_init(os_thread_attr_t *thread_attr)
{
	assert(thread_attr != NULL);
	thread_attr->sched_class = OS_SCHED_DEFAULT;
	thread_attr->priority = 0;
	thread_attr->stack_size = 1024 * 1024 * 2;	// 默认线程栈大小为 2MB
}

RET_CODE_T os_thread_create(os_thread_t *pthread, const signed char* name,
	const os_thread_attr_t *thread_attr, os_thread_route thread_route, void* arg)
{
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	if (thread_attr != NULL) {
		if (pthread_attr_setstacksize(&attr, thread_attr->stack_size) != 0) {
			perror("pthread_attr_setstacksize");
			return RET_ERR;
		}
	}

	int ret = pthread_create(pthread, &attr, thread_route, arg);
	if (ret) {
		return RET_ERR;
	}
	else {
		return RET_NO_ERR;
	}
}

RET_CODE_T os_thread_wait_exit(os_thread_t thread, void* thread_ret)
{
	assert(thread);
	int ret = pthread_join(thread, thread_ret);
	if (ret != 0) {
		return RET_ERR;
	}
	else {
		return RET_NO_ERR;
	}
}

os_thread_t os_thread_self(void)
{
	return pthread_self();
}

unsigned long os_thread_to_integer(os_thread_t thread)
{
	return thread;
}
