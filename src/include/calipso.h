/**
 *
 * Autor:(c) 2004 Borislav Sapundjiev <BSapundjiev_AT_gmail[D0T]com>
 *
 * This file is released under the terms of GPL v2 and any later version.
 *
 */

#ifndef _CALIPSO_H
#define _CALIPSO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <stdint.h>
#include "compat.h"
/* include winsock2 before windows.h */
#include <winsock2.h>
#endif

/* HTTPS support */
#ifdef USE_SSL
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#endif
/*- - - calipso related - - -*/

#include "dt.h" 	/*generic data types*/
#include "array.h"
#include "hash.h"
#include "dllist.h"
#include "queue.h"
#include "event.h"
#include "cpo_log.h"
#include "hooks.h"
#include "core.h"
#include "rfc2616.h"
#include "xmalloc.h"
#include "debug.h"
#include "config.h"
#include "cpo_string.h"
#include "cpo_file.h"

/**
 * inital WIN32 port
 */
#ifdef _WIN32 /*Windows-specific wrappers for socket functions*/

#include <windows.h>
#include <process.h>
#include <io.h>
#include <fcntl.h>
#include <malloc.h>
#include <direct.h>

#include "fakepoll.h"
#include "writev.h"

#pragma comment(lib, "ws2_32.lib")
#ifdef USE_SSL
#pragma comment(lib, "ssleay32.lib")
#endif
#else /* *NIX */

#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pwd.h>  	/* struct passwd */
#include <limits.h> /* IOV_MAX */ 
/*net*/
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <sys/sendfile.h>

#define CPO_INVALID_SOCKET  (int)(~0)
#define CPO_SOCKET_ERROR         (-1)

#define closesocket		close


#ifdef __linux
	#define LINUX
#endif

#ifndef OS
#	define OS "Unix"
#endif

#endif /*!__PLATFORMS__*/

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

//!process

#include "_process.h"

/**
 * Helper marcos
 */
#ifndef MAX				 
#define MAX(a,b)	((a < b) ? (b) : (a))
#endif
#ifndef MIN				 
#define MIN(a,b)	((a < b) ? (a) : (b))
#endif
#ifndef ABS
#define ABS(x)		(((x) < 0) ? -(x) : (x))
#endif

#define ARRAYSZ(arr) 	(sizeof(arr) / sizeof(arr[0]))

#define safe_strrchr(x,y) ((x)?strrchr((x),(y)):NULL)
#define safe_strchr(x,y) ((x)?strchr((x),(y)):NULL)

typedef char cpo_bool;

#define OK 		1
#define NOK 	0
#define CPO_OK	OK
#define CPO_ERR NOK

#define CPO_EAGAIN	(errno == EAGAIN)

#ifndef IOV_MAX
#define IOV_MAX 64
#endif

/*
 * cp_time.c
 */ 
int cpo_http_time(char * buf, time_t *timer);
void cpo_gmtime(struct tm *tm,register const time_t *timer);

/* 
 * file calipso.c
 */
extern calipso_t * calipso;

int
calipso_init(void);

void 
calipso_destroy(void);

void 
calipso_struct_unalloc(calipso_t *c);

int 
calipso_add_listener(calipso_socket_t *listener);

int 
calipso_set_pool(calipso_pool_t *pool);

calipso_pool_t *
calipso_get_pool(void);

hash_t * 
calipso_get_listeners_hash(void);

List * 
calipso_get_listeners_list(void); 

int 
calipso_set_listeners_hash(hash_t *hash);

int 
calipso_set_config(calipso_config_t *config);

calipso_config_t *
calipso_get_config(void);

List *
calipso_get_m_hook(int hook);

/*
void 
calipso_register_hook(int hook, void *callback_fn );

int
calipso_trigger_hook(int hook, ...);
*/
int 
calipso_register_handler(char *type, int (*f)(calipso_request_t*));

void *
calipso_get_handler(char *type);

int 
calipso_set_handler_hash(hash_t *hash);

hash_t *
calipso_get_handler_hash(void);

/* - - - PROCESS - - - */
int
calipso_set_mprocess(calipso_process_t *process);

calipso_process_t *
calipso_get_current_process(void);

int
calipso_set_current_process(calipso_process_t *process);

int
(*calipso_get_mprocess_model(void))(void);

/* - - - modules - - -*/
int 
calipso_modules_init(calipso_config_t *conf, int realy_load);

calipso_mod_t *
calipso_get_module_table(void);

char *
calipso_get_server_string(calipso_config_t *config);

int
calipso_set_pw(void *pw);

int
calipso_set_gr(void *gr);

int
calipso_set_uid(uid_t uid);

int
calipso_set_gid(gid_t gid);

int
calipso_get_uid(void);

int
calipso_get_gid(void);

cpo_log_t * 
calipso_get_log(void);

/*
 * file client.c 
 */
int 
calipso_client_get_socket(calipso_client_t *client);

calipso_client_t * 
calipso_client_alloc(void);
int 
calipso_client_init(calipso_client_t *client);

void
calipso_client_unalloc(calipso_client_t *client);

int 
calipso_client_set_socket(calipso_client_t *client, int csocket);

int 
calipso_client_set_listener(calipso_client_t *client, calipso_socket_t *listener);

int 
calipso_client_set_server(calipso_client_t *client, calipso_server_t *server);

int 
calipso_client_set_config(calipso_client_t *client, calipso_config_t *config);

int
calipso_client_set_request(calipso_client_t *client, calipso_request_t *request);

int 
calipso_client_set_inputbuf(calipso_client_t *client, char *inputbuf);

int 
calipso_client_set_inputbuf_size(calipso_client_t *client, size_t inputbufsz);

int 
calipso_client_set_inputbuf_real_size(calipso_client_t *client, size_t realinputbufsz);

calipso_config_t * 
calipso_client_get_config(calipso_client_t *client);

int
calipso_client_sent_data(calipso_client_t *client);

int
calipso_client_shutdown(calipso_client_t *client);

int
calipso_client_disconnect(calipso_client_t *client);

int
calipso_client_connection_error(calipso_client_t *client);

calipso_socket_t *
calipso_client_get_listener(calipso_client_t *client);

int
calipso_client_read(calipso_client_t *client, size_t nbytes);

int 
calipso_client_write_reply(calipso_client_t *client);

calipso_server_t *
calipso_client_get_server(calipso_client_t *client);

int
calipso_client_set_pool(calipso_client_t *client, calipso_pool_t *pool);

calipso_socket_t *
calipso_client_get_listener(calipso_client_t *client);

int
calipso_client_parse_inputbuf(calipso_client_t *client);

calipso_pool_t *
calipso_client_get_pool(calipso_client_t *client);

int
calipso_client_set_info(calipso_client_t *client, struct sockaddr_in *info);

int
calipso_client_set_connect_time(calipso_client_t *client, time_t connect_time);

const char * 
calipso_client_remote_ip(calipso_client_t *client);

/*
 * file socket.c
 */
#define SOCKET_STATE_ACTIVE_NONE 0x0
#define SOCKET_STATE_ACTIVE 0x1

calipso_socket_t * 
calipso_socket_alloc(void);

void 
calipso_socket_unalloc(calipso_socket_t *socket);

calipso_socket_t * 
calipso_do_listen_sock(const char *naddr,  int16_t);

queue_t * 
calipso_socket_get_client_list(calipso_socket_t *listener);

int 
calipso_socket_get_socketfd(calipso_socket_t *socket);

int16_t 
calipso_socket_get_port(calipso_socket_t *socket );

int 
calipso_socket_set_port(calipso_socket_t *socket , int16_t port);

calipso_client_t *
calipso_socket_accept_client(calipso_socket_t *listener);

ssize_t
(*calipso_socket_get_read_handler(calipso_socket_t *listener))(calipso_client_t *, void *, size_t);

int
calipso_socket_set_read_handler(calipso_socket_t *listener, 
					ssize_t (*r)(calipso_client_t *, void *, size_t));

int
calipso_socket_set_write_handler(calipso_socket_t *listener,
			       ssize_t (*w)(calipso_client_t *, const void *, size_t));

ssize_t
(*calipso_socket_get_write_handler(calipso_socket_t *listener))(calipso_client_t *, const void *, size_t);


int
calipso_socket_add_server(calipso_socket_t *listener, calipso_server_t *server);
/* helpers */
int calipso_socket_set_nonblocking(int  sock, u_int on);
int set_tcp_nopush_option(int s, short enable);
int set_tcp_nodelay_option(int s, short enable);
int set_keep_alive(int socket, short enable);

/* 
 * request.c
 */
calipso_request_t *
calipso_request_alloc(void);

/*default accept_handler - connect events*/
int 
calipso_request_init_handler(calipso_client_t * client);

time_t
calipso_request_get_time(calipso_request_t *request);

void
calipso_request_unalloc(calipso_request_t *request);

calipso_client_t *
calipso_request_get_client(calipso_request_t *request);

int
calipso_request_handler(calipso_request_t *request);

int
calipso_request_set_method(calipso_request_t *request, char *method);

int
calipso_request_set_uri(calipso_request_t *request, char *uri);

char *
calipso_request_get_uri(calipso_request_t *request);

int
calipso_request_set_version(calipso_request_t *request, char *version);

int
calipso_request_set_handler(calipso_request_t *request, int (*handler)(calipso_request_t *));

char *
calipso_request_get_header_value(calipso_request_t *request, char *header);

calipso_config_t *
calipso_request_get_config(calipso_request_t *request);

calipso_reply_t *
calipso_request_get_reply(calipso_request_t *request);

int
calipso_request_set_client(calipso_request_t *request, calipso_client_t *client);
/* public: */
calipso_pool_t *
calipso_request_get_pool(calipso_request_t *request);

char *
calipso_request_get_method(calipso_request_t *request);

char * 
calipso_request_get_querystring(calipso_request_t *request);

int 
calipso_request_set_querystring(calipso_request_t *request, char * querystring);

char *
calipso_request_get_version(calipso_request_t *request);

/*
 * reply.c
 */
calipso_reply_t *
calipso_reply_alloc(void);

int 
calipso_reply_init(calipso_reply_t *reply); 

void
calipso_reply_unalloc(calipso_reply_t *reply);

int
calipso_reply_send_header(calipso_reply_t *reply);

int
calipso_reply_handler(calipso_reply_t * reply);

int
calipso_reply_print(calipso_reply_t *reply, char *data, size_t nbytes);

int
calipso_reply_printf(calipso_reply_t *reply, char *fmt, ...);

int
calipso_reply_set_client(calipso_reply_t *reply, calipso_client_t *client);

int
calipso_reply_set_handler(calipso_reply_t *reply, int (*handler)(calipso_reply_t *));

int
calipso_reply_set_header_value(calipso_reply_t *p, char *header, char *fmt, ...);

int
calipso_reply_unset_header_value(calipso_reply_t *p, const char *header);

char *
calipso_reply_get_header_value(calipso_reply_t *reply, char *header);

int
calipso_reply_set_status(calipso_reply_t *reply, int status);

int
calipso_reply_get_status(calipso_reply_t *reply);

int
calipso_reply_set_request(calipso_reply_t *reply, calipso_request_t *request);

calipso_config_t *
calipso_reply_get_config(calipso_reply_t *reply);

calipso_client_t *
calipso_reply_get_client(calipso_reply_t *reply);

char *
calipso_replay_get_server_address(calipso_reply_t* reply);

calipso_resource_t *
calipso_reply_get_resource(calipso_reply_t *reply);

calipso_pool_t *
calipso_reply_get_pool(calipso_reply_t *reply);

/*
 * fd.c
 */
int fchk(int fd); /*check file type*/
ssize_t fd_write(int fd, const void *buf, size_t len);
ssize_t fd_read(int fd, void *buf, size_t len);
long int calipso_sendfile(int out_fd, int in_fd,  size_t size );
ssize_t splice_sendfile(int infd, int outfd, off_t *offset , ssize_t size);
ssize_t cpo_io_read(calipso_client_t * client, void *buf, size_t len);
ssize_t cpo_io_write(calipso_client_t * client, const void *buf, size_t len);

/* aio */
ssize_t fd_aio_write(int fd, const void *buf, size_t len);
ssize_t fd_aio_read(int fd, void *buf, size_t len);
/*
 * signal .h
 */
int calipso_init_all_signal_handlers(void);

/*
 * pool.c
 */

/* Create a new pool -- pass zero for default */
/* 'size' is the size of the first blob of memory that will be allocated */
struct mpool *
cpo_pool_create(size_t size);

/* Free a pool */
void cpo_pool_destroy(struct mpool *pool);
/* Allocate from a pool */
void *
cpo_pool_malloc(struct mpool *pool, size_t size);
/* free a pool chunk */
void cpo_pool_free(struct mpool *pool, void * data);
/* pool lib */
char *
cpo_pool_strdup(struct mpool *pool, const char *str);

char *
cpo_pool_strndup(struct mpool *pool, const char *str, size_t len);

char *
cpo_pool_strndup_lower(struct mpool *pool, const char *s, size_t len);

char *
cpo_pool_strndup_upper(struct mpool *pool, const char *s, size_t len);

extern int
cpo_pool_vasprintf(/*struct mpool *pool, char **buf, const char *format, va_list ap*/);

/*
 * resource.c
 */

calipso_resource_t *
calipso_resource_alloc(void);

void
calipso_resource_unalloc(calipso_resource_t *resource);

int
calipso_resource_set_path(calipso_resource_t *resource, char *path);

char *
calipso_resource_get_path(calipso_resource_t *resource);

int
calipso_resource_set_file_descriptor(calipso_resource_t *resource, int fd);

int
calipso_resource_get_file_descriptor(calipso_resource_t *resource);

int
calipso_resource_set_stat(calipso_resource_t *resource, void *sb);

void *
calipso_resource_get_stat(calipso_resource_t *resource);

int
calipso_resource_is_set(calipso_resource_t *resource);

int
calipso_resource_is_file(calipso_resource_t *resource);

int
calipso_resource_is_directory(calipso_resource_t *resource);

uintmax_t
calipso_resource_get_size(calipso_resource_t *resource);

/*
 * server.c
 */
calipso_server_t *
calipso_server_alloc(void);

void 
calipso_server_unalloc(calipso_server_t *server);

int
calipso_server_set_serverroot(calipso_server_t *server, char *serverroot);

int
calipso_server_set_hostname(calipso_server_t *server, char *hostname);

int
calipso_server_set_hostname(calipso_server_t *server, char *hostname);

char *
calipso_server_get_hostname(calipso_server_t *server);

int
calipso_server_set_state(calipso_server_t *server, int state);

int
calipso_server_set_config(calipso_server_t *server, calipso_config_t *config);

calipso_config_t *
calipso_server_get_config(calipso_server_t *server);

int
calipso_server_set_documentroot(calipso_server_t *server, char *documentroot);

char * 
calipso_server_get_documentroot(calipso_server_t *server);

/*
 * http.c
 */
char *
calipso_http_status_get_message( int code );

int 
calipso_http_status_is_info(int status);

int 
calipso_http_status_is_successful(int status);

int 
calipso_http_status_is_redirection(int status);

int 
calipso_http_status_is_client_error(int status);

int 
calipso_http_status_is_server_error(int status);

int 
calipso_http_status_is_error(int status);


#endif /*!_CALIPSO_H*/
