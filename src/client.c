/* client.c - connection
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
#include "timer.h"


calipso_client_t * calipso_client_alloc(void)
{
	calipso_client_t *client = malloc(sizeof(calipso_client_t));
	if(client == NULL) return NULL;

    return (client);
}

int 
calipso_client_init(calipso_client_t *client)
{
    //client->csocket = 0;
	client->pool = cpo_pool_create(CALIPSO_DEFAULT_POOLSIZE);
    client->pipeline = queue_new();
    client->info = cpo_pool_malloc(client->pool,  sizeof(struct sockaddr_in));

	client->request = NULL;
    calipso_client_set_connect_time(client, (time_t)NULL);
	client->keepalive = client->keepalives = 0;
	client->client_persistent_hdl = NULL;
	client->ctmr = tmr_alrm_create(client, 30);
	client->done = 0;
#ifdef USE_SSL	
	client->ssl = NULL;
#endif
	return CPO_OK;
}

void
calipso_client_unalloc(calipso_client_t *client)
{
	TRACE("@client %d\n", client->csocket);
    //client->csocket = -1;

	tmr_alrm_kill(client->ctmr);

#ifdef USE_SSL
	if(client->ssl) {

		//SSL_set_shutdown(client->ssl, SSL_SENT_SHUTDOWN);
		if(!SSL_shutdown(client->ssl)) {
			SSL_shutdown(client->ssl);
		}
		SSL_free(client->ssl);		
	}
#endif

	if(client->event) {
		cpo_event_unalloc(client->event);
	}

	if (client->request != NULL) {
		calipso_request_unalloc(client->request);
	}

	if(client->pipeline) {
		queue_delete(client->pipeline);
		free(client->pipeline);
	}

	if(client->pool) {
		cpo_pool_destroy(client->pool);
	}

    free(client);
    client = NULL;
}

queue_t *
calipso_client_get_pipeline(calipso_client_t *client)
{
    return (client->pipeline);
}

int
calipso_client_set_pipeline(calipso_client_t *client, queue_t *pipeline)
{
    return ((client->pipeline = pipeline) != NULL);
}

int
calipso_client_set_pool(calipso_client_t *client, calipso_pool_t *pool)
{
    return ((client->pool = pool) != NULL);
}

int
calipso_client_set_socket(calipso_client_t *client, int csocket)
{
    return ((client->csocket = csocket));
}


int
calipso_client_sent_data(calipso_client_t *client)
{
    int	nbytes;

    if (ioctl(calipso_client_get_socket(client), FIONREAD, &nbytes) < 0) {
        perror( "ioctl() call failure");
		exit(-1);
	}

    return (nbytes);
}


int
calipso_client_shutdown(calipso_client_t *client)
{
    //calipso_trigger_hook(HOOK_DISCONNECT, client);
    shutdown(client->csocket, SHUT_RDWR);
    closesocket(client->csocket);
    return CPO_OK;
}

/* handle persistent connection */
int 
calipso_client_handle_keep_alive(calipso_client_t *client) 
{
	calipso_socket_t *listener = calipso_client_get_listener(client);
	
	if(client->client_persistent_hdl) {
		client->client_persistent_hdl(client);
	}
	
	tmr_alrm_reset(client, 15);
	client->done = 0;
	client->keepalive = 0;
	
	if(listener) {
    	queue_t *list = calipso_socket_get_client_list(listener);

		if(list) {
    		queue_remove(list, client);
			queue_enqueue(list, client);
		}
	}

	return CPO_OK;
}

int
calipso_client_disconnect(calipso_client_t *client)
{
    calipso_socket_t *listener = calipso_client_get_listener(client);
    
	if(OK == client->keepalive) {
	
		char buf;
		int n = recv(client->csocket, &buf, 1, MSG_PEEK);	

		if(n <= 0) {
			//printf("n  = %d errno = %d , %s\n", n , errno , strerror(errno));
			if(n== -1 && errno == EAGAIN) {
				return calipso_client_handle_keep_alive(client);
			}

		} else {
				return calipso_client_handle_keep_alive(client);
		}
	}

	if(listener) {
    	queue_t *list = calipso_socket_get_client_list(listener);

		if(list) {
    		queue_remove(list, client);
    		calipso_client_shutdown(client);
    		calipso_client_unalloc(client);
		}
	}

    return (1);
}

int
calipso_client_connection_error(calipso_client_t *client)
{
	TRACE("client error handler %s\n", strerror(errno));
	client->keepalive = NOK;
	return calipso_client_disconnect(client);
}

int
calipso_client_set_request(calipso_client_t *client, calipso_request_t *request)
{
    return ((client->request = request) != NULL);
}

int
calipso_client_get_socket(calipso_client_t *client)
{
    return (client->csocket);
}

calipso_pool_t *
calipso_client_get_pool(calipso_client_t *client)
{
    return (client->pool);
}

int
calipso_client_set_listener(calipso_client_t *client, calipso_socket_t *listener)
{
    return ((client->listener = listener) != NULL);
}

int
calipso_client_set_server(calipso_client_t *client, calipso_server_t *server)
{
    return ((client->server = server) != NULL);
}

int
calipso_client_set_info(calipso_client_t *client, struct sockaddr_in *info)
{
    return ((client->info = info) != NULL);
}

int
calipso_client_set_config(calipso_client_t *client, calipso_config_t *config)
{
    return ((client->config = config) != NULL);
}

calipso_config_t *
calipso_client_get_config(calipso_client_t *client)
{
    return (client->config);
}

calipso_request_t *
calipso_client_get_request(calipso_client_t *client)
{
    return (client->request);
}

calipso_socket_t *
calipso_client_get_listener(calipso_client_t *client)
{
    return (client->listener);
}

calipso_server_t *
calipso_client_get_server(calipso_client_t *client)
{
    return (client->server);
}

int
calipso_client_set_connect_time(calipso_client_t *client, time_t connect_time)
{
    return ((client->connect_time = connect_time)!=0);
}

time_t
calipso_client_get_connect_time(calipso_client_t *client)
{
    return (client->connect_time);
}

const char * 
calipso_client_remote_ip(calipso_client_t *client)
{
	return inet_ntoa(client->info->sin_addr);
}

