/* thread.c
 *
 * Copyright (C) 2007 Borislav Sapundzhiev
 *         
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

#include "calipso.h"

#ifdef PTHREAD

int LaunchThread( void *(*func)(), void *parg)
{
#	ifdef WINDOWS
	LPDWORD tid;
	return CreateThread(NULL, 0L, (void *)func, parg, 0L, &tid);
#	else
	pthread_t pth;
	return pthread_create(&pth, NULL, (void *)func, parg);
#	endif

}

/**
 * new functions flows
 */

int
cpo_thread_create (pthread_t *thread_id,
		const pthread_attr_t *attributes,
		void *(*thread_function)(void *), void *arguments)
{
	int stat;

	stat = errno;

	return stat;
}

/*---MUTEXS---*/

TS_API THREAD_T cpo_thread_id(void)
{
#ifdef TS_WIN32
	return GetCurrentThreadId();
#elif defined(PTHREAD)
	return pthread_self();
#endif
}

/* Allocate a mutex */
TS_API MUTEX_T cpo_mutex_alloc(void)
{
	MUTEX_T mutexp;
#ifdef TS_WIN32
	mutexp = malloc(sizeof(CRITICAL_SECTION));
	InitializeCriticalSection(mutexp);

#elif defined(PTHREAD)
	mutexp = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutexp,NULL);
#endif
	return( mutexp );
}

/* Free a mutex */
TS_API void cpo_mutex_free(MUTEX_T mutexp)
{
	if (mutexp) {
#ifdef TS_WIN32
		DeleteCriticalSection(mutexp);
		free(mutexp);
#elif defined(PTHREAD)
		pthread_mutex_destroy(mutexp);
		free(mutexp);
	}
	//printf("Mutex freed thread: %d\n",mythreadid());
#endif
}

/* Lock a mutex */
TS_API int cpo_mutex_lock(MUTEX_T mutexp)
{
	//TSRM_ERROR((TSRM_ERROR_LEVEL_INFO, "Mutex locked thread: %ld", tsrm_thread_id()));
#ifdef TS_WIN32
	EnterCriticalSection(mutexp);
	return 1;
#elif defined(PTHREAD)
	return pthread_mutex_lock(mutexp);
#endif
}

/* Unlock a mutex */
TS_API int cpo_mutex_unlock(MUTEX_T mutexp)
{
	//TSRM_ERROR((TSRM_ERROR_LEVEL_INFO, "Mutex unlocked thread: %ld", tsrm_thread_id()));
#ifdef TS_WIN32
	LeaveCriticalSection(mutexp);
	return 1;
#elif defined(PTHREAD)
	return pthread_mutex_unlock(mutexp);
#endif
}

#endif /*!PTHREAD*/

