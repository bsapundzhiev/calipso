#ifndef	_DEBUG_H_
#define	_DEBUG_H_

#if defined(DEBUG) || (_DEBUG)
#include <assert.h>

#define TRACE(x, ...)\
	printf("[info] %s:%d:%s() => " x, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__) 
#else
#define TRACE(x, ...)
#endif

#endif
 
