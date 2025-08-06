#ifndef _OS_MUTEX_H_
#define _OS_MUTEX_H_

#if defined(_WIN32)
#include <Windows.h>
typedef struct {
	SRWLOCK lock;
} mutex_t;

#elif defined(__linux__)
#include <pthread.h>

typedef struct {
	pthread_mutex_t mutex;
}mutex_t;
#else
typedef void mutex_t;
#endif

#if defined (__cplusplus)
extern "C" {
#endif

/**
	* ��ʼ����
	*/
void mutex_init(mutex_t *mutex);

/**
	* ������
	*/
void mutex_destroy(mutex_t *mutex);

/**
	* ����
	*/
void mutex_lock(mutex_t *mutex); 

	
/**
	* ����
	*/
void mutex_unlock(mutex_t *mutex);

#if defined (__cplusplus)
}
#endif
#endif /* _OS_MUTEX_H_ */
