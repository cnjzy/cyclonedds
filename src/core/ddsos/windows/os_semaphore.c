#include "os_semaphore.h"

RET_CODE_T os_sem_init(os_sem_t* sem, UINT32 value)
{
	*sem = (os_sem_t)CreateSemaphore(0, value, 10, 0);
	
	return RET_NO_ERR;
}

RET_CODE_T os_sem_destroy(os_sem_t* sem)
{
	unsigned char bRet = CloseHandle(* sem);
	if ( (unsigned char)1 != bRet)
	{
		return RET_ERR;
	}
	return RET_NO_ERR;
}

RET_CODE_T os_sem_post(os_sem_t* sem)
{
	unsigned char bRet = ReleaseSemaphore((HANDLE)(*sem), 1, 0);
	if ((unsigned char)1 != bRet)
	{
		return RET_ERR;
	}
	return RET_NO_ERR;
}

RET_CODE_T os_sem_wait(os_sem_t* sem)
{
	DWORD ret = WaitForSingleObject((HANDLE)(*sem), INFINITE);
	if ((DWORD)0 != ret)
	{
		return RET_ERR;
	}
	return RET_NO_ERR;
}

