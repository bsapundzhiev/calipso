/*	mpool.c memory pool management
 *
 *  Copyright (C) 2014 by Borislav Sapundzhiev
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include "mpool.h"

#ifndef __func__
#define __func__ __FUNCTION__
#endif

#define ALIGN_STEP 16

#define ALIGN_SIZE(size)\
	(((size + ALIGN_STEP - 1) / ALIGN_STEP) * ALIGN_STEP)

typedef struct mblk mblk;

struct mblk {
    unsigned char *base;
    unsigned char *end;
    size_t size;
    struct mblk *next;
};

struct mpool {
    size_t 	size;
    size_t 	free_size;
    void	*free_blk;
    struct mblk  *blk;
};

static mblk * mblk_create( size_t size )
{
    mblk *p = malloc(size + sizeof(struct mblk));
    if (!p) return NULL;

    p->base = (unsigned char*)p + sizeof(mblk);
    p->end = p->base;
    p->size = size;
    p->next = NULL;
    return p;
}

static void mblk_destroy( mblk *p )
{
    free(p);
}

static size_t mblk_available( mblk *p )
{
    return p->size - ((char*)p->end - (char*)p->base);
}

static void * mblk_alloc( mblk *p, size_t size )
{
    void *mem = (void*)p->end;
    /*round next */
    p->end = p->base + ALIGN_SIZE(p->end - p->base + size);
    return mem;
}

static int get_ptr_size(void *ptr)
{
    int size = 0;
    char **q = (char**)&ptr;
    while ((int)((*q)[size]) != 0 ) {
        size++;
    }

    return size * sizeof(char);
}

mpool_t *mpool_create( size_t size )
{
    mpool_t * pool = (mpool_t *)malloc(sizeof(struct mpool));
    if (!pool)
        return NULL;

    pool->size = ALIGN_SIZE( size );
    pool->blk  = mblk_create( pool->size );
    pool->free_blk = NULL;
    pool->free_size = 0;
    return pool;
}

int mpool_is_valid (mpool_t * pool)
{
    if (!pool || !pool->blk) {
        printf("%s memory pool without a block", __func__ );
        return 0;
    } else
        return 1;
}

void *mpool_get_free_blk(mpool_t * pool, size_t size)
{
    void *mem = (void*)((char*)pool->free_blk - size);
    pool->free_size -= size;
    return mem;
}

void *mpool_alloc(mpool_t * pool, size_t size)
{
    struct mblk *p;

    /*Find first free block */
    if ( pool->free_size  >= size ) {
        return mpool_get_free_blk(pool, size );
    }

    p = pool->blk;
    /* resize */
    if ( mblk_available(p) <  size ||
            (char *) p->end > (p->size + (char *) p->base)) {
        /*Fine, need a new pool */
        struct mblk *new_blk;
        pool->size = 2 * ((size > p->size) ? size : p->size);
        new_blk = mblk_create( pool->size );
        new_blk->next = p;
        p = pool->blk = new_blk;
    }

    return  mblk_alloc(p,  size );
}
/*TODO: fix free */
static void mpool_free_size(mpool_t  *pool, void *ptr, size_t size)
{
    if (!mpool_is_valid(pool) && size <= 0) {
        printf("%s invalid params \n",__func__);
        return;
    }
    /*
    pool->free_blk = ptr;
    pool->free_blk = (void*)((char*)pool->free_blk + size);
    pool->free_size += size;*/
}

void mpool_free(mpool_t * pool, void *ptr)
{
    size_t size;
    char *p = ptr;
    if (!mpool_is_valid(pool))
        return;

    size = get_ptr_size(p)+1;
    mpool_free_size(pool, ptr, size);
}

void mpool_destroy(mpool_t *pool)
{
    struct mblk *p, *next;

    if (!mpool_is_valid(pool))
        return;

    p = pool->blk;

    while (p) {
        next = p->next;
        mblk_destroy(p);
        p = next;
    }

    free(pool);
}

void mpool_dump(mpool_t *pool)
{
    struct mblk *p;
    p = pool->blk;
    while (p) {
        printf("sz %ld, base %p, ptr %p\n", p->size, p->base, p->end);
        p= p->next;
    }
    printf("mpool size %ld free %ld\n",pool->size, pool->free_size );
}

void mpool_get_stats(mpool_t *pool, int *size, int *free_size)
{
    *size = pool->size;
    *free_size = pool->free_size;
}

/* pool lib */
char *cpo_pool_strdup(mpool_t * pool, const char *str)
{
    char *ret= NULL;
    size_t len;
    if (str) {
        len = strlen(str);
        ret = mpool_alloc(pool, len + 1);
        if (ret) {
            strcpy(ret, str);
        }
    }

    return ret;
}

char *cpo_pool_strndup(mpool_t *pool, const char *str, size_t len)
{
    char *ret = NULL;

    if (!str)
        return NULL;

    ret = (char*)cpo_pool_malloc(pool, len + 1);
    strncpy(ret, str, len);
    //*ret = '\0';
    ret[len] = '\0';
    return ret;
}

char *
cpo_pool_strndup_lower(mpool_t *pool, const char *s, size_t len)
{
    char *p, *save;
    p = cpo_pool_malloc(pool, len + 1);

    for (save = p; len && (*p = tolower((int)*s)); ++s, ++p, --len)
        ;

    *p = '\0';
    p = save;

    return (p);
}

char *
cpo_pool_strndup_upper(mpool_t *pool, const char *s, size_t len)
{
    char *p, *save;
    p = cpo_pool_malloc(pool, len + 1);

    for (save = p; len && (*p = toupper((int)*s)); ++s, ++p, --len)
        ;

    *p = '\0';
    p = save;

    return (p);
}

#ifndef _WIN32
#define _vscprintf(fmt,arg) vsnprintf(NULL, 0, fmt, arg)
#else
#define va_copy(dest, src) (dest = src)
#endif

int
cpo_pool_vasprintf(mpool_t *pool, char **buf, const char *format, va_list ap)
{
    int		bytes;
    va_list	apcopy;
    va_copy(apcopy, ap);
    bytes = _vscprintf(format, apcopy);
    va_end(apcopy);

    *buf = cpo_pool_malloc(pool, bytes + 1);
    if (*buf) {
        bytes = vsnprintf(*buf, bytes + 1, format, ap);
    }
    return (bytes);
}

int
cpo_pool_asprintf(mpool_t *pool, char *fmt, ...)
{
    int ret =0;
    char *buf;
    va_list	ap;

    va_start(ap, fmt);
    ret = cpo_pool_vasprintf(pool, &buf, fmt, ap);
    va_end(ap);

    return ret;
}
