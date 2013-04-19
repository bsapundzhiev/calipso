/* HTTP chunked filter
 *
 * Copyright (C) 2012 Borislav Sapundzhiev
 *         
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 */
#include "cplib.h"
#include "calipso.h"
#include "dllist.h"
#include "chunks.h"

int http_chunked_filter_header(calipso_reply_t* reply) 
{
	return calipso_reply_set_header_value(reply, "Transfer-Encoding", "chunked");
}

int http_chunked_filter_body(calipso_reply_t* reply)
{
	char hex[12];
	int hex_len;
	dllist_t *l;
	struct chunk_ctx *cc;
	chunks_t *cf = reply->out_filter;
	
	struct mpool * new_pool = cpo_pool_create( CALIPSO_DEFAULT_POOLSIZE );
	chunks_t *chunked_filter = chunks_alloc(new_pool);

	if(!chunked_filter || !new_pool) {
		return ENOMEM;
	}

	FOREACH_CHUNK_CTX(l,cf,cc)
	{
		cc = l->data;
		hex_len = hex2ascii(cc->size, hex, 8);
		//printf("ChunkSize %s == %d\n", hex, cc->size);
		chunks_add_tail(chunked_filter, hex, hex_len);
		chunks_add_tail(chunked_filter, "\r\n", 2);
		chunks_add_tail(chunked_filter, cc->b, cc->size);
		chunks_add_tail(chunked_filter, "\r\n", 2);
		//chunks_remove_data(reply->out_filter, cc);
	}

	chunks_add_tail(chunked_filter, "0000\r\n\r\n", 8);

	chunks_destroy(reply->out_filter);
    cpo_pool_destroy(reply->pool);

	reply->pool = new_pool;
	reply->out_filter = chunked_filter;
	//chunks_dump(reply->out_filter);

	return http_chunked_filter_header(reply);
}

