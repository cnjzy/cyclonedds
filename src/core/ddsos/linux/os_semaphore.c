#include "os_semaphore.h"

RET_CODE_T os_sem_init(os_sem_t* sem, UINT32 value)
{
	/*RET_CODE_T enRet = RET_NO_ERR;*/
	if (NULL == sem)
	{
		return RET_INVALID_PARAM;
	}

	int iRet = sem_init(sem, 0, value);
	if (0 != iRet)
	{
		return RET_ERR;
	}
		
	return RET_NO_ERR;
}

RET_CODE_T os_sem_destroy(os_sem_t* sem)
{
	sem_destroy(sem);
	return RET_NO_ERR;
}

RET_CODE_T os_sem_post(os_sem_t* sem)
{
	int iRet = sem_post(sem);
	if (0 != iRet)
	{
		return RET_ERR;
	}
	return RET_NO_ERR;
}

RET_CODE_T os_sem_wait(os_sem_t* sem)
{
	int iRet = sem_wait(sem);
	if (0 != iRet)
	{
		return RET_ERR;
	}
	return RET_NO_ERR;
}

