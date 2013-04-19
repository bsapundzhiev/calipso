/* Simple HTTP SSL implementation
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
#include "rfc2616.h"
#include "cplib.h"
#include "mod_http_ssl.h"
#include "cpo_io_ssl.h"

#define RSA_SERVER_CERT     "../doc/calipso_ca_cert.pem" 
#define RSA_SERVER_KEY		"../doc/calipso_privkey.pem"
#define RSA_SERVER_CA_CERT	"../doc/calipso_ca_cert.pem"


static int init_SSL_ctx(calipso_socket_t *listener);
static int mod_http_ssl_accept_callback(calipso_client_t * client);

static int mod_http_ssl_init(void);
static int mod_http_ssl_configure(void);
static int mod_http_ssl_chroot(void);
static int mod_http_ssl_privileges(void);

#define SERVER_KEEPALIVES_MAX 100
#define SSL_ERROR(err) if ((err)==-1) { ERR_print_errors_fp(stderr); exit(1); }

int pm_init()
{
    TRACE("register: %s\n",__FILE__);
    //calipso_register_handler("*/*", mod_http_reply); 
    calipso_register_hook(HOOK_INIT, (void *)mod_http_ssl_init);
    calipso_register_hook(HOOK_CONFIGURE, (void *)mod_http_ssl_configure);
    calipso_register_hook(HOOK_CHROOT, (void *)mod_http_ssl_chroot);
    calipso_register_hook(HOOK_PRIVILEGES, (void *)mod_http_ssl_privileges);

    return 1;
}

int pm_exit()
{
	TRACE("HOOK_EXIT\n");
	return 0;
}

static int mod_http_ssl_init()
{
    const char *documentroot;
    const char *hostname;
    int portn;
    const char *listen_naddr;

    calipso_socket_t *listener;
    calipso_server_t *server;
    calipso_config_t *calipso_config;

    calipso_config = calipso_get_config();

    server = calipso_server_alloc();

    portn = 443;
    listen_naddr = config_get_option(calipso_config, "listen", NULL);
    listener 	= calipso_do_listen_sock(listen_naddr, portn);
    hostname	= config_get_option(calipso_config, "server_host", NULL);
    documentroot 	= config_get_option(calipso_config, "server_docroot", NULL);
	
	server->keep_alive_max = SERVER_KEEPALIVES_MAX;
    calipso_server_set_hostname(server, (char*)hostname);
    calipso_server_set_documentroot(server, (char*)documentroot);
	
	init_SSL_ctx(listener);
	//calipso_socket_set_accept_callback(listener, mod_http_ssl_accept_callback);
	listener->accept_callback = (void*)mod_http_ssl_accept_callback;
	//r/w handlers
	//listener->r = (void*)fd_ssl_read;
	//listener->w = (void*)fd_ssl_write;
    calipso_socket_add_server(listener, server);

    printf("register port: %d\n", listener->port);
    printf("register lsocket: %d\n", listener->lsocket);
	
    listener->state = SOCKET_STATE_ACTIVE;
    calipso_add_listener( listener );

    return OK;
}

static int mod_http_ssl_configure()
{
	return OK;
}

static int mod_http_ssl_chroot()
{

    const char *chrootenable;
    const char *chrootpath;
    calipso_config_t *config;

    config = calipso_get_config();

    /* Check wether chroot()-ing is required, chroot is default behaviour */
    chrootenable = config_get_option(config, "chroot", NULL);

    if (chrootenable == NULL /*|| eoz_config_is_enabled(chrootenable)*/) {
        if ( calipso_get_uid() )
            printf("Chroot() must be disabled as non privileged user.");

        chrootpath = config_get_option(config, "chrootpath", NULL);

        if (chrootpath == NULL)
            printf("Chroot() directive enabled, but no ChrootPath() set.");
        if (chroot(chrootpath) < 0)
            printf("chroot() call failure in %s.", __func__);
        if (chdir("/") < 0)
            printf("chdir() call failure in %s.", __func__);
    }

    return 1;
}

static int mod_http_ssl_privileges(void)
{
    return NOK;
}

static int s_server_session_id_context = 1;

static int session_new_callback(SSL *ssl, SSL_SESSION *sess)
{
	printf("session_new_callback CALL %p\n", sess);	
	return OK;
}

static void session_rem_callback(SSL_CTX *ctx, SSL_SESSION *sess)
{
	printf("session_rem_callback CALL %p\n", sess);	
}

int init_SSL_ctx(calipso_socket_t *listener) 
{
	SSL_CTX *ctx;
   	const SSL_METHOD *meth;
   	//X509 *client_cert = NULL;
	char verify_client = OK; //NOK;
 
	/* Create a SSL_METHOD structure (choose a SSL/TLS protocol version) */
	meth =  SSLv23_method();
 
 	/* Create a SSL_CTX structure */
    ctx = SSL_CTX_new(meth);
 	if (!ctx) {
    	ERR_print_errors_fp(stderr);
     	exit(1);
 	}

 	//SSL_CTX_set_cipher_list(ctx, "RC4-SHA");
	//SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY |  SSL_MODE_ENABLE_PARTIAL_WRITE );

	/* Load the server certificate into the SSL_CTX structure */
	if (SSL_CTX_use_certificate_file(ctx, RSA_SERVER_CERT, SSL_FILETYPE_PEM) <= 0) { 
		ERR_print_errors_fp(stderr); 
		exit(1); 
	}
 
	/* Load the private-key corresponding to the server certificate */
	if (SSL_CTX_use_PrivateKey_file(ctx, RSA_SERVER_KEY, SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
        exit(1);
    }
 
   	/* Check if the server certificate and private-key matches */
    if (!SSL_CTX_check_private_key(ctx)) {
    	fprintf(stderr,"Private key does not match the certificate public key\n");
        exit(1);
    }

	//SSL_CTX_set_quiet_shutdown(ctx, 1);


	if(verify_client == OK) {
		/*SSL session support*/
		//SSL_CTX_sess_set_new_cb(ctx, session_new_callback);
		//SSL_CTX_sess_set_remove_cb(ctx, session_rem_callback);

		SSL_CTX_set_session_id_context(ctx,(void*)&s_server_session_id_context, 
			sizeof (s_server_session_id_context));
		SSL_CTX_set_timeout(ctx, 300);

		// to share the session, we need to ensure the SSL CTX is using CACHE_CLIENT
		long mode = SSL_CTX_get_session_cache_mode( ctx );
		mode |= SSL_SESS_CACHE_CLIENT /*| SSL_SESS_CACHE_SERVER*/;
		SSL_CTX_set_session_cache_mode( ctx, mode );

   		/* Load the RSA CA certificate into the SSL_CTX structure */
     	if (!SSL_CTX_load_verify_locations(ctx, RSA_SERVER_CA_CERT, NULL)) {
        	ERR_print_errors_fp(stderr);
           	exit(1);
      	}
 
    	/* Set to require peer (client) certificate verification */
        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
 
      	/* Set the verification depth to 1 */
        SSL_CTX_set_verify_depth(ctx,1);
	}
	
	listener->ssl_ctx = ctx;

	return OK;
}

static int mod_http_ssl_accept_callback(calipso_client_t * client)
{
	int ret=0, err;
	calipso_socket_t *listener = calipso_client_get_listener(client);
    SSL *ssl = SSL_new(listener->ssl_ctx);

	if(!ssl) {
    	ERR_print_errors_fp(stderr);
		exit(1);
     	return NOK;
 	}
	
 	calipso_request_init_handler(client);

	SSL_SESSION *session = SSL_get1_session(ssl); 
	printf("SSL_SESSION %p\n" , session);
	//if(session)
	//SSL_set_session(ssl, session) ;

	SSL_set_nonblocking(ssl);	
	SSL_set_fd(ssl, client->csocket);
	//SSL_set_accept_state(ssl);
	ret = SSL_accept(ssl);

	printf("SSL_accept: client %d ret %d err %d\n", client->csocket, ret, err);
	printf("SSL_REUSED %ld\n", SSL_session_reused(ssl));
//while(ret < 0)
//{
//	ret = SSL_do_handshake(ssl);

	if(ret < 0) {
		
		err = SSL_get_error(ssl, ret);
		switch(err) {
			case SSL_ERROR_WANT_READ: 
				TRACE("SSL_ERROR_WANT_READ\n");
				break;
			case SSL_ERROR_WANT_WRITE:
				TRACE("SSL_ERROR_WANT_WRITE\n");
				break;
			default:{
				//SSL_ERROR(ret);
				SSL_shutdown(ssl);
				SSL_free(ssl);
				return OK;
			}
		}
	}
//}
	client->ssl = ssl;
	client->listener->r = cpo_io_ssl_read; //(void*)fd_ssl_read;
	client->listener->w = cpo_io_ssl_write; //(void*)fd_ssl_write;

	return OK;
}

