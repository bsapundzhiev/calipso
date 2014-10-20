/* HTTP Errors
 *
 * Copyright (C) 2012 Borislav Sapundzhiev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 */

#include "calipso.h"
#include "http_chunked.h"
#include "chunks.h"

static void
calipso_get_status_error(calipso_request_t *request, int http_status , char *buff, int len);

char *
calipso_http_get_server_address(calipso_reply_t* reply)
{
    static char buf[256];
    calipso_request_t *request = reply->request; //calipso_reply_get_request(reply);
    calipso_client_t *client = calipso_request_get_client(request);
    calipso_socket_t *listener = calipso_client_get_listener(client);

    int port  = calipso_socket_get_port(listener);
    const char * serverstring = calipso_get_server_string(NULL);
    const char *hostname = request->host; //calipso_server_get_hostname(server);

    snprintf(buf, sizeof(buf),
             "<address> %s(%s) Server at %s Port %d </address>",
             serverstring, OS, hostname, port);

    return buf;
}

int
calipso_http_print_error(calipso_reply_t *reply)
{
    int ret;
    u_int16_t status;

    char *message;
    char error_message[256];
    char * serveraddr;


    status     = calipso_reply_get_status(reply);
    message    = calipso_http_status_get_message(status);
    serveraddr = calipso_http_get_server_address(reply);

    calipso_get_status_error(reply->request, status , error_message, sizeof(error_message));

    /* XXX: error body may be present */
    if(reply->out_filter->total_bytes) {
        struct mpool * new_pool = cpo_pool_create( CALIPSO_DEFAULT_POOLSIZE );
        chunks_t *nf = chunks_alloc(new_pool);

        chunks_destroy(reply->out_filter);
        cpo_pool_destroy(reply->pool);

        reply->pool = new_pool;
        reply->out_filter = nf;
    }

    calipso_reply_set_header_value(reply, "Content-Type", "text/html");

    ret = calipso_reply_printf(reply,
                               "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n"
                               "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\">\n"
                               "  <head>\n"
                               "    <title>%d %s</title>\n"
                               "      <meta http-equiv=\"content-type\" content=\"text/html; charset=ISO-8859-1\" />\n"
                               "  </head>\n\n"
                               "  <body>\n"
                               "    <div class=\"error-header\">\n"
                               "      <h1>%s</h1>\n"
                               "%s\n"
                               "    </div>\n"
                               "    <div class=\"error\">\n"
                               "    </div>\n"
                               "    <div class=\"error-footer\">\n"
                               "      <hr />\n"
                               "%s\n"
                               "    </div>\n"
                               "  </body>\n"
                               "</html>\n", status, message, message,error_message,serveraddr);

    http_chunked_filter_body(reply);

    calipso_reply_send_header(reply);

    return (ret);
}

static void
calipso_get_status_error(calipso_request_t *request, int http_status , char *buff , int len)
{

    const char *uri 	   = calipso_request_get_uri(request);

    switch ( http_status  ) {
    /* 400 */
    case HTTP_FORBIDDEN:
        snprintf(buff, len,
                 "You don't have permission to access %s on this server.", uri);
        break;

    case HTTP_NOT_FOUND:
        snprintf(buff, len,
                 "The requested URL %s was not found on this server.", uri);
        break;

    case HTTP_BAD_REQUEST:
        snprintf(buff, len,
                 "The request had bad syntax or was inherently impossible to be satisfied.");
        break;

    case HTTP_UNAUTHORIZED:
        snprintf(buff, len,
                 "401 Unauthorized.");
        break;

    case HTTP_REQUEST_TIMEOUT:
        snprintf(buff, len,
                 "Request Time-out.");
        break;

    case HTTP_REQUEST_ENTITY_TOO_LARGE:
        snprintf(buff, len,
                 "Request Entity Too Large");
        break;

    case HTTP_REQUEST_URI_TOO_LARGE:
        snprintf(buff, len,
                 "Request-URI Too Large");
        break;
    /* 500 */
    case HTTP_INTERNAL_SERVER_ERROR:
        snprintf(buff, len,
                 "The server encountered an unexpected condition which prevented it from fulfilling the request.");
        break;

    case HTTP_NOT_IMPLEMENTED:
        snprintf(buff, len,
                 "The requested method %s was not implemented.", request->method);
        break;

    default:
        snprintf(buff, len,
                 "Server status %d : %s ", http_status, strerror(errno));
        break;
    }
}
