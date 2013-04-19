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
#include "mod_dav.h"

static int mod_dav_init(void);
static int mod_dav_configure(void);
static int mod_dav_chroot(void);
static int mod_dav_privileges(void);

static int mod_dav_translate(calipso_request_t *);
static int mod_dav_resource(calipso_request_t *);
static int mod_dav_mime(calipso_request_t *);
static int mod_dav_reply(calipso_request_t *);

int pm_init()
{
	TRACE("register: %s\n",__FILE__);
	calipso_register_handler("*/RPC", mod_dav_reply);
	calipso_register_hook( HOOK_INIT, (void *)mod_dav_init);
	calipso_register_hook(HOOK_CONFIGURE, (void *)mod_dav_configure);
	calipso_register_hook(HOOK_PRIVILEGES, (void *)mod_dav_privileges);
	calipso_register_hook(HOOK_TRANSLATE, (void *)mod_dav_translate);
	calipso_register_hook(HOOK_RESOURCE, (void *)mod_dav_resource);
	calipso_register_hook(HOOK_MIME, (void *)mod_dav_mime);

	return 1;
}


static int mod_dav_init(void) 
{
	return 1;
}


static int mod_dav_configure(void)
{

	return 1;
}


static int mod_dav_chroot(void) 
{

	return 1;
}

static int mod_dav_privileges(void) {

	return 1;	
}

static int mod_dav_translate(calipso_request_t *request) {

	//char *pathname = NULL;
	char *uri = NULL;
	//char *documentroot = NULL;
	//calipso_server_t *server;
	calipso_client_t *client;
	//calipso_reply_t *reply;
	//calipso_resource_t *resource;
	//calipso_pool_t *pool;

	client = calipso_request_get_client(request);
	//reply  = calipso_request_get_reply(request);
	//server = calipso_client_get_server(client);
	//pool   = calipso_request_get_pool(request);
	//resource = calipso_reply_get_resource(reply);

	uri = calipso_request_get_uri(request);

	TRACE("XML-RPC-URI: %s\n", uri);

	if ( strcmp(uri ,"/RPC") ) {
		//calipso_reply_set_status(reply, HTTP_BAD_REQUEST);
		return (1);
	}

	

	return 1;
}

static int mod_dav_resource(calipso_request_t *request) {


	int fd;
	char *filename = NULL;
	struct stat sb;
	calipso_reply_t *reply;
	calipso_resource_t *resource;
	calipso_pool_t *pool;

	/* Set the default handler */
	calipso_request_set_handler(request, mod_dav_reply);

	/* And stat the resource for other modules to know what type it is */
	reply = calipso_request_get_reply(request);
	pool = calipso_reply_get_pool(reply);
	resource = calipso_reply_get_resource(reply);
	filename = calipso_resource_get_path(resource);

	TRACE("XML-RPC: %s TODO: pool_open\n", filename);
	
	//calipso_reply_set_status(reply, HTTP_NOT_IMPLEMENTED);
	
	return (1);
	
}

static int mod_dav_mime(calipso_request_t *request) {

	char *defaultmime = "text/xml";
	//char *mime = NULL;

	calipso_reply_t *reply;
	
	/* mime already set */
	if(!calipso_http_status_is_error(request->reply->state))
		return 0;

	if (defaultmime == NULL)
		defaultmime = "text/plain";

	reply = calipso_request_get_reply(request);

	calipso_reply_set_header_value(reply, "Content-Type", defaultmime);

	return 1;
}

static int mod_dav_reply(calipso_request_t *request)
{
	char *method = calipso_request_get_method(request);

	calipso_reply_t *reply = calipso_request_get_reply(request);
	//calipso_resource_t *resource = calipso_reply_get_resource(reply);
	

	TRACE("XML-RPC-STATUS: %d\n",calipso_reply_get_status(reply));
	TRACE("method: %s\n", method);

	calipso_reply_set_status(reply, HTTP_NOT_IMPLEMENTED);
	return 1;
}

