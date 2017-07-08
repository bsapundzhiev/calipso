/* mod_proxy.c todo
 *
 * Copyright (C) 2007 Borislav Sapundzhiev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

#include <calipso.h>

static int mod_proxy_init(void);
static int mod_proxy_configure(void);
static int mod_proxy_chroot(void);
//static int mod_proxy_privileges(void);
static int mod_proxy_request(calipso_request_t *request);
static int mod_proxy_translate(calipso_request_t *request);
static int mod_proxy_resource(calipso_request_t *request);
static int mod_proxy_reply(calipso_request_t *request);

int pm_init()
{
    TRACE("register: %s\n",__FILE__);
    calipso_register_handler("*/*", mod_proxy_reply);
    calipso_register_hook( HOOK_INIT, (void *)mod_proxy_init);
    calipso_register_hook(HOOK_CONFIGURE, (void *)mod_proxy_configure);
    calipso_register_hook(HOOK_CHROOT, (void *)mod_proxy_chroot);
    //calipso_register_hook(HOOK_PRIVILEGES, (void *)mod_proxy_privileges);
    calipso_register_hook(HOOK_REQUEST, (void *)mod_proxy_request);
    calipso_register_hook(HOOK_TRANSLATE, (void *)mod_proxy_translate);
    calipso_register_hook(HOOK_RESOURCE, (void *)mod_proxy_resource);
    return 1;
}

int mod_proxy_init()
{
    calipso_socket_t *listener;
    unsigned short port = 8081;
    const char * listen_naddr = "0.0.0.0";
    TRACE("init mod_proxy...\n");
    listener = calipso_do_listen_sock(listen_naddr, port);
    printf("register port: %d\n", listener->port);
    printf("register lsocket: %d\n", listener->lsocket);
    listener->state = SOCKET_STATE_ACTIVE;

    calipso_add_listener( listener );

    return 1;
}

int mod_proxy_configure()
{
    TRACE("TODO");
    return 1;
}

int mod_proxy_chroot()
{
    TRACE("TODO");
    return 1;
}

int mod_proxy_request(calipso_request_t *request)
{
    TRACE("TODO");
    return 1;
}

int mod_proxy_translate(calipso_request_t *request)
{
    TRACE("TODO");
    return 1;
}

int mod_proxy_resource(calipso_request_t *request)
{
    TRACE("TODO");
    return 1;
}

//    /* Set the default handler */
//    calipso_request_set_handler(request, mod_http_reply);
int mod_proxy_reply(calipso_request_t *request)
{
    TRACE("TODO");
    return 1;
}

