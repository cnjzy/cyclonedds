#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#include "os_mutex.h"

void mutex_init (mutex_t *mutex)
{
  assert (mutex != NULL);
  pthread_mutex_init (&mutex->mutex, NULL);
}

void mutex_destroy (mutex_t *mutex)
{
  assert (mutex != NULL);

  if (pthread_mutex_destroy (&mutex->mutex) != 0)
    abort();
}

void mutex_lock (mutex_t *mutex)
{
  assert (mutex != NULL);

  if (pthread_mutex_lock (&mutex->mutex) != 0)
    abort();
}


void mutex_unlock (mutex_t *mutex)
{
  assert (mutex != NULL);

  if (pthread_mutex_unlock (&mutex->mutex) != 0)
    abort();
}

