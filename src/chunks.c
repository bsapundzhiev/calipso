/* chunks.c - small data buffer abstraction layer
 *
 * Copyright (C) 2012 Borislav Sapundzhiev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include "dllist.h"
#include "chunks.h"
#include "calipso.h"

/*bucket ctx*/
struct chunk_ctx * chunk_ctx_alloc(struct mpool * pool) {
    struct chunk_ctx *ctx = cpo_pool_malloc(pool, sizeof(struct chunk_ctx));
    if (ctx == NULL)
        return NULL;
    ctx->b = NULL;
    ctx->size = 0;

    return ctx;
}

int chunk_ctx_append(struct mpool * pool, struct chunk_ctx *ctx,
                     const char *data, int size)
{
    /* just make room for data */
    char * newbuf = cpo_pool_malloc(pool, ctx->size + size + 1);
    if (newbuf) {
        memmove(newbuf, ctx->b, ctx->size);
        memmove(newbuf + ctx->size, data, size + 1);

        ctx->b = newbuf;
        ctx->size += size;
        return size;
    }
    return 0;
}

/* string operations */
int chunk_ctx_printf(struct mpool * pool, struct chunk_ctx *ctx, char *fmt, ...)
{
    char 	*buf;
	int		bytes;
  	va_list arg_ptr, apcopy;
  	va_start(arg_ptr, fmt);
  	va_copy(apcopy, arg_ptr);
	bytes = vsnprintf(NULL, 0, fmt, apcopy);
	va_end(apcopy);

    buf = malloc(bytes + 1);
    if(buf) {
    	/* cpo_vslprintf */
    	bytes = vsnprintf(buf, bytes + 1, fmt, arg_ptr);
    	if(bytes) {
        	chunk_ctx_append(pool, ctx, buf, bytes);
        }
    }
    
    va_end(arg_ptr);
    free(buf);
	
    return bytes;
}

int chunk_ctx_from_txt_file(struct mpool * pool, struct chunk_ctx *ctx,
                            const char *file)
{
    int read = 0;
    char text[CHUNKS_BUFFER_SIZE + 1];

    FILE *fp = fopen(file, "rb");
    if (fp) {
        while (!feof(fp)) {
            memset(text, 0, sizeof(text));
            read = fread(text, sizeof(char), CHUNKS_BUFFER_SIZE, fp);

            chunk_ctx_append(pool, ctx, text, read);
        }
        fclose(fp);
        return 1;
    }

    return 0;
}
/*!ctx*/

chunks_t *chunks_alloc(struct mpool * pool)
{
    chunks_t *c = cpo_pool_malloc(pool, sizeof(chunks_t));
    if (!c)
        return NULL;

    c->total_bytes = 0;
    c->list = NULL;
    c->pool = pool;
    return c;
}

int chunks_add_head(chunks_t *c, char *b, int sz)
{
    struct chunk_ctx *ctx = chunk_ctx_alloc(c->pool);
    if (ctx == NULL)
        return 0;

    chunk_ctx_append(c->pool, ctx, b, sz);

    c->list = dllist_insert_head(c->list, ctx);
    c->total_bytes = c->total_bytes + sz;

    return sz;
}

int chunks_add_tail(chunks_t *c, char *b, int sz)
{
    struct chunk_ctx *ctx = chunk_ctx_alloc(c->pool);
    if (ctx == NULL)
        return 0;

    chunk_ctx_append(c->pool, ctx, b, sz);

    c->list = dllist_insert_tail(c->list, ctx);
    c->total_bytes = c->total_bytes + sz;

    return sz;
}

void chunks_printf(chunks_t *c, char *fmt, ...)
{
    char *buf;
    va_list ap;

    va_start(ap, fmt);
    cpo_pool_vasprintf(c->pool, &buf, fmt, ap);
    va_end(ap);

    chunks_add_tail(c, buf, cpo_strlen(buf));
}

/* read block this will _consume_ the chunks
 * simple bucket elimination
 */
int chunks_read_block(chunks_t *c, char *b, int size)
{
    dllist_t *l;
    int block = 0;
    struct chunk_ctx *ctx;
    l = c->list;

    //*b = 0;
    memset(b, 0, size);
    while (l) {
        ctx = l->data;
        block += ctx->size;

        if (block <= size) {
            //printf("1read %d\n", ctx->size);
            memmove(b, ctx->b, ctx->size);
            b += ctx->size;
            chunks_remove_data(c, ctx);
            l = c->list;
            continue;
        } else if (block > size) {
            int last = block - size;
            int len = ctx->size - last;
            //printf("last %d block %d , size %d\n", last, block , size);
            //printf("2read %d %s\n", len, ctx->b);
            memmove(b, ctx->b, len);
            ctx->b += len;
            ctx->size -= len;
            c->total_bytes -= len;
            block = block - last;
            break;
        } else {
            assert(0);
        }

        l = l->next;
    }

    //printf("readed %d\n", block);
    return block;
}

/* get chunk  by size */
struct chunk_ctx * chunks_read_data(chunks_t *c, int size) {
    struct chunk_ctx *ctx = NULL;
    dllist_t *l;
    l = c->list;

    while (l) {
        ctx = l->data;
        if (ctx->size == size) {
            return ctx;
        }
        l = l->next;
    }
    return NULL;
}

void chunks_remove_data(chunks_t *c, struct chunk_ctx * ctx)
{
    dllist_t *l = dllist_find(c->list, ctx);

    if (l) {
        c->total_bytes -= ctx->size;
        c->list = dllist_find_release(l, ctx);
    }
}

/* debug */
void dump_buffer(void *buffer, int buffer_size)
{
    int i;
    for (i = 0; i < buffer_size; ++i)
        printf("0x%X", ((char *) buffer)[i]);
    printf("\n");
}

void chunks_dump(chunks_t *c, int hex)
{
    dllist_t *l;
    struct chunk_ctx *ctx;
    l = c->list;
    printf(" %s\n[", __FUNCTION__);
    while (l) {
        ctx = l->data;
        if (!hex) {
            printf("size: %d  b: %s\n", ctx->size, ctx->b);
        } else {
            dump_buffer(ctx->b, ctx->size);
        }
        l = l->next;
    }
    printf("] total %d \n", c->total_bytes);
}

void chunks_destroy(chunks_t *c)
{
    dllist_t *l;
    l = c->list;
    if (l) {
        dllist_relase_list(l);
    }
}
