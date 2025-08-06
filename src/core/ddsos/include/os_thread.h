#ifndef OS_THREAD_H
#define OS_THREAD_H

#if defined(__cplusplus)
extern "C" {
#endif


#include "base_types.h"
#include "os_types.h"

#if defined(_WIN32)
#include <Windows.h>
typedef HANDLE os_thread_t;
#elif defined(__linux__)
#include <pthread.h>
typedef pthread_t os_thread_t;
#else
#include <os/pos/apex/apexLib.h>
#include <stdlib.h>
#include <stdio.h>
typedef PROCESS_ID_TYPE os_thread_t;

#endif


typedef enum OS_SCHED_CLASS {
	OS_SCHED_DEFAULT,
	OS_SCHED_REALTIME,
	OS_SCHED_TIMESHARE,
	OS_SCHED_MAX
} OS_SCHED_CLASS;

typedef struct os_thread_attr_s {
	OS_SCHED_CLASS sched_class;
	signed int priority;
	unsigned int stack_size;
} os_thread_attr_t;
void os_thread_attr_init(os_thread_attr_t *thread_attr);

/* 创建线程 */
typedef void* (*os_thread_route)(void* obj);
OS_API RET_CODE_T os_thread_create(os_thread_t *pthread, const signed char* name,
	const os_thread_attr_t *thread_attr, os_thread_route thread_route, void* arg);

/* 等待线程结束，并获取线程退出状态 */
OS_API RET_CODE_T os_thread_wait_exit(os_thread_t thread, void* thread_ret);

/* 获取当前线程编号 */
OS_API os_thread_t os_thread_self(void);
OS_API unsigned long os_thread_to_integer(os_thread_t thread);

#if defined(__cplusplus)
}
#endif

#endif	/* OS_THREAD_H*/
