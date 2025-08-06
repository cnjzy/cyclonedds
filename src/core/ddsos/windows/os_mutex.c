#include <assert.h>
#include <stdlib.h>

#include "os_mutex.h"

void mutex_init(mutex_t *mutex)
{
  assert(mutex != NULL);
  InitializeSRWLock(&mutex->lock);
}

void mutex_destroy(mutex_t *mutex)
{
  assert(mutex != NULL);
}

void mutex_lock(mutex_t *mutex)
{
  assert(mutex != NULL);
  AcquireSRWLockExclusive(&mutex->lock);
}


void mutex_unlock(mutex_t *mutex)
{
  assert(mutex != NULL);
  ReleaseSRWLockExclusive(&mutex->lock);
}

