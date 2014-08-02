/*
 *   Copyright (C) 2007 by Borislav Sapundjiev <BSapundjiev_AT_gmail[D0T]com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 */

#include <sys/stat.h>
#include <sys/types.h>

#include "calipso.h"

calipso_resource_t *
calipso_resource_alloc(void)
{
    calipso_resource_t *resource = malloc(sizeof(calipso_resource_t));

    if (resource != NULL) {
        resource->resource_fd = -1;
        resource->resource_path = NULL;
        resource->pool = cpo_pool_create(CALIPSO_DEFAULT_POOLSIZE);
        resource->resource_stat = NULL;
        resource->offset = 0x0;
    }

    return (resource);
}
/*
int calipso_resource_init(calipso_resource_t *resource, calipso_pool_t *pool)
{
    return 0;
}
*/
void calipso_resource_unalloc(calipso_resource_t *resource)
{
    if (resource->resource_fd > 0)
        close(resource->resource_fd);

    cpo_pool_destroy(resource->pool);
    free(resource);
    return;
}

int calipso_resource_set_path(calipso_resource_t *resource, char *path)
{
    return ((resource->resource_path = path) != NULL);
}

char *
calipso_resource_get_path(calipso_resource_t *resource)
{
    return (resource->resource_path);
}

int calipso_resource_set_file_descriptor(calipso_resource_t *resource, int fd)
{
    return ((resource->resource_fd = fd));
}

int calipso_resource_get_file_descriptor(calipso_resource_t *resource)
{
    return (resource->resource_fd);
}

int calipso_resource_set_stat(calipso_resource_t *resource, void *sb)
{

    if (resource->resource_stat == NULL) {
        resource->resource_stat = cpo_pool_malloc(resource->pool,
                                  sizeof(struct stat));
    }

    *(struct stat *) resource->resource_stat = *(struct stat *) sb;
    return (1);
}

void *
calipso_resource_get_stat(calipso_resource_t *resource)
{
    return (resource->resource_stat);
}

int calipso_resource_is_set(calipso_resource_t *resource)
{
    return (resource->resource_stat != NULL);
}

int calipso_resource_is_file(calipso_resource_t *resource)
{
    if (resource->resource_stat == NULL)
        return (0);

    if (!S_ISREG(((struct stat * )resource->resource_stat)->st_mode))
        return (0);

    return (1);
}

int calipso_resource_is_directory(calipso_resource_t *resource)
{
    if (resource->resource_stat == NULL)
        return (0);

    if (!S_ISDIR(((struct stat * )resource->resource_stat)->st_mode))
        return (0);

    return (1);
}

uintmax_t calipso_resource_get_size(calipso_resource_t *resource)
{
    if (resource->resource_stat == NULL)
        return (0);

    return (((struct stat *) resource->resource_stat)->st_size);
}

