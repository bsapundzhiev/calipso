/* 
 *
 * Copyright (C) 2007 Borislav Sapundzhiev
 *         
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xmalloc.h"

void* xmalloc(unsigned size) {
	void *ret;

	ret = malloc(size);
	if (ret != NULL)
		return ret;

	printf("%s,%s:Virtual memory exhausted", __FILE__, __FUNCTION__);

	return 0; /*NOTREACHED*/
}

void* xzmalloc(unsigned size) {
	void *ret;

	ret = malloc(size);
	if (ret != NULL) {
		memset(ret, 0, size);
		return ret;
	}

	printf("%s,%s:Virtual memory exhausted", __FILE__, __FUNCTION__);
	return 0; /*NOTREACHED*/
}

void *xrealloc(void* ptr, unsigned size) {
	void *ret;

	/* xrealloc (NULL, size) behaves like xmalloc (size), as in ANSI C */
	ret = (!ptr ? malloc(size) : realloc(ptr, size));
	if (ret != NULL)
		return ret;

	printf("%s,%s:Virtual memory exhausted", __FILE__, __FUNCTION__);
	return 0; /*NOTREACHED*/
}

char *xstrdup(const char* str) {
	char *p = xmalloc(strlen(str) + 1);
	strcpy(p, str);
	return p;
}

char *xstrndup(const char* str, unsigned len) {
	char *p = xmalloc(len + 1);
	strncpy(p, str, len);
	p[len] = '\0';
	return p;
}

