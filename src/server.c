/* server.c - server ctx
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

#define SERVER_STATE_NONE 0x0

calipso_server_t *
calipso_server_alloc(void)
{
    calipso_server_t *server = malloc( sizeof(calipso_server_t) );
    //server->pool = cpo_pool_create( CALIPSO_DEFAULT_POOLSIZE );
    //server->config = list_new();
    server->state = SERVER_STATE_NONE;

    return (server);
}

void calipso_server_unalloc(calipso_server_t *server)
{
    //server->pool 
    //server->config  
    free(server);
}

int
calipso_server_set_hostname(calipso_server_t *server, char *hostname)
{
    if (strlcpy(server->hostname, hostname, MAXHOSTNAMELEN) >= MAXHOSTNAMELEN)
        return (0);

    return (1);
}

int
calipso_server_set_serverroot(calipso_server_t *server, char *serverroot)
{
    if (strlcpy(server->serverroot, serverroot, MAXPATHLEN) >= MAXPATHLEN)
        return (0);

    return (1);
}

int
calipso_server_set_documentroot(calipso_server_t *server, char *documentroot)
{
    if (strlcpy(server->documentroot, documentroot, MAXPATHLEN) >= MAXPATHLEN)
        return (0);

    return (1);
}

char *
calipso_server_get_hostname(calipso_server_t *server)
{
    return (server->hostname);
}


int
calipso_server_set_state(calipso_server_t *server, int state)
{
    return ((server->state = state));
}

int
calipso_server_set_config(calipso_server_t *server, calipso_config_t *config)
{
    return ((server->config = config) != NULL);
}

calipso_config_t *
calipso_server_get_config(calipso_server_t *server)
{
    return (server->config);
}


char *
calipso_server_get_documentroot(calipso_server_t *server)
{
    return (server->documentroot);
}

