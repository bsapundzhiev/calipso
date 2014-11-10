/* mod_http.c HTTP
 *
 * Copyright (C) 2007 Borislav Sapundzhiev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

/* hook triger order
    => HOOK_REQUEST
    => HOOK_TRANSLATE
    => HOOK_RESOURCE
	=> HOOK_ACCESS_CHECK
    => HOOK_MIME
    => HOOK_REPLY
*/
#include "calipso.h"
#include "rfc2616.h"
#include "cplib.h"
#include "mod_http.h"
#include "http_mime.h"
#include "http_error.h"
#include "chunks.h"
#include "http_chunked.h"
#include "http_auth.h"
/* TODO: config */
#define SERVER_KEEPALIVES_MAX 			100
#define KEEPALIVE_TIMEOUT 			15
#define USE_HEADER_MODFIED_SINCE		1
#define USE_HEADER_HTTP_KEEPALIVE_TIMEMAX	0

static int mod_http_init(void);
static int mod_http_configure(void);
static int mod_http_chroot(void);
static int mod_http_privileges(void);
static int mod_http_request(calipso_request_t *);
static int mod_http_translate(calipso_request_t *);
static int mod_http_resource(calipso_request_t *);
static int mod_http_mime(calipso_request_t *);
static int mod_http_reply(calipso_request_t *);

/*TODO: rfc implementation*/
static int mod_http_set_keepalive(calipso_request_t *request);
static int mod_http_partial_content(calipso_request_t *request, int fd);
static int mod_http_set_if_modified_since(calipso_request_t *request, const char *http_date);
static void http_range_tokenize(const char *range , char **type , char **from , char **total);

struct http_conf_ctx {
    unsigned int portn;
    const char *documentroot;
    const char *hostname;
    const char *listen_naddr;
    unsigned char use_ssl;
};

static void config_port_handler(void *c,  void *val);
static void config_listen_handler(void *c,  void *val);
static void config_hostname_handler(void *c,  void *val);
static void config_documentroot_handler(void *c,  void *val);
static void config_location_handler(void *c,  void *val);
static void config_ssl_handler(void *c,  void *val);

#define CONF_BLOCK_CTX "server"

cfg_t conf_opt[] = {
    { "server_port", NULL, config_port_handler },
    { "listen", NULL, config_listen_handler },
    { "server_host", NULL, config_hostname_handler },
    { "server_docroot", NULL, config_documentroot_handler },
    { "location", NULL, config_location_handler },
    { "use_ssl", NULL, config_ssl_handler },
};

static void config_port_handler(void *c,  void *val)
{
    struct http_conf_ctx *ctx = c;
    ctx->portn = val ?  atoi((const char *)val) : 0;
}

static void config_listen_handler(void *c,  void *val)
{
    struct http_conf_ctx *ctx = c;
    ctx->listen_naddr = (const char *)val;
}

static void config_hostname_handler(void *c,  void *val)
{
    struct http_conf_ctx *ctx = c;
    ctx->hostname = (const char *)val;
}

static void config_documentroot_handler(void *c,  void *val)
{
    struct http_conf_ctx *ctx = c;
    ctx->documentroot = (const char *)val;
}

static void config_location_handler(void *c,  void *val)
{
    struct http_conf_ctx *ctx = c;
    const char *v = val;

    if(v) {
        TRACE("TODO: Location handler -> %s\n", v);
    }
}

static void config_ssl_handler(void *c,  void *val)
{
    struct http_conf_ctx *ctx = c;
    ctx->use_ssl = val ? OK : NOK;
}

static int config_parse_run(struct http_conf_ctx * ctx, const char *option, void *value)
{
    int i, size = ARRAYSZ(conf_opt);

    for(i=0; i < size; i++) {

        if(!conf_opt[i].p) {
            continue;
        }

        if(option == NULL) {

            conf_opt[i].p(ctx, NULL);
        } else {

            if(! strcmp(conf_opt[i].name, option) ) {
                conf_opt[i].p(ctx, value);
            }
        }
    }

    return CPO_OK;
}

static int mod_http_init_server_ctx(struct http_conf_ctx * ctx)
{
    calipso_server_t *server;
    calipso_socket_t *listener = NULL;
    /*dbg */
    printf("DBG:server %d %s %s ssl %d\n",
           ctx->portn,
           ctx->documentroot,
           ctx->hostname,
           ctx->use_ssl);

    if(ctx->listen_naddr)
        printf("listen_naddr %s\n", ctx->listen_naddr);

    server = calipso_server_alloc();
    if(server == NULL) {
        printf("cant alloc server!\n");
        exit(-1);
    }

    if(ctx->portn != 0) {
        printf("Init new listener...\n");
        listener = calipso_do_listen_sock(ctx->listen_naddr, ctx->portn);
    } else {
        hash_t *listeners_hash = calipso_get_listeners_hash();
        hash_node_t *n = hash_get_last_entry(listeners_hash);
        printf("Update existing listener...\n");
        if(n != NULL) {
            listener = (calipso_socket_t *)n->data;
        } else
            return CPO_ERR;
    }

    /*mark to set ssl*/
    if(ctx->use_ssl) {
#ifdef USE_SSL
        listener->state |= SOCKET_STATE_INIT_SSL;
#else
        printf("SSL is not enabled\n");
        exit(-1);
#endif
    }

    /* confgure server */
    server->keep_alive_max = SERVER_KEEPALIVES_MAX;
    calipso_server_set_hostname(server, (char*)ctx->hostname);
    calipso_server_set_documentroot(server, (char*)ctx->documentroot);

    /* connect request events */
    if(listener->accept_callback == NULL) {

        listener->accept_callback = (void*)calipso_request_init_handler;
    }

    calipso_socket_add_server(listener, server);

    listener->state |= SOCKET_STATE_ACTIVE;
    /*add/update listener hash;*/
    printf("register port: %d\n", listener->port);
    printf("register lsocket: %d\n", listener->lsocket);
    calipso_add_listener( listener );

    return CPO_OK;
}

static int mod_http_init_config(calipso_config_t * config)
{
    struct http_conf_ctx x, * ctx = NULL;

    for(config = list_get_first_entry( config );
            config != NULL;
            config = list_get_next_entry( config )) {

        conf_ctx_t *c = list_get_entry_value( config );

        if(c && c->block) {
            if(!strcasecmp(c->block, CONF_BLOCK_CTX)) {

                int state = config_get_state_ctx(c);
                /* lazzy loading */
                if(CTX_BLOCK_BEGIN == state) {
                    if(!ctx) {
                        config_parse_run(&x, NULL, NULL);
                        ctx = &x;
                    }
                }

                if(CTX_BLOCK_END == state)	{
                    mod_http_init_server_ctx(ctx);
                    if(ctx) {
                        ctx = NULL;
                    }
                }

                if(ctx) {
                    config_parse_run(ctx, c->option, c->value);
                }
            }
        }
    }

    return CPO_OK;
}

int pm_init()
{
    TRACE("register: %s\n",__FILE__);

    calipso_register_handler("*/*", mod_http_reply);

    calipso_register_hook(HOOK_INIT, (void *)mod_http_init);
    calipso_register_hook(HOOK_CONFIGURE, (void *)mod_http_configure);
    calipso_register_hook(HOOK_CHROOT, (void *)mod_http_chroot);
    calipso_register_hook(HOOK_PRIVILEGES, (void *)mod_http_privileges);
    calipso_register_hook(HOOK_REQUEST, (void *)mod_http_request);
    calipso_register_hook(HOOK_TRANSLATE, (void *)mod_http_translate);
    calipso_register_hook(HOOK_RESOURCE, (void *)mod_http_resource);
    calipso_register_hook(HOOK_MIME, (void *)mod_http_mime);
    /*TODO: http auth */
    http_auth_basic_init();
    return CPO_OK;
}

int pm_exit()
{
    TRACE("HOOK_EXIT\n");
    return 0;
}

static int mod_http_init(void)
{
    calipso_config_t *calipso_config = calipso_get_config();
    /* new config init */
    return mod_http_init_config(calipso_config);
}

static int mod_http_configure(void)
{
    /*TODO: fix */
    mime_type = hash_table_create(64, NULL);
#ifdef _WIN32
#ifdef WP8
    mime_load_file(mime_type, "mime.types");
#else
    mime_load_file(mime_type, "../../doc/mime.types");
#endif
#else
#if defined(ANDROID)
    mime_load_file(mime_type, "/storage/emulated/0/Android/data/com.bsapundzhiev.calipso/files/mime.types");
#else
    mime_load_file(mime_type, "../doc/mime.types");
#endif

#endif
    return 1;
}

static int mod_http_chroot(void)
{
    calipso_config_t *config = calipso_get_config();

    /* Check wether chroot()-ing is required, chroot is default behaviour */
    const char *chrootenable = config_get_option(config, "chroot", NULL);
    const char *chrootpath = config_get_option(config, "chrootpath", NULL);

    if (chrootenable == NULL) {
        if ( calipso_get_uid() )
            printf("Chroot() must be disabled as non privileged user.");
        if (chrootpath == NULL)
            printf("Chroot() directive enabled, but no ChrootPath() set.");
        if (chroot(chrootpath) < 0)
            printf("chroot() call failure in %s.", __func__);
        if (chdir("/") < 0)
            printf("chdir() call failure in %s.", __func__);
    }

    return CPO_OK;
}

static int mod_http_privileges(void)
{
    return CPO_OK;
}

static int mod_http_request(calipso_request_t *request)
{
    char date[64];
    time_t tm;
    //calipso_pool_t *pool = calipso_request_get_pool(request);
    calipso_config_t * config = calipso_request_get_config(request);
    calipso_reply_t * reply = calipso_request_get_reply(request);
    int http_status  = calipso_reply_get_status(reply);

    tm = calipso_request_get_time(request);

    cpo_http_time(date, &tm);

    calipso_reply_set_header_value(reply, "Date", "%s", date);
    calipso_reply_set_header_value(reply, "Server", "%s", calipso_get_server_string(config));
    /* connection ? */
    mod_http_set_keepalive(request);

    /* check pre errors from request/reply */
    if ( calipso_http_status_is_error(http_status)) {
        TRACE("Pre http error %d\n", http_status);
        return (0);
    }

    /*XXX: HTTP 1.1 require host */
    if (request->host == NULL) {
        calipso_reply_set_status(reply, HTTP_BAD_REQUEST);
        return (0);
    }

    calipso_reply_set_status(reply, HTTP_OK);

    return 1;
}


static int mod_http_translate(calipso_request_t *request)
{
    char *pathname = NULL;
    char *uri = NULL;
    char *documentroot = NULL;

    calipso_client_t *client = calipso_request_get_client(request);
    calipso_server_t *server = calipso_client_get_server(client);
    calipso_reply_t *reply = calipso_request_get_reply(request);
    calipso_resource_t *resource = calipso_reply_get_resource(reply);
    calipso_pool_t *pool = calipso_request_get_pool(request);

    int http_status  = calipso_reply_get_status(reply);

    if ( calipso_http_status_is_error(http_status)) {
        TRACE(" http error %d\n", http_status);
        return (0);
    }

    if( remove_dots_from_uri_path(request->uri) == -1 ) {
        calipso_reply_set_status(reply, HTTP_BAD_REQUEST);
        return 0;
    }

    if (!cpo_uri_sanity_check( request->uri )) {
        calipso_reply_set_status(reply, HTTP_BAD_REQUEST);
        return 0;
    }

    uri = calipso_request_get_uri(request);

    //if (*uri != '/') {
    //    calipso_reply_set_status(reply, HTTP_BAD_REQUEST);
    //    return (0);
    //}

    documentroot = calipso_server_get_documentroot(server);

    if (!documentroot) {
        calipso_reply_set_status(reply, HTTP_BAD_REQUEST);
        return (0);
    }

    pathname = cpo_pool_malloc(pool, MAXPATHLEN);
    cpo_strlcpy(pathname, documentroot, MAXPATHLEN);
    cpo_uri_strdecode(uri, uri);
    if (cpo_strlcat(pathname, uri, MAXPATHLEN) >= MAXPATHLEN) {
        calipso_reply_set_status(reply, HTTP_REQUEST_URI_TOO_LARGE);
        return (0);
    }

    /* security chek */
    //if (!cpo_uri_sanity_check( pathname )) {
    //   	calipso_reply_set_status(reply, HTTP_BAD_REQUEST);
    //    return 0;
    //}

    /* decode string */
    //cpo_uri_strdecode(pathname, pathname);
    calipso_resource_set_path(resource, pathname);

    return 1;
}

static int mod_http_resource(calipso_request_t *request)
{
    int fd;
    struct stat sb;
    char date[32];

    calipso_reply_t *reply = calipso_request_get_reply(request);
    calipso_resource_t *resource = calipso_reply_get_resource(reply);
    const char *filename = calipso_resource_get_path(resource);
    int http_status  = calipso_reply_get_status(reply);

    /* Set the default handler */
    calipso_request_set_handler(request, mod_http_reply);

    if (calipso_http_status_is_error(http_status)) {
        TRACE(" http error %d\n", http_status);
        return (0);
    }
    //XXX: IsHandler AddHandelr ...
    if ( is_file_of(filename, ".php") ) {
        return (0);
    }

    /* check method */
    if (strcasecmp(request->method, "GET")  != 0 &&
            strcasecmp(request->method, "HEAD") != 0) {
        calipso_reply_set_status(reply, HTTP_NOT_IMPLEMENTED);
        return (NOK);
    }
    /* stat the resource for other modules to know what type it is */
#ifdef _WIN32
    remove_trailing_slash(filename);
#endif
    if (stat(filename, &sb) < 0) {

        //TRACE("ERROR: %s %s\n", strerror(errno), filename);

        switch (errno) {
        case ENOTDIR:
        case ENOENT:
            calipso_reply_set_status(reply, HTTP_NOT_FOUND);
            break;
        case EACCES:
            calipso_reply_set_status(reply, HTTP_FORBIDDEN);
            break;
        case ENAMETOOLONG:
            calipso_reply_set_status(reply, HTTP_REQUEST_URI_TOO_LARGE);
            break;
        default:
            calipso_reply_set_status(reply, HTTP_INTERNAL_SERVER_ERROR);
        }

        return CPO_OK;
    }

    calipso_resource_set_stat(resource, &sb);

    cpo_http_time(date, &sb.st_mtime);

    /* modified_since */
    if(USE_HEADER_MODFIED_SINCE &&
            CPO_OK == mod_http_set_if_modified_since(request, date))
        return OK;

    if (! calipso_resource_is_file(resource)) {

        /* default state mod_directory override */
        if(!calipso_http_status_is_error(http_status)) {
            calipso_reply_set_status(reply, HTTP_FORBIDDEN);
        }
        return CPO_OK;
    }

    /*XXX: fcache needed */
    fd = cpo_file_open(filename, 0);

    if(fd < 0 ) {
        calipso_reply_set_status(reply, HTTP_INTERNAL_SERVER_ERROR);
        return CPO_ERR;
    }

    calipso_reply_set_header_value(reply, "Accept-Ranges", "bytes");

    if (!calipso_reply_get_header_value(reply, "content-range"))
        reply->content_length = calipso_resource_get_size(resource);
    //calipso_reply_set_header_value(reply, "Content-Length", "%llu",
    //                               calipso_resource_get_size(resource));

    calipso_reply_set_header_value(reply, "Last-Modified", date);

    calipso_resource_set_file_descriptor(resource, fd);

    mod_http_partial_content(request, fd);

    return CPO_OK;
}

static int mod_http_mime(calipso_request_t *request)
{
    const char *mime = NULL;
    int http_status;
    calipso_resource_t *resource;

    calipso_reply_t *reply = calipso_request_get_reply(request);
    resource = calipso_reply_get_resource(reply);
    http_status = calipso_reply_get_status(reply);

    /* mime already set */
    if (calipso_http_status_is_error(http_status)) {
        return 0;
    }

    if(!calipso_resource_is_directory(resource)) {
        char *file = calipso_resource_get_path(resource);
        //XXX: IsHandler AddHandelr ...
        if ( is_file_of(file, ".php") ) {
            return (0);
        }
        mime = mime_get_type_value(mime_type, file);

        if(reply) {
            calipso_reply_set_header_value(reply, "Content-Type", (char*)mime);
        }
    }

    return 1;
}

static int mod_http_reply(calipso_request_t *request)
{

    int http_status;
    calipso_reply_t *reply;

    reply = calipso_request_get_reply(request);
    http_status = calipso_reply_get_status(reply);
    TRACE("STATUS: %d\n", http_status);
    if(calipso_http_status_is_error(http_status)) {
        calipso_http_print_error(reply);
        return NOK;
    }

    if( calipso_http_status_is_redirection(http_status)) {
        /* http info or redirection */
        reply->content_length = 0;
        calipso_reply_set_header_value(reply,
                                       "Content-Length", "%d", reply->content_length);
        calipso_reply_send_header(reply);
        return (NOK);
    }

    /* XXX: chunked filter */
    if(reply->content_length == -1
            && reply->out_filter->total_bytes > 0 /*OUTPUTBUFSZ*/) {
        http_chunked_filter_body(reply);
    } else if(reply->content_length == -1
              && reply->out_filter->total_bytes == 0) {
        /* content lenght and buffer not present */
        reply->content_length =0;
        calipso_reply_set_header_value(reply,
                                       "Content-Length", "%d", reply->content_length);
    } else {
        /* content lenght is present */
        calipso_reply_set_header_value(reply,
                                       "Content-Length", "%lu", reply->content_length);
    }


    calipso_reply_send_header(reply);

    return OK;
}

/* http protocol */
static int mod_http_set_keepalive(calipso_request_t *request)
{
    const char * conn;
    calipso_reply_t *reply;
    calipso_client_t *client;
    client = calipso_request_get_client(request);
    reply = calipso_request_get_reply(request);

    client->keepalive = NOK;
    conn = calipso_request_get_header_value(request, "connection");

    if(conn != NULL && !strcasecmp(conn,"keep-alive")) {
        /*limit keep alives*/
        int max;
        client->keepalives++;
        max = client->server->keep_alive_max - client->keepalives;

        if(max > 0) {
            client->keepalive = OK;

            if(USE_HEADER_HTTP_KEEPALIVE_TIMEMAX) {
                calipso_reply_set_header_value(reply, "Keep-Alive",
                                               "timeout=%d, max=%d", KEEPALIVE_TIMEOUT, max);
            }
        }
    }

    calipso_reply_set_header_value(reply, "Connection",
                                   client->keepalive ? "Keep-Alive" : "Close");

    return OK;
}

static int mod_http_set_if_modified_since(calipso_request_t *request, const char *http_date)
{
    const char *modified_since = calipso_request_get_header_value(request, "if-modified-since");

    if(modified_since && !strcmp(http_date , modified_since)) {

        calipso_reply_set_status(request->reply, HTTP_NOT_MODIFIED);
        return OK;
    }
    return NOK;
}

static int mod_http_partial_content(calipso_request_t *request, int fd)
{
    /* HTTP_PARTIAL_CONTENT */

    char *range;
    char *type;
    char *offset1;
    char *offset2;

    uintmax_t total_size;
    uintmax_t partial_size;

    range = calipso_request_get_header_value(request, "range");

    if (range) {
        calipso_reply_set_status(request->reply, HTTP_PARTIAL_CONTENT);
        type = offset1 = offset2 = NULL;
        http_range_tokenize(range, &type, &offset1 , &offset2);
        TRACE("RANGE: %s\n", range);
        TRACE("type: %s offset1: %s offset2: %s \n", type, offset1, offset2);

        total_size = calipso_resource_get_size(request->reply->resource);
        partial_size = total_size - atol(offset1);
        TRACE("partial size: %llu ? total size %llu\n", partial_size, total_size);

        calipso_reply_set_header_value(request->reply,
                                       "content-range", "bytes=%s-%llu/%llu", offset1, total_size - 1, total_size);
        calipso_reply_set_header_value(request->reply,
                                       "Content-Length", "%llu", partial_size);

        /*set fd position */
        (off_t)lseek(fd, (off_t)atoi(offset1), SEEK_SET);

    }
    /* !HTTP_PARTIAL_CONTENT*/
    return OK;
}

/* !protocol */
/* helper */
void http_range_tokenize(const char *range , char **type , char **from , char **total)
{
    char *copy = (char *)malloc(strlen(range) + 1);

    if (copy != NULL) {
        strcpy(copy, range);
        *type = strtok(copy, "=");
        *from = strtok(NULL, "-");
        *total = strtok(NULL, "-");
        free(copy);
    }
}

