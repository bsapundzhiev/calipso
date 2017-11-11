#include "calipso.h"
#include "http_auth.h"
extern long base64_encode(char *to, char *from, unsigned int len);
extern long base64_decode(char *to, char *from, unsigned int len);

static int http_auth_access_check(calipso_request_t * request);
static int http_auth_header(calipso_request_t * request);

static int http_auth_decode_basic(calipso_request_t * request, const char *hdr);

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

    if(!request->user && !strncmp(request->uri, "/test/priv", strlen("/test/priv") )) {

        char *authhdr = calipso_request_get_header_value(request, "Authorization");
        printf("authhdr '%s'\n", authhdr);
        if(!authhdr) {
            http_auth_header(request);
        } else {
            char *type = cpo_strtok(authhdr, " ");
            if(!strcasecmp(type, "basic")) {
                char *tok = cpo_strtok(NULL," ");
                http_auth_decode_basic(request, tok);
            } else {
                return calipso_reply_set_status(request->reply, HTTP_UNAUTHORIZED);
            }
        }

    }

    return CPO_OK;
}

static int http_auth_decode_basic(calipso_request_t * request, const char *hdr)
{
    //Authorization: Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ==
    char decstr[1024];
    base64_decode(decstr, (char*)hdr, 1024);
    printf("decstr %s\n", decstr);
    return 0;
}
