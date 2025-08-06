#include"os_semaphore.h"
#include <stdio.h>

static signed char name_index=0;
#define Max_Sem_Value 128

OS_API	RET_CODE_T os_sem_init(os_sem_t* sem, UINT32 value)
{

    RETURN_CODE_TYPE returnCode = 0;
    SEMAPHORE_NAME_TYPE sem_name ={0};
    sprintf(sem_name,"sem_name_%d",name_index++);
    CREATE_SEMAPHORE(sem_name, (SEMAPHORE_VALUE_TYPE)value, (SEMAPHORE_VALUE_TYPE)Max_Sem_Value, 0, sem, &returnCode);
    switch(returnCode)
    {
    case 0:{return 0;break;}
    default:{return -1;break;}
    }


}

OS_API	RET_CODE_T os_sem_destroy(os_sem_t* sem)
{
	return RET_NO_ERR;
}

OS_API	RET_CODE_T os_sem_post(os_sem_t* sem)
{
    RETURN_CODE_TYPE returnCode;
	SIGNAL_SEMAPHORE(* sem, &returnCode);


	 switch(returnCode)
	    {
	    case 0:{return 0;break;}
	    default:{return -1;break;}
	    }
	return RET_NO_ERR;
}

OS_API	RET_CODE_T os_sem_wait(os_sem_t* sem)
{
    RETURN_CODE_TYPE returnCode;

	WAIT_SEMAPHORE(*sem,-1,&returnCode);

	 switch(returnCode)
	    {
	    case 0:{return 0;break;}
	    default:{return -1;break;}
	    }
	return RET_NO_ERR;

}
