/*	mpool.h memory pool management
 *
 *  Copyright (C) 2014 by Borislav Sapundzhiev
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 */
#ifndef __MMPOOL_H
#define __MMPOOL_H

typedef struct	mpool mpool_t;

mpool_t *mpool_create( size_t size );
void *mpool_alloc(mpool_t * pool, size_t size);
void mpool_free(mpool_t * pool, void *ptr);
void mpool_destroy(mpool_t *pool);
void mpool_dump(mpool_t *pool);
void mpool_get_stats(mpool_t *pool, int *size, int *free_size);

/* lib */
/*TODO:*/
//#define calipso_pool_t		mpool_t
#define cpo_pool_create 	mpool_create
#define cpo_pool_destroy 	mpool_destroy
#define cpo_pool_malloc 	mpool_alloc
#define cpo_pool_free 		mpool_free
#define cpo_pool_dump 		mpool_dump

char *cpo_pool_strdup(mpool_t * pool, const char *str);
char *cpo_pool_strndup(mpool_t *pool, const char *str, size_t len);
char *cpo_pool_strndup_upper(mpool_t *pool, const char *s, size_t len);
char *cpo_pool_strndup_lower(mpool_t *pool, const char *s, size_t len);
int cpo_pool_vasprintf(mpool_t *pool, char **buf, const char *format, va_list ap);
int cpo_pool_asprintf(mpool_t *pool, char *fmt, ...);

#endif