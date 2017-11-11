#ifndef	_DEBUG_H_
#define	_DEBUG_H_

#if defined(DEBUG) || (_DEBUG)
#	include <assert.h>
#ifndef TRACE
#define TRACE(x, ...)\
	printf("[info] %s:%d:%s() => " x, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#endif

#ifdef WP8
#undef TRACE
__inline void TRACE(const char *format, ...)
{
    char    buf[4096], *p = buf;
    va_list args;
    int     n;

    va_start(args, format);
    n = _vsnprintf(p, sizeof buf - 3, format, args); // buf-3 is room for CR/LF/NUL
    va_end(args);

    p += (n < 0) ? sizeof buf - 3 : n;

    while ( p > buf  &&  isspace(p[-1]) )
        *--p = '\0';

    *p++ = '\r';
    *p++ = '\n';
    *p   = '\0';

    OutputDebugString(buf);
}
#endif
#else
#	define TRACE(x, ...)
#endif

#endif

