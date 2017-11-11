/* chunks.h - small data buffer abstraction layer
 *
 * Copyright (C) 2012 Borislav Sapundzhiev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 */

#ifndef _CHUNKS_H
#define _CHUNKS_H

#include <ctype.h>
#include <stddef.h>

//typedef struct mpool mpool_t;

#define CHUNKS_BUFFER_SIZE 4096

#define FOREACH_CHUNK_CTX(l,c,ctx)\
for(l = c->list, ctx = l->data; l != NULL && (ctx = l->data); l= l->next)\

#define CNUNKS_ADD_HEAD_CTX(c, ctx)\
	c->list = dllist_insert_head(c->list, ctx);\
	c->total_bytes += ctx->size;\

#define CNUNKS_ADD_TAIL_CTX(c, ctx)\
	c->list = dllist_insert_tail(c->list, ctx);\
	c->total_bytes += ctx->size;\

typedef struct chunk_ctx {
    int size;
    char *b;
    int pos;
    int fd;
    int in_file;
    size_t file_pos;
} chunk_buf_t;

typedef struct chunks {
    int total_bytes;
    dllist_t *list;
    struct mpool *pool;
    //int max_size;
} chunks_t;

/*ctx*/
struct chunk_ctx * chunk_ctx_alloc(struct mpool * pool);
int chunk_ctx_append(struct mpool * pool, struct chunk_ctx *ctx, const char *data, int size);
int chunk_ctx_from_txt_file(struct mpool * pool, struct chunk_ctx *ctx, const char *file);
int chunk_ctx_printf(struct mpool * pool, struct chunk_ctx *ctx, char *fmt, ...);
/*!ctx*/
chunks_t *chunks_alloc(struct mpool * pool);
int chunks_add_head(chunks_t *c, char *b, int sz);
int chunks_add_tail(chunks_t *c, char *b, int sz);
/* rw data */
void chunks_printf(chunks_t *c, char *fmt, ...);
int chunks_read_block(chunks_t *c, char *b, int size );
struct chunk_ctx * chunks_read_data(chunks_t *c, int size );
void chunks_remove_data(chunks_t *c, struct chunk_ctx * ctx);
void chunks_destroy(chunks_t *c);
/* dbg */
void dump_buffer(void *buffer, int buffer_size);
void chunks_dump(chunks_t *c, int hex);

#endif //_CHUNKS_H

