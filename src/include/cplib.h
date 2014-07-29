#ifndef _CPL_H
#define _CPL_H

#include <stdio.h>
#ifdef _WIN32
#include "compat.h"
#endif
/*calipso portable lib */
#define F_BIN 	0x0
#define F_ASCII	0x1

void cpo_uri_strdecode( char* to , char* from );

int cpo_uri_sanity_check(const char *str );

int cpo_uri_normalize_remove_dots(char* path);

int remove_dots_from_uri_path(char* path);

char *ltrim(char *s);
char *rtrim(char *s);
char *trim(char *s);

char *prepend(char* s, const char* t);

int hex2ascii(unsigned long val, char* buf, unsigned short len);
void hex2string(unsigned long val, char *buf, unsigned short len);
int cpo_explode(char ***arr_ptr, char *str, char delimiter);

/*files*/
int fchk(int fd);
int is_file_of(const char * file, const char *ext);

#endif
