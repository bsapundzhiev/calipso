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

#define SSL_ERROR(err) if ((err)==-1) { ERR_print_errors_fp(stderr); exit(1); }

#define SSL_USE_QUIET_SHUTDOWN 	0

struct http_ssl_conf_ctx {
	unsigned int portn;
	unsigned char use_ssl;
	char * ssl_cert_file;
	char * ssl_key_file; 
	char * ssl_chiper;
};

static void config_port_handler(struct http_ssl_conf_ctx *ctx,  void *val);
static void config_ssl_handler(struct http_ssl_conf_ctx *ctx,  void *val);
static void config_ssl_cert_handler(struct http_ssl_conf_ctx *ctx,  void *val);
static void config_ssl_cert_key_handler(struct http_ssl_conf_ctx *ctx,  void *val);
static void config_ssl_chiper_handler(struct http_ssl_conf_ctx *ctx,  void *val);

#define CONF_BLOCK_CTX "server"

cfg_t conf_opt[] = {
	{ "server_port", NULL, (void*)config_port_handler },
	{ "use_ssl", NULL, (void*)config_ssl_handler },
	{ "use_ssl_rsa_certfile", NULL, (void*)config_ssl_cert_handler},
	{ "use_ssl_rsa_keyfile", NULL, (void*)config_ssl_cert_key_handler },
	{ "use_ssl_chiper", NULL, (void*)config_ssl_chiper_handler }
};

/* module */
static int init_SSL_ctx(struct http_ssl_conf_ctx *conf_ctx, calipso_socket_t *listener);
static int mod_http_ssl_accept_callback(calipso_client_t * client);

static int mod_http_ssl_init(void);

int pm_init()
{
    TRACE("register: %s\n",__FILE__);
    calipso_register_hook(HOOK_INIT, (void *)mod_http_ssl_init);

    return CPO_OK;
}

int pm_exit()
{
	TRACE("HOOK_EXIT\n");
	return 0;
}

static void config_port_handler(struct http_ssl_conf_ctx *ctx,  void *val)
{
	ctx->portn = val ?  atoi((const char *)val) : 0;
}

static void config_ssl_handler(struct http_ssl_conf_ctx *ctx,  void *val)
{
	ctx->use_ssl = val ? OK : NOK;
}

static void config_ssl_cert_handler(struct http_ssl_conf_ctx *ctx,  void *val)
{
	ctx->ssl_cert_file = val;
	
}

static void config_ssl_cert_key_handler(struct http_ssl_conf_ctx *ctx,  void *val)
{
	ctx->ssl_key_file = val;
}

static void config_ssl_chiper_handler(struct http_ssl_conf_ctx *ctx,  void *val)
{
	ctx->ssl_chiper = val;
}

static int config_parse_run(struct http_ssl_conf_ctx * ctx, const char *option, void *value)
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

static int mod_http_ssl_init_server_ctx(struct http_ssl_conf_ctx * ctx)
{
	List *l, *listeners = calipso_get_listeners_list();

	if(listeners == NULL || ctx->use_ssl ==0) 
		return CPO_ERR;
	
	for(l = list_get_first_entry( listeners );
		l != NULL;
		l = list_get_next_entry( l )) {
	
        calipso_socket_t *listener = list_get_entry_value( l );
		
		if(listener->state & SOCKET_STATE_INIT_SSL 
			&& ctx->portn == listener->port) {
			
			init_SSL_ctx(ctx, listener); 
		}
	}

	list_delete(listeners);
 	free(listeners);

	return CPO_OK;
}

static int mod_http_ssl_init_config(calipso_config_t * config)
{
	struct http_ssl_conf_ctx * ctx = NULL;

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
						ctx = xmalloc(sizeof(*ctx));
						config_parse_run(ctx, NULL, NULL);
					}
				}

				if(CTX_BLOCK_END == state)	{
					mod_http_ssl_init_server_ctx(ctx);
					if(ctx) {
						free(ctx);
						ctx = NULL;
					}
				}
	
				if(ctx) {
					config_parse_run(ctx, c->option, c->value);
				}
			}
		}
    }

	if(ctx) {
		free(ctx);
		ctx=NULL;
	}

	return CPO_OK;
}

static int mod_http_ssl_init()
{
    calipso_config_t *calipso_config = calipso_get_config();
	/* new config init */
	return mod_http_ssl_init_config(calipso_config);
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

int init_SSL_ctx(struct http_ssl_conf_ctx *conf_ctx, calipso_socket_t *listener) 
{
	SSL_CTX *ctx;
   	const SSL_METHOD *meth;
   	//X509 *client_cert = NULL;
	char verify_client = OK;
 
	/* Create a SSL_METHOD structure (choose a SSL/TLS protocol version) */
	meth =  SSLv23_method();
 
 	/* Create a SSL_CTX structure */
    ctx = SSL_CTX_new(meth);
 	if (!ctx) {
    	ERR_print_errors_fp(stderr);
     	exit(1);
 	}

	if(conf_ctx->ssl_chiper) {
 		SSL_CTX_set_cipher_list(ctx, conf_ctx->ssl_chiper);
	}

	//SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY |  SSL_MODE_ENABLE_PARTIAL_WRITE );

	/* Load the server certificate into the SSL_CTX structure */
	if (SSL_CTX_use_certificate_file(ctx, conf_ctx->ssl_cert_file, SSL_FILETYPE_PEM) <= 0) { 
		ERR_print_errors_fp(stderr); 
		exit(1); 
	}
 
	/* Load the private-key corresponding to the server certificate */
	if (SSL_CTX_use_PrivateKey_file(ctx, conf_ctx->ssl_key_file, SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
        exit(1);
    }
 
   	/* Check if the server certificate and private-key matches */
    if (!SSL_CTX_check_private_key(ctx)) {
    	fprintf(stderr,"Private key does not match the certificate public key\n");
        exit(1);
    }

	SSL_CTX_set_quiet_shutdown(ctx, SSL_USE_QUIET_SHUTDOWN);

	if(verify_client == OK) {
		/*TODO: our ssl session support*/
		SSL_CTX_sess_set_new_cb(ctx, session_new_callback);
		SSL_CTX_sess_set_remove_cb(ctx, session_rem_callback);

		SSL_CTX_set_session_id_context(ctx,(void*)&s_server_session_id_context, 
			sizeof (s_server_session_id_context));
		SSL_CTX_set_timeout(ctx, 300);

		// to share the session, we need to ensure the SSL CTX is using CACHE_CLIENT
		long mode = SSL_CTX_get_session_cache_mode( ctx );
		mode |= SSL_SESS_CACHE_CLIENT /*| SSL_SESS_CACHE_SERVER*/;
		SSL_CTX_set_session_cache_mode( ctx, mode );

   		/* Load the RSA CA certificate into the SSL_CTX structure */
     	if (!SSL_CTX_load_verify_locations(ctx, conf_ctx->ssl_cert_file, NULL)) {
        	ERR_print_errors_fp(stderr);
           	exit(1);
      	}
 
    	/* Set to require peer (client) certificate verification */
        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
 
      	/* Set the verification depth to 1 */
        SSL_CTX_set_verify_depth(ctx, 1);
	}
	
	listener->ssl_ctx = ctx;
	listener->accept_callback = (void*)mod_http_ssl_accept_callback;

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
	client->listener->r = cpo_io_ssl_read; 
	client->listener->w = cpo_io_ssl_write;

	return OK;
}

