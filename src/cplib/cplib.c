/* cplib.c - utils
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
#include <ctype.h>
#include <stdarg.h>
#ifdef _WIN32
#include "compat.h"
#include <io.h>
#else
#include <unistd.h>
#endif
#include "cplib.h"

#define HEXVALUE(c) \
  (((c) >= 'a' && (c) <= 'f') \
  	? (c)-'a'+10 \
  	: (c) >= 'A' && (c) <= 'F' ? (c)-'A'+10 : (c)-'0')

#ifdef isxdigit
#undef isxdigit
#define isxdigit(c) ( isdigit((c)) || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F') )
#endif

#define IS_INVALID_PATH(path)\
	(!path || !*path || *path != '/')\
 
/* http related */

/*
 * Copies and decodes a string.
 * It's ok for from and to to be the same string.
 */
void cpo_uri_strdecode(char* to, char* from)
{
    for (; *from != '\0'; ++to, ++from) {
        if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2])) {
            *to = HEXVALUE( from[1] ) * 16 + HEXVALUE(from[2]);
            from += 2;
        } else
            *to = *from;
    }

    *to = '\0';
}

/* normalize uri remove dots
 * returns true on succcess on error false */
int cpo_uri_normalize_remove_dots(char* path)
{
    char prev = 0;
    char *p = path;

    if (IS_INVALID_PATH(path))
        return 0;

    while (*path != '\0') {

        if (*path != '.') {

            if (prev == '.' && *path == '/') {
                prev = *path;
                path++;
                continue;
            } else {
                *p = *path;
                p++;
            }
        } else if (*path == '.' && prev != '.' && prev != '/'
                   && *(path + 1) != '.' && *(path + 1) != '/') {

            *p = *path;
            p++;
        }

        prev = *path;
        path++;
    }

    *p = '\0';
    return 1;
}

//nxweb
int remove_dots_from_uri_path(char* path)
{
    if (!path || !*path)
        return 0; // end of path
    if (*path != '/')
        return -1; // invalid path
    while (1) {
        if (path[1] == '.' && path[2] == '.'
                && (path[3] == '/' || path[3] == '\0')) { // /..(/.*)?$
            memmove(path, path + 3, strlen(path + 3) + 1);
            return 1;
        } else {
            char* p1 = strchr(path + 1, '/');
            if (!p1)
                return 0;
            if (!remove_dots_from_uri_path(p1))
                return 0;
            memmove(path, p1, strlen(p1) + 1);
        }
    }
}


int cpo_uri_sanity_check(const char *path)
{
    unsigned i;

    if (IS_INVALID_PATH(path))
        return 0;

    for (i = 0; path[i] != '\0'; i++) {

        if (path[i] == '.' && path[i + 1] == '.' && path[i - 1] == '/') {
            return 0;
        }
    }

    return 1;
}

char *trim(char *s)
{
    char *cp1, *cp2;

    for (cp1 = s; isspace(*cp1); cp1++)
        ;
    for (cp2 = s; *cp1; cp1++, cp2++)
        *cp2 = *cp1;
    *cp2-- = 0;

    while (cp2 > s && isspace(*cp2))
        *cp2-- = 0;

    return s;
}
char *prepend(char* s, const char* t)
{
    size_t len = strlen(t);

    size_t i;

    memmove(s + len, s, len + 1);

    for (i = 0; i < len; ++i) {
        s[i] = t[i];
    }
    return s;
}

int hex2ascii(unsigned long val, char* buf, unsigned short len)
{
    unsigned char n = 0;
    int index;

    for (index = 0; index < len; index++) {
        n = val & 0x0000000f;
        //printf("nib=%d index %d\n",n, index);
        if (n <= 9)
            n = '0' + n;
        else
            n = 'A' + (n - 10);
        buf[len - index - 1] = n;
        val = val >> 4;
    }

    buf[index] = '\0';
    return index;
}

/* hex2ascii slow variant */
void hex2string(unsigned long val, char *buf, unsigned short len)
{
    snprintf(buf, len, "%lX", val);
    buf[len] = '\0';
}

int cpo_explode(char ***arr_ptr, char *str, char delimiter)
{
    char *src = str, *end, *dst;
    char **arr;
    int size = 1, i;
    if(!str) return 0;
    // Find number of strings
    while ((end = strchr(src, delimiter)) != NULL) {
        ++size;
        src = end + 1;
    }

    arr = malloc(size * sizeof(char *) + (strlen(str) + 1) * sizeof(char));

    src = str;
    dst = (char *) arr + size * sizeof(char *);
    for (i = 0; i < size; ++i) {
        if ((end = strchr(src, delimiter)) == NULL)
            end = src + strlen(src);
        arr[i] = dst;
        strncpy(dst, src, end - src);
        dst[end - src] = '\0';
        dst += end - src + 1;
        src = end + 1;
    }
    *arr_ptr = arr;

    return size;
}

/**
 *\ consider_file_type
 */
int fchk(int fd)
{
    unsigned char chr;
    int n = 0;

    while (n != EOF) {
        n = read(fd, (char*) &chr, 1);
        //printf("[%d]read: 0x%x - %c - %d n: %d\n",i,chr,chr,(int)chr,n);
        /*!byte order*/
        if ( isalpha(chr) || ispunct(chr) || isspace(chr) || isdigit(chr)) {
            if (n <= 0)
                break;
            continue;
        } else
            /*binary file*/
            return (F_BIN);
    }
    /*F_ASCII*/
    return (F_ASCII);
}

int is_file_of(const char * file, const char *ext)
{
    char *trim = NULL;
    char *ext1 = (char*) ext;

    if(!file || !ext)
        return 0;

    trim = strrchr(file, '.');
    if (trim == NULL)
        return 0;
    while (*trim++ && *ext1++) {
        if (tolower((int) *trim) != tolower((int) *ext1)) {
            //printf("0 : %c cmp %c\n", *trim, *ext1);
            return 0;
        }
        //else
        //	printf("1 : %c cmp %c\n", *trim, *ext1);
    }

    return 1;
}

#ifdef CPL_TEST
int main ()
{
    int i = 1234567890;

    char *key;		//[6];

    cpl_itoa(key, i);

    printf("---integer: %d\n",i);
    printf("------itoa: %s\n", key );

    char buffer[9];
    unsigned long n=8000;
    //for(n = 0; n < 80000; n++)
    {
        hex2ascii(n, buffer, sizeof(int));
        printf("%s\n",buffer);
        hex2string(n, buffer , sizeof(int));
        printf("%s\n",buffer);
    }
    return 0;
}
#endif
