#include "calipso.h"
#include "http_auth.h"

static int http_auth_access_check(calipso_request_t * request);
static int http_auth_header(calipso_request_t * request);

int http_auth_basic_init() 
{
	calipso_register_hook(HOOK_ACCESS_CHECK, (void *)http_auth_access_check);
	return CPO_OK;
}

static int http_auth_header(calipso_request_t * request)
{
	const char *realm = "insert realm";

	calipso_reply_set_header_value(request->reply, "WWW-Authenticate", "Basic realm=\"%s\"", realm);
	return calipso_reply_set_status(request->reply, HTTP_UNAUTHORIZED);
}

int http_auth_access_check(calipso_request_t * request)
{
	http_auth_header(request);
	//Authorization: Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ==
	return CPO_OK;
}

