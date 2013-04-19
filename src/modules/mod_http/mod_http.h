#ifndef _MOD_HTTP_H
#define _MOD_HTTP_H

#include "module.h"

CPMOD_API int pm_init();
CPMOD_API int pm_exit();
CPMOD_INITIALIZER();

const char *
mime_get_type_value(hash_t *h, char const *filename);

#endif

