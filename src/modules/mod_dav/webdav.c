#include <string.h>
#include "calipso.h"
#include "chunks.h"

#include "webdav.h"
#include "dav_fs.h"

struct dav_status {
    int code;
    char *message;
};

static struct dav_status dav_status_codes[] = {
    {200, "OK"},
    {201, "Created"},
    {204, "No Content"},
    {207, "Multi-Status"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {409, "Conflict"},
    {415, "Unsupported Media Type"},
    {500, "Internal Server Error"},
    {501, "Method not implemented"},
    { 0, NULL },
};

//"OPTIONS","GET", "HEAD", "POST", "DELETE","TRACE","PROPFIND","PROPPATCH","COPY","MKCOL","PUT"
static struct dav_status dav_str_methods[] = {
    {PROPFIND, "PROPFIND"},
    {   MKCOL, "MKCOL"},
    {  DELETE, "DELETE"},
    {     PUT, "PUT"},
    {    COPY, "COPY"},
    {    MOVE, "MOVE"},
    {    LOCK, "LOCK"},
    {  UNLOCK, "UNLOCK"},
    { OPTIONS, "OPTIONS"},
    { 0, NULL },
};


static int webdav_options(calipso_request_t *request);
static int webdav_propfind(calipso_request_t *request);

static int webdav_get_method(const char *method)
{
    int i;
    for (i =0; i < ARRAYSZ(dav_str_methods) -1; i++ ) {
        if(!strcasecmp(dav_str_methods[i].message, method)) {
            return dav_str_methods[i].code;
        }
    }

    return -1;
}


int webdav_invoke(calipso_request_t *request)
{
    char *method = calipso_request_get_method(request);
    int dav_method = webdav_get_method(method);
    int ret =0;
    if (dav_method < 0)
        return 0;

    switch(dav_method) {
    case OPTIONS:
        ret = webdav_options(request);
        break;
    case PROPFIND:
        ret = webdav_propfind(request);
        break;

    default:
        return 0;
    }

    return ret;
}

int webdav_options(calipso_request_t *request)
{
    calipso_reply_t *reply = calipso_request_get_reply(request);
    //DAV class 1 for now
    calipso_reply_set_header_value(reply,"Allows", "OPTIONS, GET, HEAD, POST, DELETE, TRACE, PROPFIND, PROPPATCH, COPY, MKCOL, PUT");
    calipso_reply_set_header_value(reply, HEADER_DAV, HEADER_DAV_1);
    return 1;
}

static int webdav_propfind(calipso_request_t *request)
{
    int max_read;
    char buf[1024];
    TRACE("request->in_filter->total_bytes %d\n", request->in_filter->total_bytes);
    /*while(request->in_filter->total_bytes > 0) {
    	max_read = MIN( request->in_filter->total_bytes, sizeof(buf) );
     	chunks_read_block(request->in_filter, buf , max_read);
     	printf(">>>buf %s\n", buf);
    }*/
    max_read = chunks_read_block(request->in_filter, buf, sizeof(buf));
    printf(">>>buf %s\n", buf);
    return 0;
}
