#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "cpo_string.h"

#define isdigit(c) 	((c) >= '0' && (c) <= '9')
#define tolower(c)     	((c>='A' && c<='Z') ? (c+('a'-'A')) : c)
#define toupper(c)     	((c>='a' && c<='z') ? (c-('a'-'A')) : c)

size_t cpo_strlen(const char * src) {
	const char * src2 = src;
	while (*src2 != '\0')
		++src2;
	return (src2 - src);
}

/* dloop pattern -*/
size_t new_strlen(const char * src) {
	unsigned int i = 0;
	while ((unsigned char) src[i] != '\0') {
		if ((unsigned char) src[i + 1] == '\0') {
			i++;
			break;
		}
		i += 2;
		while (((unsigned char) src[i] & (unsigned char) src[i + 1]) != 0)
			i += 2;
	}
	return i;
}

/*
 * string token
 */
char * cpo_strtok(char *str, const char *delims) {
	static char *pos = (char *) 0;
	char *start = (char *) 0;

	if (str) /* Start a new string? */
		pos = str;

	if (pos) {
		/* Skip delimiters */
		while (*pos && strchr(delims, *pos))
			pos++;
		if (*pos) {
			start = pos;
			/* Skip non-delimiters */
			while (*pos && !strchr(delims, *pos))
				pos++;
			if (*pos)
				*pos++ = '\0';
		}
	}

	return start;
}

/*
 * strnicmp routines -- mising on some distrib 
 */
int cpo_strnicmp(const char *_src1, const char *_src2, size_t _num) {

	if (_num == 0)
		return 0;

	for (; *_src1 && _num; _num--) {
		if (tolower(*_src1) != tolower(*_src2++))
			return (tolower(*_src1) - tolower(*--_src2));
	}/* for ( ; *_src1 && _num; _num-- ) */

	return 0;
}

/* 
 * itoa() is not a standard function, and we cannot safely call printf()
 * after suspending threads. So, we just implement our own copy. A
 * recursive approach is the easiest here.
 */
char *cpo_itoa(char *buf, int i) {

	if (i < 0) {
		*buf++ = '-';
		return cpo_itoa(buf, -i);
	} else {
		if (i >= 10)
			buf = cpo_itoa(buf, i / 10);
		*buf++ = (i % 10) + '0';
		*buf = '\000';
		return buf;
	}
}

/*
 * Local substitute for the atoi() function, which is not necessarily safe
 * to call once threads are suspended (depending on whether libc looks up
 * locale information,  when executing atoi()).
 */
int cpo_atoi(const char *s) {

	int n = 0;
	int neg = *s == '-';
	if (neg)
		s++;
	while (*s >= '0' && *s <= '9')
		n = 10 * n + (*s++ - '0');
	return neg ? -n : n;
}

/*
 * strlcpy - like strcpy/strncpy, doesn't overflow destination buffer,
 * always leaves destination null-terminated (for len > 0).
 */
size_t cpo_strlcpy(char *dest, const char *src, size_t len) {
	size_t ret = strlen(src);

	if (len != 0) {
		if (ret < len)
			strcpy(dest, src);
		else {
			strncpy(dest, src, len - 1);
			dest[len - 1] = 0;
		}
	}
	return ret;
}

/*
 * strlcat - like strcat/strncat, doesn't overflow destination buffer,
 * always leaves destination null-terminated (for len > 0).
 */
size_t cpo_strlcat(char *dest, const char *src, size_t len) {
	size_t dlen = strlen(dest);

	return dlen + cpo_strlcpy(dest + dlen, src, (len > dlen ? len - dlen : 0));
}

//#define OUTCHAR(c)	(buflen > 0? (--buflen, *buf++ = (c)): 0)
#define OUTCHAR(c)	(*buf++ = (c))

int cpo_vslprintf(char *buf, int buflen, char *fmt, va_list args) {
	int c, i, n;
	int width, prec, fillch;
	int base, len, neg, quoted;
	unsigned long long val = 0;
	char *str, *f, *buf0;
	unsigned char *p;
	char num[32];
	time_t t;
	//u_int32_t ip;
	static char hexchars[] = "0123456789abcdef";

	buf0 = buf;
	--buflen;
	while (buflen > 0 || (*fmt /*&& buf */)) {
		for (f = fmt; *f != '%' && *f != 0; ++f)
			;
		if (f > fmt) {
			len = f - fmt;
			if (len > buflen)
				len = buflen;
			memcpy(buf, fmt, len);
			buf += len;
			buflen -= len;
			fmt = f;
		}
		if (*fmt == 0)
			break;
		c = *++fmt;
		width = 0;
		prec = -1;
		fillch = ' ';
		if (c == '0') {
			fillch = '0';
			c = *++fmt;
		}
		if (c == '*') {
			width = va_arg(args, int);
			c = *++fmt;
		} else {
			while (isdigit(c)) {
				width = width * 10 + c - '0';
				c = *++fmt;
			}
		}
		if (c == '.') {
			c = *++fmt;
			if (c == '*') {
				prec = va_arg(args, int);
				c = *++fmt;
			} else {
				prec = 0;
				while (isdigit(c)) {
					prec = prec * 10 + c - '0';
					c = *++fmt;
				}
			}
		}
		str = 0;
		base = 0;
		neg = 0;
		++fmt;
		switch (c) {
		case 'l':
			c = *fmt++;
			switch (c) {
			case 'd':
				val = va_arg(args, long);
				if ((long) val < 0) {
					neg = 1;
					val = (unsigned long) (-(long) val);
				}
				base = 10;
				break;
			case 'u':
				val = va_arg(args, unsigned long);
				base = 10;
				break;
			default:
				//printf("XX %c  cc %c\n" , c , *fmt++ );

				if (c == 'z' || (c == 'l' && *fmt++ == 'u')) {
					val = va_arg(args, unsigned long long);
					base = 10;
					break;
				}

				//*fmt--;
				*buf++ = '%';
				--buflen;
				*buf++ = 'l';
				--buflen;
				--fmt; /* so %lz outputs %lz etc. */
				continue;
			}
			break;
		case 'd':
			i = va_arg(args, int);
			if (i < 0) {
				neg = 1;
				val = -i;
			} else
				val = i;
			base = 10;
			break;
		case 'u':
			val = va_arg(args, unsigned int);
			base = 10;
			break;
		case 'o':
			val = va_arg(args, unsigned int);
			base = 8;
			break;
		case 'x':
		case 'X':
			val = va_arg(args, unsigned int);
			base = 16;
			break;
		case 'p':
			val = (unsigned long) va_arg(args, void *);
			base = 16;
			neg = 2;
			break;
		case 's':
			str = va_arg(args, char *);
			break;
		case 'c':
			num[0] = va_arg(args, int);
			num[1] = 0;
			str = num;
			break;
		case 'm':
			str = strerror(errno);
			break;
			/*case 'I':
			 ip = va_arg(args, u_int32_t);
			 ip = ntohl(ip);
			 slprintf(num, sizeof(num), "%d.%d.%d.%d", (ip >> 24) & 0xff,
			 (ip >> 16) & 0xff, (ip >> 8) & 0xff, ip & 0xff);
			 str = num;
			 break;*/

		case 't':
			time(&t);
			str = ctime(&t);
			str += 4; /* chop off the day name */
			str[15] = 0; /* chop off year and newline */
			break;
		case 'v': /* "visible" string */
		case 'q': /* quoted string */
			quoted = c == 'q';
			p = va_arg(args, unsigned char *);
			if (fillch == '0' && prec >= 0) {
				n = prec;
			} else {
				n = strlen((char *) p);
				if (prec >= 0 && n > prec)
					n = prec;
			}
			while (n > 0 && buflen > 0) {
				c = *p++;
				--n;
				if (!quoted && c >= 0x80) {
					OUTCHAR('M');
					OUTCHAR('-');
					c -= 0x80;
				}
				if (quoted && (c == '"' || c == '\\'))
					OUTCHAR('\\');
				if (c < 0x20 || (0x7f <= c && c < 0xa0)) {
					if (quoted) {
						OUTCHAR('\\');
						switch (c) {
						case '\t':
							OUTCHAR('t');
							break;
						case '\n':
							OUTCHAR('n');
							break;
						case '\b':
							OUTCHAR('b');
							break;
						case '\f':
							OUTCHAR('f');
							break;
						default:
							OUTCHAR('x');
							OUTCHAR(hexchars[c >> 4]);
							OUTCHAR(hexchars[c & 0xf]);
						}
					} else {
						if (c == '\t')
							OUTCHAR(c);
						else {
							OUTCHAR('^');
							OUTCHAR(c ^ 0x40);
						}
					}
				} else
					OUTCHAR(c);
			}
			continue;

		case 'B':
			p = va_arg(args, unsigned char *);
			for (n = prec; n > 0; --n) {
				c = *p++;
				if (fillch == ' ')
					OUTCHAR(' ');
				OUTCHAR(hexchars[(c >> 4) & 0xf]);
				OUTCHAR(hexchars[c & 0xf]);
			}
			continue;
		default:
			*buf++ = '%';
			if (c != '%')
				--fmt; /* so %z outputs %z etc. */
			--buflen;
			continue;
		}
		if (base != 0) {
			str = num + sizeof(num);
			*--str = 0;
			while (str > num + neg) {
				*--str = hexchars[val % base];
				val = val / base;
				if (--prec <= 0 && val == 0)
					break;
			}
			switch (neg) {
			case 1:
				*--str = '-';
				break;
			case 2:
				*--str = 'x';
				*--str = '0';
				break;
			}
			len = num + sizeof(num) - 1 - str;
		} else {
			len = strlen(str);
			if (prec >= 0 && len > prec)
				len = prec;
		}
		if (width > 0) {
			if (width > buflen)
				width = buflen;
			if ((n = width - len) > 0) {
				buflen -= n;
				for (; n > 0; --n)
					*buf++ = fillch;
			}
		}
		if (len > buflen)
			len = buflen;

		memcpy(buf, str, len);
		buf += len;
		buflen -= len;
	}
	*buf = 0;
	return buf - buf0;
}

int cpo_sprintf(char *buf, char *fmt, ...) {
	va_list args;
	int ret;
	va_start(args, fmt);
	ret = cpo_vslprintf(buf, -1, fmt, args);
	va_end(args);

	return ret;
}

int cpo_snprintf(char *buf, int len, char *fmt, ...) {
	va_list args;
	int ret;
	va_start(args, fmt);
	ret = cpo_vslprintf(buf, len, fmt, args);
	va_end(args);

	return ret;
}

#ifdef _TEST
int main()
{
	char test[255];
	int i;

	//for(i =0; i < 100000; i++) 
	{
		cpo_snprintf(test, sizeof(test) , "xxx %s %20.llu\n", "Hui", 1000000000111);
		//snprintf(test, sizeof(test) , "xxx %s %10.lu\n", "Hui", 1000);
		//cpo_sprintf(test , "xxx %s %10.lu\n", "Hui", 1000);
		//sprintf(test , "xxx %s %10.lu\n", "Hui", 1000);
	}
	printf("result %s\n", test);

	return 0;
}
#endif

