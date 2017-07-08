/* mod_dav.c - TODO webdav
 *
 * Copyright (C) 2007 Borislav Sapundzhiev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

#include "calipso.h"
#include "rfc2616.h"
#include "cplib.h"
#include "webdav.h"
#include "mod_dav.h"

static int mod_dav_init(void);
static int mod_dav_configure(void);
//static int mod_dav_chroot(void);
static int mod_dav_privileges(void);

//static int mod_dav_translate(calipso_request_t *);
static int mod_dav_resource(calipso_request_t *);
static int mod_dav_mime(calipso_request_t *);
static int mod_dav_reply(calipso_request_t *);

int pm_init()
{
    TRACE("register: %s\n",__FILE__);
    //calipso_register_handler("webdav", mod_dav_reply);
    calipso_register_hook( HOOK_INIT, (void *)mod_dav_init);
    calipso_register_hook(HOOK_CONFIGURE, (void *)mod_dav_configure);
    calipso_register_hook(HOOK_PRIVILEGES, (void *)mod_dav_privileges);
    //calipso_register_hook(HOOK_TRANSLATE, (void *)mod_dav_translate);
    calipso_register_hook(HOOK_RESOURCE, (void *)mod_dav_resource);
    calipso_register_hook(HOOK_MIME, (void *)mod_dav_mime);

    return 1;
}

int pm_exit()
{
    TRACE("Exit\n");
    return OK;
}

static int mod_dav_init(void)
{
    return 1;
}

static int mod_dav_configure(void)
{
    return 1;
}

static int mod_dav_privileges(void)
{
    return 1;
}

static int mod_dav_resource(calipso_request_t *request)
{
    //int http_status;
    char *filename = NULL;
    calipso_reply_t *reply;
    calipso_resource_t *resource;
    calipso_pool_t *pool;
    char *method = calipso_request_get_method(request);
    char *uri = calipso_request_get_uri(request);

    /* Set the default handler */
    //calipso_request_set_handler(request, mod_dav_reply);

    /* And stat the resource for other modules to know what type it is */
    reply = calipso_request_get_reply(request);
    //http_status = calipso_reply_get_status(reply);
    pool = calipso_reply_get_pool(reply);
    resource = calipso_reply_get_resource(reply);
    filename = calipso_resource_get_path(resource);

    //if(calipso_http_status_is_redirection(http_status))
    //	return 1;

    TRACE("dav_method: %s %s filename %s\n", method, uri, filename);
    if ( is_file_of(filename, ".php") ) {
        return (0);
    }

    if (strcasecmp(request->method, "GET")  != 0 &&
            strcasecmp(request->method, "HEAD") != 0) {

        if(webdav_invoke(request)) {
            calipso_reply_set_status(reply, HTTP_OK);
        } else {

            //calipso_reply_set_status(reply, HTTP_NOT_IMPLEMENTED);
            return 0;
        }
    }

    return 1;
}

static int mod_dav_mime(calipso_request_t *request)
{

    char *defaultmime = dav_contnet_type;
    calipso_reply_t *reply = calipso_request_get_reply(request);
    calipso_resource_t *resource = calipso_reply_get_resource(reply);
    int http_status = calipso_reply_get_status(reply);

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
        /*
        if(reply) {
        	calipso_reply_set_header_value(reply, "Content-Type", (char*)mime);
        }*/
    }

    return 1;
}

static int mod_dav_reply(calipso_request_t *request)
{
    char *method = calipso_request_get_method(request);

    calipso_reply_t *reply = calipso_request_get_reply(request);
    //calipso_resource_t *resource = calipso_reply_get_resource(reply);


    TRACE("XML-RPC-STATUS: %d\n",calipso_reply_get_status(reply));
    TRACE("method: %s\n", method);

    if(webdav_invoke(request)) {

    } else {

        calipso_reply_set_status(reply, HTTP_NOT_IMPLEMENTED);
    }
    return 1;
}

