/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2009 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Borislav Sapundzhiev  <bsapundjiev@gmail.com>                |
  | parts based on phttpd SAPI by Thies C. Arntzen <thies@thieso.net>    |
  | parts based on 														 |
  |       apache2handler SAPI by Sascha Schumann <sascha@schumann.cx>    |
  +----------------------------------------------------------------------+
*/

#include "php.h"
#include "php_ini.h"
#include "php_globals.h"
#include "SAPI.h"
#include "php_main.h"
#include "php_variables.h"
#include "php_version.h"
#include "TSRM.h"

#include "ext/standard/info.h"
#include "ext/standard/php_smart_str.h"

#ifndef ZTS
#warning CALIPSO module is only useable in thread-safe mode
#endif

#include "calipso.h"
#include "rfc2616.h"
#include "cplib.h"
#include "php_calipso.h"
#include <signal.h>

#include "dllist.h"
#include "chunks.h"

#define SERVER_API_NAME "Calipso PHP Handler"

typedef struct {
    //char *cookie;
    size_t data_avail;
} calipso_globals_struct;

static int ph_globals_id;
#define PHG(v) TSRMG(ph_globals_id, calipso_globals_struct *, v)

static int php_calipso_startup(sapi_module_struct *sapi_module)
{
    printf("\n*** %s ...\n", __FUNCTION__);

    if ( php_module_startup(sapi_module, NULL, 0) == FAILURE) {
        printf("***: ERROR!\n");
        return FAILURE;
    }

    return SUCCESS;
}

static int php_calipso_sapi_ub_write(const char *str, uint str_length TSRMLS_DC)
{
    int n;
    uint sent = 0;

    int clientsock;

    calipso_socket_t *listener;
    calipso_client_t *client;

    calipso_request_t *request = (calipso_request_t *) SG(server_context);
    calipso_reply_t *reply = calipso_request_get_reply( request );
    client   = calipso_reply_get_client( reply );
    listener = calipso_client_get_listener(client);
    clientsock = calipso_client_get_socket(client);

    printf("php_calipso_sapi_ub_write %d\n", str_length);
    //sent = listener->w(clientsock, (const void *) str, str_length);

    if( calipso_reply_print(reply, str, str_length) )
        sent = str_length;

    if (sent  != str_length) {
        printf("php_aborted_connection: %s\n",strerror(errno));
        php_handle_aborted_connection();
    }

    return sent;
}

static int
php_calipso_sapi_header_handler(sapi_header_struct *sapi_header, sapi_header_op_enum op, sapi_headers_struct *sapi_headers TSRMLS_DC)
{
    char *val, *ptr;

    calipso_request_t *request = (calipso_request_t *) SG(server_context);
    calipso_reply_t *reply = calipso_request_get_reply( request );

    switch(op) {

    case SAPI_HEADER_DELETE:
        printf("SAPI_HEADER_DELETE %s\n", sapi_header->header);
        //apr_table_unset(ctx->r->headers_out, sapi_header->header);
        return 0;

    case SAPI_HEADER_DELETE_ALL:
        printf("SAPI_HEADER_DELETE_ALL %s\n", sapi_header->header);
        //apr_table_clear(ctx->r->headers_out);
        return 0;

    case SAPI_HEADER_ADD:
    case SAPI_HEADER_REPLACE:

        val = strchr(sapi_header->header, ':');

        if (!val) {
            sapi_free_header(sapi_header);
            return 0;
        }
        ptr = val;

        *val = '\0';

        do {
            val++;
        } while (*val == ' ');

        if (!strcasecmp(sapi_header->header, "content-type")) {
            calipso_reply_set_header_value(reply, "Content-Type", val);
        } else if (!strcasecmp(sapi_header->header, "content-length")) {
            reply->content_length = atol(val);
            calipso_reply_set_header_value(reply, sapi_header->header, val);
        } else {
            //printf("*** %d header(%s: %s)\n", op, sapi_header->header, val);
            //calipso_reply_set_header_value(reply, sapi_header->header, val);
            //calipso_reply_set_header_value(reply, sapi_header->header, "%s",val);
            char *hdr_val = cpo_pool_strdup(reply->request->pool, val);

            if(SAPI_HEADER_REPLACE == op) {
                hash_table_update(reply->header, sapi_header->header, hdr_val);
            } else {
                hash_table_insert(reply->header, sapi_header->header, hdr_val);
            }
        }
        *ptr = ':';
        return SAPI_HEADER_ADD;

    default:
        return 0;
    }
}


static int php_calipso_sapi_send_headers(sapi_headers_struct *sapi_headers TSRMLS_DC)
{
    calipso_request_t *request = (calipso_request_t *) SG(server_context);
    calipso_reply_t *reply = calipso_request_get_reply( request );

    int http_status = SG(sapi_headers).http_response_code;
    calipso_reply_set_status(reply, http_status);

    if (SG(sapi_headers).send_default_content_type) {
        calipso_reply_set_header_value(reply, "Content-Type", "text/html");
    }

    return SAPI_HEADER_SENT_SUCCESSFULLY;
}

static char *php_calipso_sapi_read_cookies( TSRMLS_D )
{
    calipso_request_t *request = (calipso_request_t *) SG(server_context);
    const char *http_cookie = calipso_request_get_header_value(request, "cookie");

    return (char*)http_cookie;
}

static int
php_calipso_sapi_read_post(char *buf, uint count_bytes TSRMLS_DC)
{
    uint max_read;
    uint total_read = 0;
    int clientsock;

    calipso_socket_t *listener;
    calipso_client_t *client;
    calipso_request_t *request = (calipso_request_t *) SG(server_context);

    client   = calipso_request_get_client(request);
    /*
    	listener = calipso_client_get_listener(client);
    	clientsock = calipso_client_get_socket(client);

    	max_read = MIN( PHG(data_avail), count_bytes );
    	printf("[POST] %d\n", max_read);
    	if(max_read)
    	{
    		printf("max_read %d count_bytes %d\n", max_read, count_bytes);
    		total_read = listener->r(clientsock, buf, max_read);

    		if(total_read == -1) {
    			total_read = -1;
    		} else {
    			PHG(data_avail) -= total_read;
    		}
    	}
    */
    max_read = MIN( request->in_filter->total_bytes, count_bytes );
    total_read = chunks_read_block(request->in_filter, buf, max_read);

    return total_read;
}

/**
 * php_ns_sapi_register_variables() populates the php script environment
 * with a number of variables. HTTP_* variables are created for
 * the HTTP header data, so that a script can access these.
 */

#define ADD_STRINGX(name, buf)\
	php_register_variable(name, buf, track_vars_array TSRMLS_CC)

#define ADD_STRING(name)\
	ADD_STRINGX(name, buf)

static void
php_calipso_sapi_register_variables(zval *track_vars_array TSRMLS_DC)
{
    /*
     *TODO: add all vars
     */
    char tmp[6];

    calipso_request_t *request = (calipso_request_t *) SG(server_context);
    //calipso_reply_t *reply = calipso_request_get_reply( request );
    calipso_client_t *client = calipso_request_get_client( request );
    /* http header */
    char * host = calipso_request_get_header_value(request, "host");
    if(host)
        ADD_STRINGX("HTTP_HOST", host);
    char * content_len = calipso_request_get_header_value(request, "content-length");
    if(content_len)
        ADD_STRINGX("CONTENT_LENGTH",content_len);

    char * user_agent = calipso_request_get_header_value(request, "user-agent");
    if(user_agent)
        ADD_STRINGX("HTTP_USER_AGENT", user_agent);

    char * accept_language = calipso_request_get_header_value(request, "accept-language");
    if(accept_language)
        ADD_STRINGX("HTTP_ACCEPT_LANGUAGE", accept_language);

    char* accept_encoding = calipso_request_get_header_value(request, "accept-encoding");
    if(accept_encoding)
        ADD_STRINGX("HTTP_ACCEPT_ENCODING", accept_encoding);

    char* accept_charset = calipso_request_get_header_value(request, "accept-charset");
    if(accept_charset)
        ADD_STRINGX("HTTP_ACCEPT_CHARSET", accept_charset);

    char *keep_alive = calipso_request_get_header_value(request, "keep-alive");
    if(keep_alive)
        ADD_STRINGX("HTTP_KEEP_ALIVE", keep_alive);

    char *connection = calipso_request_get_header_value(request, "connection");
    if(connection)
        ADD_STRINGX("HTTP_CONNECTION", connection);

    char* referer = calipso_request_get_header_value(request, "referer");
    if(referer)
        ADD_STRINGX("HTTP_REFERER", referer);
    if(request->host)
        ADD_STRINGX("SERVER_NAME",	request->host);
    ADD_STRINGX("SERVER_ADDR",	client->server->hostname);
    sprintf(tmp, "%d", client->listener->port);
    ADD_STRINGX("SERVER_PORT",	tmp);

    ADD_STRINGX("SERVER_SOFTWARE", calipso_get_server_string(NULL));

    ADD_STRINGX("SERVER_PROTOCOL", request->version);

    ADD_STRINGX("REQUEST_METHOD", request->method);

    if(request->querystring) {
        ADD_STRINGX("QUERY_STRING", request->querystring);
    }

    ADD_STRINGX("REMOTE_ADDR", inet_ntoa( client->info->sin_addr) );

    sprintf(tmp, "%d", htons( client->info->sin_port ) );
    ADD_STRINGX("REMOTE_PORT", tmp);

    ADD_STRINGX("PATH_TRANSLATED", SG(request_info).path_translated);
    ADD_STRINGX("SCRIPT_FILENAME", SG(request_info).path_translated);
    ADD_STRINGX("SCRIPT_NAME", SG(request_info).request_uri);
    ADD_STRINGX("REQUEST_URI", SG(request_info).request_uri);
    ADD_STRINGX("PHP_SELF", SG(request_info).request_uri);
    ADD_STRINGX("GATEWAY_INTERFACE", "CGI/1.1");

}
static void php_calipso_sapi_flush(void *server_context)
{
    printf("\n*** FLUSH ***\n");
    calipso_request_t *request;
    TSRMLS_FETCH();
    request = server_context;

    if(!request) {
        printf("!server_context\n");
        return;
    }

    sapi_send_headers(TSRMLS_C);
    request->reply->status = SG(sapi_headers).http_response_code;
    SG(headers_sent) = 1;
    //server_context = NULL;
}

static void php_calipso_sapi_log_message(char *msg TSRMLS_DC)
{
    //ctx = SG(server_context);
    cpo_log_error(calipso->log, "%s", msg);
}

static double php_calipso_sapi_get_request_time(TSRMLS_D)
{
    calipso_request_t *r = SG(server_context);
    return ((double) r->request_time) / 1000.0;
}

static sapi_module_struct calipso_sapi_module = {
    "calipso",
    SERVER_API_NAME,
    php_calipso_startup,            /* startup */
    php_module_shutdown_wrapper,    /* shutdown */

    NULL,							/* activate */
    NULL,							/* deactivate */

    php_calipso_sapi_ub_write,     	/* unbuffered write */
    php_calipso_sapi_flush,			/* flush */
    NULL,							/* get uid */
    NULL,							/* getenv */

    php_error,                      /* error handler */

    php_calipso_sapi_header_handler,/* header handler */
    php_calipso_sapi_send_headers,  /* send headers handler */
    NULL,                           /* send header handler */

    php_calipso_sapi_read_post,    	/* read POST data */
    php_calipso_sapi_read_cookies,	/* read Cookies */

    php_calipso_sapi_register_variables,/* register server variables */
    php_calipso_sapi_log_message,		/* Log message */
    php_calipso_sapi_get_request_time,	/* Get request time */

    STANDARD_SAPI_MODULE_PROPERTIES
};

static void php_calipso_request_ctor(calipso_request_t *request TSRMLS_DC )
{
    //#define safe_strdup(x) ((x)?strdup((x)):NULL)
    //#define safe_strdup(x) cpo_pool_strdup(request->reply->pool, x);
#define safe_strdup(x) cpo_pool_strdup(request->pool, x);

    const char* auth = NULL;
    //calipso_request_t *request = (calipso_request_t *) SG(server_context);
    calipso_reply_t *reply = calipso_request_get_reply(request);
    calipso_resource_t *resource = calipso_reply_get_resource(reply);

    const char * filename = calipso_resource_get_path(resource);

    SG(sapi_headers).http_response_code = 200;

    SG(request_info).request_method = request->method;

    SG(request_info).query_string 	= safe_strdup( request->querystring );

    SG(request_info).path_translated = safe_strdup( filename );

    if( strlen( strrchr(request->uri,'/') ) == 1) {
        strcat(request->uri, "index.php");
    }

    SG(request_info).request_uri	 = safe_strdup( request->uri );

    const char * content_length = calipso_request_get_header_value(request, "content-length");
    SG(request_info).content_length = (content_length ? atol(content_length) : 0);

    const char * content_type = calipso_request_get_header_value(request, "content-type");
    if(content_type) {
        SG(request_info).content_type	= safe_strdup( content_type );
    }

    //PHG( data_avail ) = SG(request_info).content_length;

    auth = calipso_request_get_header_value(request, "Authorization");
    php_handle_auth_data(auth TSRMLS_CC);

    if(SG(request_info).auth_user == NULL && request->user) {
        SG(request_info).auth_user = safe_strdup(request->user);
    }

    if(SG(request_info).auth_user && !request->user) {
        request->user = safe_strdup(SG(request_info).auth_user);
    }

}

static void php_calipso_request_dtor(calipso_request_t *request TSRMLS_DC )
{
    php_request_shutdown(NULL);
}


static int php_calipso_execute( TSRMLS_D )
{

    zend_first_try {
        zend_file_handle file_handle;

        if (php_request_startup( TSRMLS_C ) == FAILURE)
        {
            return FAILURE;
        }

        file_handle.type 	  		= ZEND_HANDLE_FILENAME;
        file_handle.filename 	  	= SG(request_info).path_translated;
        file_handle.opened_path   	= NULL;
        file_handle.free_filename 	= 0;

        php_execute_script(&file_handle TSRMLS_CC);
        //zend_execute_scripts(ZEND_INCLUDE TSRMLS_CC, NULL, 1, &file_handle);

    } zend_end_try();

    return SUCCESS;
}

/* external */
PHPAPI int pm_init()
{
    /* use config */
    char * calipso_php_ini_path = "/etc/calipso";
    if (calipso_php_ini_path) {
        calipso_sapi_module.php_ini_path_override = calipso_php_ini_path;
    }

#ifdef ZTS
    tsrm_startup(1, 1, 0, NULL);
    ts_allocate_id(&ph_globals_id, sizeof(calipso_globals_struct), NULL, NULL);
#endif
    sapi_startup( &calipso_sapi_module );
    calipso_sapi_module.startup(&calipso_sapi_module);

    /* register hooks */
    mod_php5_register_hooks();

    return 1;
}

PHPAPI void pm_exit()
{
    printf("\n*** pm_exit ***\n");
    sapi_module.shutdown(&calipso_sapi_module);
    sapi_shutdown();
#ifdef ZTS
    tsrm_shutdown();
#endif
}

/* execute handler */
int php_handler(calipso_request_t *request)
{
    int status = 0;
    calipso_request_t *req_prev;

    printf("php_handler()\n");

    TSRMLS_FETCH();
    req_prev = SG(server_context);

    if(req_prev) {
        //	printf("req_prev %s == %s\n",  SG(request_info).path_translated, request->uri);
        request->client->keepalive = 0;
    }

    SG(server_context) = request;

    php_calipso_request_ctor(request TSRMLS_CC );

    status = php_calipso_execute( TSRMLS_C );

    //printf("php mem_usage %u\n", zend_memory_peak_usage(1 TSRMLS_CC));

    php_calipso_request_dtor(request TSRMLS_CC );

    printf("end php_handler()\n");

    return status;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
