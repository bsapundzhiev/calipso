#ifndef THREAD_H
#define THREAD_H

#ifdef TS_WIN32
#	ifdef TSM_EXPORTS
#	define TS_API __declspec(dllexport)
#	else
#	define TS_API __declspec(dllimport)
#	endif
#else
#	define TS_API
#endif



/* Define THREAD_T and MUTEX_T */
#ifdef TS_WIN32
# define THREAD_T DWORD
# define MUTEX_T CRITICAL_SECTION *
#elif defined(PTHREAD)
# define THREAD_T pthread_t
# define MUTEX_T pthread_mutex_t *
#endif

int service_count;

pthread_mutex_t ilock; //= PTHREAD_MUTEX_INITIALIZER;
#define UP(lock) pthread_mutex_lock(&lock)
#define DOWN(lock) pthread_mutex_unlock(&lock)

/*---MUTEX---*/
TS_API THREAD_T cpo_thread_id(void);
TS_API MUTEX_T cpo_mutex_alloc(void);
TS_API void cpo_mutex_free(MUTEX_T mutexp);
TS_API int cpo_mutex_lock(MUTEX_T mutexp);
TS_API int cpo_mutex_unlock(MUTEX_T mutexp);

int LaunchThread( void *(*func)(), void *parg);

//TODO: func

#endif /*!THREAD_H*/
