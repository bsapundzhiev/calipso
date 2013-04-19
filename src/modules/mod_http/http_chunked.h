#ifndef _HTTP_CHUNKED_H
#define _HTTP_CHUNKED_H

int http_chunked_filter_header(calipso_reply_t* reply);
int http_chunked_filter_body(calipso_reply_t* reply);

#endif
