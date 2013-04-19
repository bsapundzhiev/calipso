/* mpool.c - memory pool
 *
 * Copyright (C) 2007 Borislav Sapundzhiev
 *         
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

//#include "xmalloc.h"
#ifndef xmalloc
#define xmalloc malloc
#endif
#ifndef __func__
#define __func__ __FUNCTION__ 
#endif 

extern int vasprintf();
extern int cpo_vslprintf();
#define DEFAULT_MPOOL_SIZE 32768 

struct mpool {
    struct mpool_blob *blob;
};

struct mpool_blob {
    size_t size;
    unsigned char *base; /* Base of allocated section */
    unsigned char *ptr; /* End of allocated section */
    struct mpool_blob *next; /* Next Pool */
};


static struct mpool_blob *new_mpool_blob(size_t size) {
    struct mpool_blob *blob = xmalloc(sizeof(struct mpool_blob));

    if (!size)
        size = DEFAULT_MPOOL_SIZE;

    blob->base = blob->ptr = xmalloc(size);
    blob->size = size;
    blob->next = NULL;

    return blob;
}

 /* Create a new pool */
 struct mpool *cpo_pool_create(size_t size) 
 {
     struct mpool *ret = xmalloc(sizeof(struct mpool));
 
     ret->blob = new_mpool_blob(size);
     
     return ret;
 }
 
 /* Free a pool */
void cpo_pool_destroy(struct mpool *pool) 
{
     struct mpool_blob *p, *p_next;
 
     if (!pool) return;
     if (!pool->blob) {
         printf("%s memory pool without a blob", __func__ );
         return;
    }
     
     p = pool->blob;
 
     while(p) {
         p_next = p->next;
		 //printf("pool_free %p\n", p->base);
         free(p->base);
         free(p);
         p = p_next;
     }
   
     free(pool);
}

#ifdef ROUNDUP
#undef ROUNDUP
#endif
 
/* round up to the next multiple of 16 bytes if necessary */
/* 0xFF...FFF0 = ~0 ^ 0xF */
#define ROUNDUP(num) (((num) + 15) & (~((unsigned long) 0x0) ^ 0xF))

/* Allocate from a pool */
void *cpo_pool_malloc(struct mpool *pool, size_t size) 
{
     void *ret = NULL;
     struct mpool_blob *p;
     size_t remain;
     
     if(!pool || !pool->blob) {
         printf("%s cpo_pool_malloc called without a valid pool", __func__);
     }
    if(!size) {
         /* This is legal under ANSI C, so we should allow it too */
         size = 1;
     }
 
     p = pool->blob;
 
     /* This is a bit tricky, not only do we have to make sure that the current
      * pool has enough room, we need to be sure that we haven't rounded p->ptr
      * outside of the current pool anyway */
     
     remain = p->size - ((char *)p->ptr - (char *)p->base);

     if (remain < size ||
         (char *) p->ptr > (p->size + (char *) p->base)) {
        /* Need a new pool */
         struct mpool_blob *new_pool;
         size_t new_pool_size = 2 * ((size > p->size) ? size : p->size);
         
         new_pool = new_mpool_blob(new_pool_size);
         new_pool->next = p;
         p = pool->blob = new_pool;
     }
 
     ret = p->ptr;
 
     /* make sure that the next thing we allocate is align on
        a ROUNDUP boundary */
     p->ptr = p->base + ROUNDUP(p->ptr - p->base + size);
 
     return ret;
}

void cpo_pool_free(struct mpool *pool, void * data)
{

}

//free block
void cpo_pool_free_block(struct mpool *pool, void * data)
{
	struct mpool_blob *p, *old;

    if (!pool)
        return;
    if (!pool->blob) {
        printf("%s memory pool without a blob", __func__ );
        return;
    }

    p = pool->blob;

    while (p) {
		if(p->base == data)
		{
			printf("free data %p\n", p->base);
			old = p;		
			free(old->base);
			if(p->next!=NULL)
				/*pool->blob*/ p = p->next;
			//else 
				/*pool->blob = p;*/
			free(old);
			pool->blob = p;
			break;
		}
        p = p->next;
    }
}

void cpo_pool_dump(struct mpool *pool)
{
	struct mpool_blob *p;
	p = pool->blob;
	printf("%s\n", __func__);
	while (p) {
		printf("sz %d, base %p, ptr %p\n", p->size, p->base, p->ptr);
		p= p->next;
    }
}

char *cpo_pool_strdup(struct mpool *pool, const char *str)
{
    char *ret;
    size_t len;

    if (!str)
        return NULL;

    len = strlen(str);

    ret = cpo_pool_malloc(pool, len + 1);

    strcpy(ret, str);
    return ret;
}

char *cpo_pool_strndup(struct mpool *pool, const char *str, size_t len)
{
    char *ret;
	
    if (!str)
        return NULL;
		
    ret = cpo_pool_malloc(pool, len + 1);
    strncpy(ret, str, len);
	ret[len] = '\0';
    return ret;
}

char *
cpo_pool_strndup_lower(struct mpool *pool, const char *s, size_t len)
{
    char *p;
    char *save;

    //p = xmalloc(len + 1);
    p = cpo_pool_malloc(pool, len + 1);

    for (save = p; len && (*p = tolower((int)*s)); ++s, ++p, --len)
        ;

    *p = '\0';
    p = save;

    return (p);
}

char *
cpo_pool_strndup_upper(struct mpool *pool, const char *s, size_t len)
{
    char *p;
    char *save;

    //p = xmalloc(len + 1);
    p = cpo_pool_malloc(pool, len + 1);

    for (save = p; len && (*p = toupper((int)*s)); ++s, ++p, --len)
        ;

    *p = '\0';
    p = save;

    return (p);
}

int
cpo_pool_vasprintf(struct mpool *pool, char **buf, const char *format, va_list ap)
{
    int ret, len;
	len = strlen(format) + 1024;

	*buf = cpo_pool_malloc(pool, len);
	//ret = vsnprintf(*buf, len, format, ap);
	ret = cpo_vslprintf(*buf, len, format, ap);
    if (ret < 0)
        printf("vasprintf() call failure in %s.", __func__);

    return (ret);
}

int
cpo_pool_asprintf(struct mpool *pool, char *fmt, ...)
{
	int ret =0;
	char *buf;
    va_list	ap;

    va_start(ap, fmt);
    ret = cpo_pool_vasprintf(pool, &buf, fmt, ap);
    va_end(ap);

	return ret;
}
