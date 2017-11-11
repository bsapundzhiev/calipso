#ifndef INCLUDED_XMALLOC_H
#define INCLUDED_XMALLOC_H

#ifndef __GNUC__
#define __attribute__(foo)
#endif

extern void *xmalloc (unsigned size);
extern void *xzmalloc (unsigned size);
extern void *xrealloc (void *ptr, unsigned size);
extern char *xstrdup (const char *str);
extern char *xstrndup (const char *str, unsigned len);
//extern void *fs_get (unsigned size);
//extern void fs_give (void **ptr);



/* Functions using xmalloc.h must provide a function called fatal() conforming
   to the following: */
extern void fatal(const char *fatal_message, int fatal_code)
__attribute__ ((noreturn));

#endif /* INCLUDED_XMALLOC_H */
