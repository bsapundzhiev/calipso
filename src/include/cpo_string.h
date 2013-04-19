#ifndef _CPO_STRING_H
#define _CPO_STRING_H

size_t 
cpo_strlen(const char * src);

int 
cpo_strnicmp( const char *_src1, const char *_src2, size_t _num );

char *
cpo_strtok(char *str, const char *delims);

int 
cpo_atoi(const char *s);

char *
cpo_itoa(char *buf, int i);

size_t
strlcpy(char *dest, const char *src, size_t len);

size_t
strlcat(char *dest, const char *src, size_t len);

int 
cpo_snprintf(char *buf, int len, char *fmt, ...);

int cpo_vslprintf(char *buf, int buflen, char *fmt, va_list args);

#endif
