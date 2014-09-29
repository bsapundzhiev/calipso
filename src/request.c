/* request.c - http request
 *
 * Copyright (C) 2007 Borislav Sapundzhiev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

#include "cplib.h"
#include "calipso.h"
#include "chunks.h"
#include "timer.h"
#ifdef USE_SSL
#include "cpo_io_ssl.h"
#endif

#define REQUEST_BODY_SIZE_MAX	(2 * (1024 * 1024))

#define MAX_HASH_REQUEST_HEADER	32
/* parser */
#define EOL 	"\n"
#define EOLCR	"\r\n"

static int calipso_request_event_read_handler(calipso_client_t *client);
static int calipso_request_event_write_handler(calipso_client_t *client);
static int calipso_request_read_header(calipso_request_t *request);
static int calipso_request_read_body(calipso_request_t *request);

static int calipso_request_parse_header_buf(calipso_request_t *request);
static int request_parse_status_line(calipso_request_t *, char *);
static int request_parse_header(calipso_request_t *, char *);
static int request_persistent_handler(calipso_client_t *client);

calipso_request_t *
calipso_request_alloc(void)
{
    calipso_request_t *request = xmalloc(sizeof(calipso_request_t));

    if (request == NULL)
        return NULL;

    request->pool = cpo_pool_create( CALIPSO_DEFAULT_POOLSIZE * 2);

    return (request);
}

void calipso_request_unalloc(calipso_request_t *request)
{

    if (request->reply != NULL) {
        calipso_reply_unalloc(request->reply);
    }

    if (request->header != NULL) {
        hash_table_destroy(request->header);
    }

    if (request->in_filter) {
        chunks_destroy(request->in_filter);
    }

    if (request->pool != NULL) {
        cpo_pool_destroy(request->pool);
    }

    free(request);
    request = NULL;
}

int calipso_request_init_handler(calipso_client_t * client)
{
    cpo_event_t *event = client->event;

    calipso_request_t *request = calipso_request_alloc();

    if (request) {
        request->reply = calipso_reply_alloc();
        calipso_reply_init(request->reply);
    } else
        return CPO_ERR;

    if (request->reply)
        request->reply->request = request;
    else
        return CPO_ERR;

    event->handler_read = (void*)calipso_request_event_read_handler;
    event->handler_write = (void*)calipso_request_event_write_handler;

    request->header = hash_table_create(MAX_HASH_REQUEST_HEADER, NULL);

    calipso_request_set_method(request, NULL);
    calipso_request_set_uri(request, "/");
    calipso_request_set_version(request, "HTTP/1.1");

    calipso_request_set_querystring(request, NULL);
    calipso_request_set_handler(request, NULL);

    request->host = NULL;
    request->user = NULL;
    request->body_length = 0;
    request->request_time = (time_t) 0;

    client->client_persistent_hdl = request_persistent_handler;
    calipso_request_set_client(request, client);

    /* header head and body */
    request->header_buf = chunk_ctx_alloc(request->pool);
    request->in_filter = chunks_alloc(request->pool);

    if (request->header_buf) {
        request->header_buf->b = cpo_pool_malloc(request->pool, INPUTBUFSZ);
    }

    /*XXX: pipelining ? */
    //!calipso_client_pipeline_request(client, request);
    calipso_client_set_request(client, request);

    return CPO_OK;
}

int request_persistent_handler(calipso_client_t *client)
{
    //XXX: add cache

    if (client->request != NULL) {
        assert(client->request->reply->state == 2);
        calipso_request_unalloc(client->request);
        client->request = NULL;
    }

    return CPO_OK;
}

static int calipso_request_read_header(calipso_request_t *request)
{
    int n;
    char p;
    char *end;
    calipso_client_t *client = request->client;

    chunk_buf_t * c = request->header_buf;

    calipso_socket_t *listener = calipso_client_get_listener(client);

    for (;;) {

        if ((INPUTBUFSZ - 1) <= c->size) {
            *(c->b + c->size) = '\0';
            client->done = CPO_OK;
            return calipso_reply_set_status(request->reply,
                                            HTTP_REQUEST_URI_TOO_LARGE);
        }

        n = listener->r(client, &p, 1);

        if (n <= 0) {
            if (errno == EAGAIN) {
                TRACE("EAGAIN\n");
                break;
            } else {
                *(c->b + c->size) = '\0';
                cpo_log_error(calipso->log, "connection error %s",
                              strerror(errno));
                calipso_client_connection_error(client);
                return CPO_ERR;
            }
        } else {
            //printf("read_char 0x%X - %c\n", p, p);
            if (p < 0x9 || p > 0x7e) {
                *(c->b + c->size) = '\0';
                client->done = CPO_OK;
                cpo_log_error(calipso->log, "Non printable char -> 0x%X", p);
                return calipso_reply_set_status(request->reply,
                                                HTTP_BAD_REQUEST);
            }

            *(c->b + c->size) = p;
            c->size++;
            *(c->b + c->size) = '\0';
        }

        if (c->size > 4) {

            end = (c->b + (c->size - 4));

            if (!strcmp(end, EOLCR EOLCR)) {
                return CPO_OK;
            }

            end = (c->b + (c->size - 2));

            if (!strcmp(end, EOL EOL)) {
                return CPO_OK;
            }
        }

    }

    return NOK;
}
#if 0
static int calipso_request_read_body(calipso_request_t *request)
{
    int reqlen = 0;
    char buf[INPUTBUFSZ] = {'\0'};
    calipso_client_t *client = request->client;
    calipso_socket_t *listener = calipso_client_get_listener(client);

    int size = MIN(client->pending_bytes, INPUTBUFSZ);

    if (request->in_filter->total_bytes + size < REQUEST_BODY_SIZE_MAX) {

        reqlen = listener->r(client, buf, size);

        printf("body_buf[reqlen %d == %d]===> %s\n",reqlen, size, buf);
        if (reqlen > 0) {
            printf("reqlen %d strlen(buf) %d\n", reqlen , strlen(buf));
            assert(reqlen == strlen(buf));
            chunks_add_tail(request->in_filter, buf, reqlen);
        } else if (reqlen < 0) {

            calipso_client_connection_error(client);
            return CPO_ERR;
        }
    } else {
        return calipso_reply_set_status(request->reply, HTTP_REQUEST_ENTITY_TOO_LARGE);
    }

    return reqlen;
}
#endif

static int calipso_request_read_body(calipso_request_t *request)
{
    int reqlen = 0;

    calipso_client_t *client = request->client;
    calipso_socket_t *listener = client->listener;

    int max_size = request->in_filter->total_bytes + client->pending_bytes;

    if (max_size < REQUEST_BODY_SIZE_MAX) {

        char *buf = xmalloc(client->pending_bytes + 2);

        if (buf == NULL) {
            return calipso_reply_set_status(request->reply,
                                            HTTP_INTERNAL_SERVER_ERROR);
        }

        reqlen = listener->r(client, buf, client->pending_bytes);

        if (reqlen > 0) {

            assert(reqlen == (int)client->pending_bytes);

            chunks_add_tail(request->in_filter, buf, reqlen);
        } else if (reqlen < 0) {

            calipso_client_connection_error(client);
            reqlen = CPO_ERR;
        }

        free(buf);
    } else {
        return calipso_reply_set_status(request->reply,
                                        HTTP_REQUEST_ENTITY_TOO_LARGE);
    }

    return reqlen;
}

int calipso_request_event_read_handler(calipso_client_t *client)
{
    int ret = CPO_OK;
    calipso_request_t *request = client->request;

    if (request == NULL) {
        return calipso_request_init_handler(client);
    }

#ifdef USE_SSL
    /*check for SSL handshakeing*/
    if (!cpo_io_ssl_is_handshake_done(client))
        return 0;
#endif

    if (!client->done) {

        ret = calipso_request_read_header(request);

        if (CPO_OK == ret) {
			/*TODO: timeout*/
            tmr_alrm_reset(client, 300);
            client->done = calipso_request_parse_header_buf(request);
        }

    } else {

        if (request->body_length) {

            ret = calipso_request_read_body(request);

            if (request->in_filter->total_bytes == request->body_length) {

                calipso_request_parse_header_buf(request);
            }
        }
    }

    return ret;
}

int calipso_request_event_write_handler(calipso_client_t *client)
{

#ifdef USE_SSL

    /* SSL_read can cause write event during re-negotiation */
    if (client->ssl) {
        int pbytes = SSL_pending(client->ssl);

        if (pbytes) {
            client->pending_bytes = pbytes;
            calipso_request_event_read_handler(client);
            return CPO_ERR;
        }
    }

#endif

    return calipso_reply_handler(client->request->reply);
}

calipso_pool_t *
calipso_request_get_pool(calipso_request_t *request)
{
    return (request->pool);
}

int calipso_request_set_client(calipso_request_t *request,
                               calipso_client_t *client)
{
    calipso_reply_set_client(request->reply, client);

    return ((request->client = client) != NULL);
}

int calipso_request_set_method(calipso_request_t *request, char *method)
{
    return ((request->method = method) != 0);
}

int calipso_request_set_uri(calipso_request_t *request, char *uri)
{
    return ((request->uri = uri) != 0);
}

int calipso_request_set_version(calipso_request_t *request, char *version)
{
    return ((request->version = version) != 0);
}

int calipso_request_set_handler(calipso_request_t *request,
                                int (*handler)(calipso_request_t *))
{
    return ((request->request_hdl = handler) != NULL);
}

char *
calipso_request_get_method(calipso_request_t *request)
{
    return (request->method);
}

char *
calipso_request_get_uri(calipso_request_t *request)
{
    return (request->uri);
}

char *
calipso_request_get_version(calipso_request_t *request)
{
    return (request->version);
}

char *
calipso_request_get_querystring(calipso_request_t *request)
{
    return (request->querystring);
}

int calipso_request_set_querystring(calipso_request_t *request,
                                    char * querystring)
{
    return ((request->querystring = querystring) != NULL);
}

calipso_reply_t *
calipso_request_get_reply(calipso_request_t *request)
{
    return (request->reply);
}

calipso_client_t *
calipso_request_get_client(calipso_request_t *request)
{
    return (request->client);
}

int calipso_request_set_config(calipso_request_t *request,
                               calipso_config_t *config)
{
    return ((request->config = config) != NULL);
}

calipso_config_t *
calipso_request_get_config(calipso_request_t *request)
{
    return (request->config);
}

int calipso_request_set_time(calipso_request_t *request, time_t request_time)
{
    return ((request->request_time = request_time) != 0);
}

time_t calipso_request_get_time(calipso_request_t *request)
{
    return (request->request_time);
}

int calipso_request_handler(calipso_request_t *request)
{
    if (request->request_hdl)
        return (request->request_hdl(request));

    return CPO_OK;
}

char *
calipso_request_get_header_value(calipso_request_t *request, char *header)
{
    char *key;
    char *val = NULL;

    calipso_pool_t *pool = calipso_request_get_pool(request);
    int keylen = cpo_strlen(header);

    if (pool && keylen) {
        key = cpo_pool_strndup_lower(pool, header, keylen);
        val = hash_table_get_data(request->header, key);
        cpo_pool_free(pool, key);
    }

    return val;
}

static int request_set_header_vals(calipso_request_t *request)
{
    char *host = calipso_request_get_header_value(request, "host");
    char *clen = calipso_request_get_header_value(request, "content-length");

    if (host) {
        char * p = strrchr(host, ':');
        request->host =
            (p) ? cpo_pool_strndup(request->pool, host, p - host) : cpo_pool_strdup(
                request->pool, host);
    }

    if (clen) {
        request->body_length = cpo_atoi(clen);
    } /*XXX: transfer-coding */
    return CPO_OK;
}

int calipso_request_parse_header_buf(calipso_request_t *request)
{
    calipso_client_t *client = calipso_request_get_client(request);

    if (!client->done) {
        char *eol;
        char *requestbuf = request->header_buf->b;

        eol = strstr(requestbuf, EOLCR);
        if (eol == NULL)
            eol = strstr(requestbuf, EOL);

        if (eol == NULL)
            request_parse_status_line(request, requestbuf);
        else {
            if (*eol == '\r') {
                *eol = *(eol + 1) = 0;
                eol += 2;
            } else
                *eol++ = 0;

            request_parse_status_line(request, requestbuf);
            request_parse_header(request, eol);
            request_set_header_vals(request);
        }
    }

    if (request->body_length == request->in_filter->total_bytes) {
        /* Request hook call */
        calipso_trigger_hook(HOOK_REQUEST, request);
        calipso_trigger_hook(HOOK_TRANSLATE, request);
        calipso_trigger_hook(HOOK_RESOURCE, request);
        calipso_trigger_hook(HOOK_ACCESS_CHECK, request);
        calipso_trigger_hook(HOOK_MIME, request);
        calipso_trigger_hook(HOOK_REPLY, request);
    }

    return CPO_OK;
}

static int request_parse_status_line(calipso_request_t *request, char *line)
{
    char *method = NULL;
    char *uri = NULL;
    char *lastword = NULL;
    char *querystring = NULL;
/*TODO: fix me */
    /* Method */
    method = line;
    calipso_request_set_method(request, method);
    while (*line && !isspace((int )*line) && ++line)
        ;
    if (*line == 0)
        return (1);
    *line++ = 0;

    uri = line;
    if (*uri == 0)
        return (1);

    lastword = NULL;
    while (*line) {
        if (isspace((int)*line) && *(line + 1) && !isspace((int )*(line + 1)))
            lastword = line + 1;
        ++line;
    }

    if (lastword) {
        *(lastword - 1) = 0;
        calipso_request_set_version(request, lastword);

        /* remove tailing spaces */
        while (*lastword && ++lastword)
            ;
        --lastword;
        while (isspace((int)*lastword) && (*lastword-- = 0) == 0)
            ;
    }

    while (*uri && isspace((int )*uri) && ++uri)
        ;

    uri = cpo_strtok(uri, "?");

    querystring = cpo_strtok( NULL, "?");

    if (querystring) {
		char *new_querystring = cpo_pool_strdup(request->pool, querystring);
        calipso_request_set_querystring(request, new_querystring);
    }

    if (uri && *uri) {
    	char *new_uri = cpo_pool_strdup(request->pool, uri);
        calipso_request_set_uri(request, new_uri);
        /* remove tailing spaces */
        /*
         while (*uri && ++uri)
         ;
         --uri;
         while (isspace((int)*uri) && (*uri-- = 0) == 0)
         ;
         */
    } else {
    	calipso_reply_set_status(request->reply,
                                                HTTP_BAD_REQUEST);
    }

    return (1);
}

/* parser header lines */
static int request_parse_header(calipso_request_t *request, char *header)
{
    char *eol;
    char *hdr;
    char *val;

    while (*header) {
        hdr = eol = header;
        while (*eol
                && !((*eol == '\r' && *(eol + 1) == '\n' && *(eol + 2) != '\t')
                     || (*eol == '\n' && *(eol + 1) != '\t')) && ++eol)
            ;

        /* eol now points to end of current header */
        if (*eol == '\r')
            header = eol + 2;
        else if (*eol == '\n')
            header = eol + 1;
        else
            header = eol;
        *eol = 0;

        eol = strchr(hdr, ':');
        if (eol) {
            *eol = 0;
            val = eol + 1;
            if (isspace((int)*val) && ++val){}
            //    ;
            eol = hdr;
            while (*eol && (*eol = tolower((int) *eol)) && ++eol)
                ;
            if (hdr != NULL) {

                hash_table_insert(request->header, hdr, val);
            }
        }
    }

    time(&request->request_time);

    return (1);
}

