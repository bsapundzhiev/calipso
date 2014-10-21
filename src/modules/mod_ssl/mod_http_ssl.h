#ifndef _MOD_HTTP_SSL_H
#define _MOD_HTTP_SSL_H

#include "module.h"

CPMOD_API int pm_init();
CPMOD_API int pm_exit();
CPMOD_API inline void initCalipso(calipso_t *c)
{
    if(calipso == NULL) {
        printf("calipso is NULL\n");
        calipso = c;
    }
}

#endif
