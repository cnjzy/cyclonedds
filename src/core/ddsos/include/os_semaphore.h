#include "base_types.h"
#include "os_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#include <Windows.h>
	typedef HANDLE os_sem_t;

#elif defined(__linux__)
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
	typedef sem_t os_sem_t;

#else
#include <os/pos/apex/apexSemaphore.h>
	typedef SEMAPHORE_ID_TYPE os_sem_t;
#endif

OS_API	RET_CODE_T os_sem_init(os_sem_t* sem, UINT32 value);

OS_API	RET_CODE_T os_sem_destroy(os_sem_t* sem);

OS_API	RET_CODE_T os_sem_post(os_sem_t* sem);

OS_API	RET_CODE_T os_sem_wait(os_sem_t* sem);

#ifdef __cplusplus
}
#endif
