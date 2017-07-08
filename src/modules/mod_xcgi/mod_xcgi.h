#ifndef _MOD_XCGI_H
#define _MOD_XCGI_H

#include "module.h"

CPMOD_API int pm_init();
CPMOD_API int pm_exit();
CPMOD_INITIALIZER();

int mod_xcgi_parse_cgi_header(calipso_request_t *request, const char *hdr_line);
int mod_xcgi_readline(int fd, char *buf, int len);

#endif
