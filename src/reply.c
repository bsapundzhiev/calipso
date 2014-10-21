/* reply.c - http reply
 *
 * Copyright (C) 2007 Borislav Sapundzhiev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

#include <stdarg.h>
#include "calipso.h"
#include "dllist.h"
#include "chunks.h"
#ifdef USE_SSL
#include "cpo_io_ssl.h"
#endif

//#define DEBUG_HEADER
//#define DEBUG_HANDLER

#define MAX_HASH_REPLY_HEADER	32

calipso_reply_t *
calipso_reply_alloc(void)
{
    calipso_reply_t *reply = xmalloc(sizeof(calipso_reply_t));
    if (reply == NULL)
        return NULL;

    reply->pool = cpo_pool_create( CALIPSO_DEFAULT_POOLSIZE);

    return (reply);
}

int calipso_reply_init(calipso_reply_t *reply)
{
    reply->header = hash_table_create(MAX_HASH_REPLY_HEADER, NULL);

    reply->resource = calipso_resource_alloc();
    if (reply->resource == NULL)
        return CPO_ERR;

    reply->state = REPLY_STATE_NONE;
    reply->status = 0;
    reply->replybufsz = 0;
    reply->bytes_sent = 0;
    reply->bytes_to_send = 0;
    //reply->client = NULL;
    //reply->request = NULL;
    //reply->config = calipso_config_alloc();
    calipso_reply_set_handler(reply, NULL);
    /* init output filtes */
    reply->out_filter = chunks_alloc(reply->pool);
    reply->content_length = -1;

    return CPO_OK;
}

void calipso_reply_unalloc(calipso_reply_t *reply)
{
    if (reply->header) {
        hash_table_destroy(reply->header);
    }

    if (reply->resource) {
        calipso_resource_unalloc(reply->resource);
    }

    /* clear filter*/
    if (reply->out_filter) {
        //chunks_dump(reply->out_filter);
        chunks_destroy(reply->out_filter);
    }

    if (reply->pool) {
        //cpo_pool_dump(reply->pool);
        cpo_pool_destroy(reply->pool);
    }

    free(reply);
}

calipso_pool_t *
calipso_reply_get_pool(calipso_reply_t *reply)
{
    return (reply->pool);
}

size_t calipso_reply_get_replybuf_size(calipso_reply_t *reply)
{
    return (reply->replybufsz);
}

int calipso_reply_set_replybuf_size(calipso_reply_t *reply, size_t replybufsz)
{
    return ((reply->replybufsz = replybufsz));
}

int calipso_reply_set_status(calipso_reply_t *reply, int status)
{
    return ((reply->status = status) != 0);
}

int calipso_reply_get_status(calipso_reply_t *reply)
{
    return (reply->status);
}

int calipso_reply_set_client(calipso_reply_t *reply, calipso_client_t *client)
{
    return ((reply->client = client) != NULL);
}

calipso_client_t *
calipso_reply_get_client(calipso_reply_t *reply)
{
    return (reply->client);
}

int calipso_reply_set_request(calipso_reply_t *reply,
                              calipso_request_t *request)
{
    return ((reply->request = request) != NULL);
}

calipso_request_t *
calipso_reply_get_request(calipso_reply_t *reply)
{
    return (reply->request);
}

calipso_config_t *
calipso_reply_get_config(calipso_reply_t *reply)
{
    return (reply->config);
}

int calipso_reply_set_header_value(calipso_reply_t *p, char *header, char *fmt,
                                   ...)
{
    char *buf = NULL;
    va_list ap;

    va_start(ap, fmt);
    cpo_pool_vasprintf(p->request->pool/*p->pool*/, &buf, fmt, ap);
    va_end(ap);

    //TRACE("key: %s value: %s\n", header, buf);

    return (header && buf) ?
           (hash_table_insert(p->header, header, buf) == 0) : CPO_ERR;
}

int calipso_reply_unset_header_value(calipso_reply_t *p, const char *header)
{
    return (hash_table_remove(p->header, header) == 0);
}

int calipso_reply_set_state(calipso_reply_t *reply, int state)
{
    return ((reply->state = state));
}

int calipso_reply_get_state(calipso_reply_t *reply)
{
    return (reply->state);
}

int calipso_reply_set_handler(calipso_reply_t *reply,
                              int (*handler)(calipso_reply_t *))
{
    return ((reply->reply_hdl = handler) != NULL);
}

calipso_resource_t *
calipso_reply_get_resource(calipso_reply_t *reply)
{
    return (reply->resource);
}

int calipso_reply_set_config(calipso_reply_t *reply, calipso_config_t *config)
{
    return ((reply->config = config) != NULL);
}

char *
calipso_reply_get_header_value(calipso_reply_t *reply, char *header)
{
    char *key, *val = NULL;
    size_t keylen;
    calipso_pool_t *pool;

    pool = reply->request->pool; //calipso_reply_get_pool(reply);
    keylen = cpo_strlen(header);

    if (pool && keylen) {
        key = cpo_pool_strndup_lower(pool, header, keylen);
        val = hash_table_get_data(reply->header, key);
    }

    return val;
}

int calipso_reply_send_header(calipso_reply_t *reply)
{
    hash_size i;
    hash_node_t *node;
    hash_t * hash = reply->header;

    struct chunk_ctx *ctx_header = chunk_ctx_alloc(reply->pool);
    u_int16_t status = calipso_reply_get_status(reply);
    const char *message = calipso_http_status_get_message(status);

    /* First we will have to send the status code */
    chunk_ctx_printf(reply->pool, ctx_header, "HTTP/1.1 %d %s\r\n", status,
                     message);

    for (i = 0; i < hash_table_get_size(hash); i++) {

        node = hash->nodes[i];
        while (node) {
#ifdef DEBUG_HEADER
            TRACE("HEADER_LINE %s: %s\n", node->key, (char*)node->data);
#endif
	    chunk_ctx_printf(reply->pool, ctx_header, "%s: %s\r\n", node->key, (char*)node->data);
            node = node->next;
        }
    }

    chunk_ctx_append(reply->pool, ctx_header, "\r\n", 2);

    CNUNKS_ADD_HEAD_CTX(reply->out_filter, ctx_header);

    reply->bytes_to_send = reply->out_filter->total_bytes; //calipso_reply_get_replybuf_size(reply);

    calipso_reply_set_replybuf_size(reply, reply->out_filter->total_bytes);

    return (1);
}

int calipso_reply_print(calipso_reply_t *reply, char *data, size_t nbytes)
{

    chunks_add_tail(reply->out_filter, data, nbytes);

    reply->bytes_to_send = reply->out_filter->total_bytes;

    return (1);
}

int calipso_reply_printf(calipso_reply_t *reply, char *fmt, ...)
{
    int ret;

    char *buf;
    calipso_pool_t *pool;
    va_list ap;

    pool = calipso_reply_get_pool(reply);

    va_start(ap, fmt);
    ret = cpo_pool_vasprintf(pool, &buf, fmt, ap);
    va_end(ap);

    calipso_reply_print(reply, buf, ret);

    cpo_pool_free(pool, buf);

    return (ret);
}

int calipso_reply_send_writev(calipso_reply_t *reply)
{
    struct iovec *iovv, iov[IOV_MAX];
    cpo_array_t arr;

    dllist_t *l;
    struct chunk_ctx *cc;
    chunks_t *cf = reply->out_filter;
    calipso_client_t *client;
    int prev_send, sent = 0;
    int send = 0;

    //int complete = 0;
    int nw, block;

    arr.elem_size = sizeof(struct iovec);
    arr.max = IOV_MAX;
    arr.v = iov;

    client = calipso_reply_get_client(reply);

    for (;;) {
        arr.num = 0;
        iovv = NULL;
        prev_send = send;

        for (l = cf->list; l && (cc = l->data); l = l->next) {
            if (arr.num == IOV_MAX) {
                break;
            }

            if (cc->size) {
                send += cc->size;
                iovv = cpo_array_push(&arr);
                iovv->iov_base = cc->b;
                iovv->iov_len = cc->size;
            }
        }

#ifdef USE_SSL

        if (client->ssl)
            nw = SSL_writev(client->ssl, arr.v, arr.num);
        else

#endif
            nw = writev(client->csocket, arr.v, arr.num);

        if (nw < 0) {
            if (nw == -1 && errno == EWOULDBLOCK) {
                break;
            } else {
                reply->bytes_to_send = reply->bytes_sent;
                reply->out_filter->total_bytes = 0;
                TRACE("writev error %s\n", strerror(errno));
                cpo_log_error(calipso->log, "writev error %s\n",
                              strerror(errno));
                break;
            }
        }

        sent = nw > 0 ? nw : 0;

        if (send - prev_send == sent) {
            printf("Complete %d\n", send - prev_send);
            //complete =1;
        }

        //printf("nwritten %d bytes_sent %d\n", nw , reply->bytes_sent);
        reply->bytes_sent += sent;
        reply->out_filter->total_bytes -= sent;

        if (!reply->out_filter->total_bytes) {
            //printf("Break loop\n");
            break;
        }

        block = 0;

        FOREACH_CHUNK_CTX(l,cf,cc) {
            //dllist_t *ll = l;

            block += cc->size;
            if (block <= sent) {

                cc->b += cc->size;
                cc->size -= cc->size;

                //ll = dllist_find_release(ll, cc);
                //l = ll;
            }
            if (block > sent) {
                int last = block - sent;
                int len = cc->size - last;

                cc->b += len;
                cc->size -= len;
                reply->out_filter = cf;
                return 1;
            }
        }

    } //for;;

    calipso_reply_set_replybuf_size(reply, reply->out_filter->total_bytes);
    //assert(calipso_reply_get_replybuf_size(reply) >= 0);

    return 1;
}

int calipso_reply_send_write(calipso_reply_t *reply)
{
    int ret, read;
    size_t replybufsz;
    calipso_socket_t *listener;
    calipso_client_t *client;
    calipso_request_t *request;
    //ssize_t	(*w)(int, const void *, size_t);
    char buffer[OUTPUTBUFSZ];

    request = calipso_reply_get_request(reply);
    client = calipso_request_get_client(request);
    listener = calipso_client_get_listener(client);

    replybufsz = reply->out_filter->total_bytes;

    TRACE("replybufsz %ld\n", replybufsz);

    read = chunks_read_block(reply->out_filter, buffer, OUTPUTBUFSZ);

    /* get write handler */
    //w = listener->w;
    ret = listener->w(client, buffer, read);
    if (ret == -1) {
        TRACE(" ret = -1  NEEDS_FIX\n");
        return (0);
    }

    calipso_reply_set_replybuf_size(reply, reply->out_filter->total_bytes);
    TRACE("bytes_to_send %d bytes_sent %d read %d\n", reply->bytes_to_send,
          reply->bytes_sent, read);

    reply->bytes_sent += read;

    return (1);

}

/* TODO: complete */
int calipso_reply_send_file(calipso_reply_t * reply)
{
    ssize_t nr;
    calipso_request_t *request = calipso_reply_get_request(reply);
    calipso_client_t *client = calipso_request_get_client(request);
    //calipso_socket_t * listener = calipso_client_get_listener(client);
    int clientsock = calipso_client_get_socket(client);
    int rdesc = calipso_resource_get_file_descriptor(reply->resource);

    //set_tcp_nopush_option(client->csocket, 1);
    //set_tcp_nodelay_option(client->csocket, 1);
//for(;;)
    {

#ifdef USE_SSL
        if (client->ssl) {
            nr = SSL_send_file(client->ssl, rdesc);

        } else
#endif
        {
#ifdef _WIN32
            nr = calipso_sendfile(clientsock, rdesc , calipso_resource_get_size(reply->resource) );
#else
#ifdef __APPLE__
			off_t sent_bytes = calipso_resource_get_size(reply->resource);
			nr = sendfile(rdesc, clientsock, reply->resource->offset, &sent_bytes, NULL, 0);
#else
            nr = sendfile(clientsock, rdesc, &(reply->resource)->offset,
                          calipso_resource_get_size(reply->resource));
#endif
            //nr = splice_sendfile( rdesc, clientsock, &(reply->resource)->offset, calipso_resource_get_size(reply->resource) );

            //nr = calipso_sendfile( clientsock, rdesc,  calipso_resource_get_size(reply->resource) );
            //nr = calipso_aio_sendfile(clientsock, rdesc ,  calipso_resource_get_size(reply->resource)  );
#endif
            TRACE(" nr %lu ? %lu\n", nr,
                  calipso_resource_get_size(reply->resource));
        }
        if (nr <= 0) {
            if (nr == -1 && errno == EAGAIN)
                return 0;

            if (nr < 0)
                perror("Sendfile: ");
            close(rdesc);
            calipso_resource_set_file_descriptor(reply->resource, -1);
            //break;
        }
    }
    //set_tcp_nopush_option(client->csocket, 0);
    //set_tcp_nodelay_option(client->csocket, 0);
    return nr;
}

int calipso_reply_handler(calipso_reply_t * reply)
{

#ifdef  DEBUG_HANDLER
    TRACE("<<<REPLY_HANDLER>>> state: %d status: %d \n",
          calipso_reply_get_state(reply), calipso_reply_get_status(reply) );
    TRACE("bytes_to_send %d bytes_sent %d\n", reply->bytes_to_send, reply->bytes_sent );
#endif
    /* There are three possible states:
     * - REPLY_STATE_NONE
     *   which triggers the sending of reply headers
     *
     * - REPLY_STATE_TRANSFER
     *   which will either fill the replybuf by reading from the resource,
     *   send data from the replybuf, or call a module replybuf manager
     *   when data to be sent cannot be read from a file descriptor (this is
     *   the case for directory listing for example).
     *
     * - REPLY_STATE_DONE
     *   which is set after the data is fully transfered and causes the
     *   transfer state machine to stop calling the reply handler.
     */

    if (calipso_reply_get_state(reply) == REPLY_STATE_NONE) {
        calipso_reply_set_state(reply, REPLY_STATE_TRANSFER);
    }

    if (calipso_reply_get_state(reply) == REPLY_STATE_TRANSFER) {
        /* Is there data in the replybuf already ? */
        if (calipso_reply_get_replybuf_size(reply) != 0) {
            calipso_reply_send_writev(reply);
        } else {

            /* Are we reading data from a resource ? */
            if (calipso_resource_get_file_descriptor(reply->resource) != -1) {

                calipso_reply_send_file(reply);

            } else if (reply->reply_hdl != NULL) {
                if (reply->reply_hdl(reply) == 1)
                    reply->reply_hdl = NULL;
                calipso_reply_send_writev(reply);
            }
        }
    }

    if (calipso_reply_get_status(reply) /*== HTTP_OK*/&&
            reply->bytes_to_send == reply->bytes_sent &&
            calipso_resource_get_file_descriptor(reply->resource) == -1 &&
            reply->reply_hdl == NULL) {
        calipso_reply_set_state(reply, REPLY_STATE_DONE);
        //calipso_trigger_hook(HOOK_LOG, calipso_reply_get_request(reply));
        cpo_log_write_access_log(calipso->log, reply->request);
        return CPO_OK;
    }
    return (0);
}

