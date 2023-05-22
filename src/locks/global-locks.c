#include <pthread.h>
#include "global-locks.h"

pthread_mutex_t gloc__mem_counter_mtx;

void global_locks_init(void)
{
	pthread_mutex_init(&gloc__mem_counter_mtx, NULL);
}