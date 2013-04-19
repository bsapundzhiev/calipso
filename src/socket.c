/* socket.c - server listners
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

#define NON_BLOCKING_MODE 		1
#define MAX_SERVERS_TABLE 		32

#define USE_LINGER				1
#define USE_TCP_KEEPALIVE		0
#define USE_TCP_NOPUSH			0
#define USE_TCP_NODELAY			0

/* keepalive settings */
#define CPO_KEEPALIVE_IDLE_TIME	5000
#define CPO_KEEPALIVE_CNT		5
#define CPO_KEEPALIVE_INTVL		1

#define CP_SSO(SOCK, LEVEL, OPT, VAL)\
if (setsockopt(SOCK, LEVEL, OPT, (const char*)&VAL, sizeof(VAL)) < 0) {\
perror("setsockopt() call failure.");\
close(SOCK);\
return (-1);\
}

static int socket_create_inet(const char *addr, int16_t portn);
static int socket_set_options(int lsocket);
static char * calipso_socket_get_peer_name(int s);


calipso_socket_t *
calipso_socket_alloc(void)
{
    calipso_socket_t * socket = malloc(sizeof(calipso_socket_t));
	if(!socket) return NULL;

    socket->config = NULL;//list_new();
    socket->server = hash_table_create(MAX_SERVERS_TABLE, NULL);
    socket->client = queue_new();
    socket->lsocket = 0;
    socket->state = 0;
    socket->port  = 0;
    socket->accept_callback = NULL;
    socket->w = cpo_io_write;
    socket->r = cpo_io_read;
	
    return (socket);
}

void
calipso_socket_unalloc(calipso_socket_t *socket)
{
  	size_t i;

	if(socket->server) 
	{
		hash_node_t* node;
		for(i=0; i<  hash_table_get_size(socket->server); ++i) {
			
			for(node = socket->server->nodes[i]; 
					node != NULL; node = node->next) {
				
				printf("unalloc server %s \n", node->key);
				calipso_server_unalloc(node->data);
			}
		}

		hash_table_destroy(socket->server);
	}

	if(socket->config) {
		list_delete(socket->config);
		free(socket->config);
	}

#ifdef USE_SSL
	if(socket->ssl_ctx) {
		SSL_CTX_free(socket->ssl_ctx);
	}
#endif

	if(socket->event) {
		cpo_event_unalloc(socket->event);
	}

	if(socket->client) {
		queue_delete(socket->client);
		free(socket->client);
	}

    free(socket);
}


int
calipso_socket_get_socket(calipso_socket_t *listener)
{
    return (listener->lsocket);
}

queue_t *
calipso_socket_get_client_list(calipso_socket_t *listener)
{
    return (listener->client);
}

hash_t *
calipso_socket_get_server_hash(calipso_socket_t *listener)
{
    return (listener->server);
}

int16_t
calipso_socket_get_port(calipso_socket_t *socket)
{
    return (socket->port);
}

int calipso_socket_set_port(calipso_socket_t *socket , int16_t port)
{
    return ((socket->port = port));
}

int calipso_socket_set_socketfd(calipso_socket_t *socket, int lsocket)
{
    return ((socket->lsocket = lsocket));
}

int calipso_socket_get_socketfd(calipso_socket_t *socket)
{
    return ((socket->lsocket));
}

calipso_client_t *
calipso_socket_accept_client(calipso_socket_t *listener)
{
    socklen_t len;
    calipso_client_t *client;
    calipso_server_t *server;
    hash_node_t * server_hash;

	/* allocate resource */
    client = calipso_client_alloc();
	if(client == NULL)	return NULL;
	
	calipso_client_init(client);

	len = sizeof(struct sockaddr_in);
    client->csocket = accept(listener->lsocket, (struct sockaddr *)client->info, &len);

    if (client->csocket < 0) {
		calipso_client_unalloc(client);
        cpo_log_error(calipso->log, " accept() call failure: %s\n", strerror(errno));
        return NULL;
    } 

	//TRACE("Accept client %d\n", client->csocket);
	//calipso_client_set_connect_time(client, 
	//		calipso_process_get_time(calipso_get_current_process()));
	
	calipso_socket_set_nonblocking(client->csocket, CPO_OK);
	
	calipso_client_set_listener(client, listener);

	//XXX: vhost key = hostname - move to request
	server_hash = hash_get_first_entry( listener->server );
	if(!server_hash) {
		fprintf(stderr, "%s() client without server\n", __FUNCTION__);
		exit(1);
	}

	server = (calipso_server_t *)server_hash->data;
	if(server) {
		//TRACE("server->documentroot %s\n", server->documentroot);
		//TRACE("lsocket name %s\n", host);
		calipso_client_set_server(client, server);
	}

	/* init client event */
	client->event = cpo_event_alloc(EVENT_CONNECTION);
	client->event->data = client;

	queue_enqueue(listener->client, client);
	
	if (listener->accept_callback) {
		listener->accept_callback(client);
	}
	
    return (client);
}

int
calipso_socket_set_read_handler(calipso_socket_t *listener,
                                ssize_t (*r)(calipso_client_t *, void *, size_t))
{
    return ((listener->r = r) != NULL);
}


ssize_t
(*calipso_socket_get_read_handler(calipso_socket_t *listener))(calipso_client_t *, void *, size_t)
{
    return (listener->r);
}

int
calipso_socket_set_write_handler(calipso_socket_t *listener,
                                 ssize_t (*w)(calipso_client_t *, const void *, size_t))
{
    return ((listener->w = w) != NULL);
}

ssize_t
(*calipso_socket_get_write_handler(calipso_socket_t *listener))(calipso_client_t *, const void *, size_t)
{
    return (listener->w);
}

int
calipso_socket_set_accept_callback(calipso_socket_t *listener,
                                   int (*accept_callback)(calipso_client_t *))
{
    return ((listener->accept_callback = (void *)accept_callback) != NULL);
}

int
calipso_socket_add_server(calipso_socket_t *listener, calipso_server_t *server)
{
    return (hash_table_insert(listener->server, 
				calipso_server_get_hostname(server), server) == 0);
    
}

calipso_socket_t * calipso_do_listen_sock(const char *naddr, int16_t portn)
{
	int sd = socket_create_inet(naddr, portn);
  	calipso_socket_t * socket = calipso_socket_alloc(); 

	if(socket && sd != -1) {
    	calipso_socket_set_port(socket, portn);
    	calipso_socket_set_socketfd(socket, sd);
		/* init listener event */
		socket->event = cpo_event_alloc(EVENT_LISTENER);
		socket->event->data = socket;
	} else {
		cpo_log_error(calipso->log, "Can't create lisnten socket %s", strerror(errno));
		exit(-1);
	}

    return (socket);
}

int set_keep_alive(int socket, short enable) 
{
	int on = enable;
	int keep_alive_idle = CPO_KEEPALIVE_IDLE_TIME;
	int keep_alive_cnt = CPO_KEEPALIVE_CNT;
	int keep_alive_intvl = CPO_KEEPALIVE_INTVL;

	CP_SSO(socket, SOL_SOCKET, SO_KEEPALIVE, on);
#ifndef _WIN32
    CP_SSO(socket, IPPROTO_TCP, TCP_KEEPIDLE, keep_alive_idle);
    CP_SSO(socket, IPPROTO_TCP, TCP_KEEPCNT, keep_alive_cnt);
    CP_SSO(socket, IPPROTO_TCP, TCP_KEEPINTVL, keep_alive_intvl);
#endif
    return CPO_OK;
}

static int socket_set_options(int lsocket)
{
	struct linger l;
	int linger_timeout =5;
	int yes=1, no=0;

#ifdef SO_REUSEPORT
    CP_SSO(lsocket, SOL_SOCKET, SO_REUSEPORT, yes);
#endif

#ifdef SO_REUSEADDR 
    CP_SSO(lsocket, SOL_SOCKET, SO_REUSEADDR, yes);
#endif

	//XXX: use lignering socket ? 
#if USE_LINGER 
#ifdef SOL_SOCKET
	l.l_onoff = 1;
	l.l_linger = 30;
   	CP_SSO(lsocket, SOL_SOCKET, SO_LINGER, l);
#endif
#ifdef TCP_LINGER2
	CP_SSO(lsocket, SOL_SOCKET, TCP_LINGER2, linger_timeout);
#endif 
#endif

#if USE_TCP_KEEPALIVE
	set_keep_alive(lsocket, yes);
#endif

#if USE_TCP_NODELAY
	set_tcp_nodelay_option(lsocket, yes);
#endif

#if USE_TCP_NOPUSH
	set_tcp_nopush_option(lsocket, yes);
#endif

#ifdef TCP_QUICKACK
 	CP_SSO(lsocket, IPPROTO_TCP, TCP_QUICKACK, yes);
#endif

#ifdef TCP_DEFER_ACCEPT
	CP_SSO(lsocket, IPPROTO_TCP, TCP_DEFER_ACCEPT, no);
#endif

	return CPO_OK;
}

static int socket_create_inet(const char *addr, int16_t portn)
{
    int ret;
    int lsocket;
    struct sockaddr_in sin;
	u_int listen_backlog = LISTEN_BACKLOG;

#ifdef _WIN32
	WSADATA wsaData;
	ret = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
	if ( ret != 0 ) {
		fprintf(stderr, "Error: WSAStartup %s\n", strerror(errno));
		return -1;
	}
#endif

    /* create a TCP/IP stream socket to listen with */
    lsocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (lsocket < 0 ) {
        perror("socket()");
        close(lsocket);
        return -1;
    }

	/* setup Socket/TCP/IP */
	socket_set_options(lsocket);

    sin.sin_family      = AF_INET;
    sin.sin_port        = htons(portn);
	sin.sin_addr.s_addr = (addr == NULL) ? htonl(INADDR_ANY) : inet_addr(addr);

    ret = bind(lsocket, (struct sockaddr*)&sin, sizeof( struct sockaddr_in ) );

    if (ret < 0) {
        fprintf(stderr, "Error: bind %s:%d %s\n", inet_ntoa(sin.sin_addr), portn, strerror(errno));
        close(lsocket);
        return -1;
    }

    TRACE("Listen on %s:%d\n", inet_ntoa(sin.sin_addr), portn);

#ifdef NON_BLOCKING_MODE
    ret = calipso_socket_set_nonblocking(lsocket, CPO_OK);
    if ( -1 == ret ) {
        perror("failed to set non-block mode on socket\n");
        return -1;
    }
    TRACE("Server in O_NONBLOCK\n");
#endif

    if ( listen( lsocket, listen_backlog) != 0 ) {
        perror("listen()");
        close( lsocket );
        return -1;
    }

    return (lsocket);
}

int calipso_socket_set_nonblocking(int sock, u_int on)
{
#ifdef FIONBIO
    unsigned long	nbParm = on;
    return ioctl(sock, FIONBIO, &nbParm);
#else
    return fcntl(sock, F_SETFL, (on ? O_NONBLOCK : 0));
#endif
}
 
int set_tcp_nopush_option(int s, short enable) 
{
#ifndef _WIN32
	int yes = enable;
	CP_SSO(s, IPPROTO_TCP, TCP_CORK, yes);
#endif
	return OK;
}

int set_tcp_nodelay_option(int s, short enable)
{
	int yes = enable;
	CP_SSO(s, IPPROTO_TCP, TCP_NODELAY, yes);
    return OK;
}

char * calipso_socket_get_peer_name(int s)
{
	struct sockaddr_in sa;
   	int sa_len = sizeof(sa);
                           
   	if (getpeername(s, (struct sockaddr *)&sa, (socklen_t *)&sa_len) == -1) {
    	perror("getpeername(): ");
    	return NULL;
   	}
	/*
   	printf("Local IP address is: %s\n", inet_ntoa(sa.sin_addr));
   	printf("Local port is: %d\n", (int) ntohs(sa.sin_port));
  	*/
	return inet_ntoa(sa.sin_addr);
}

