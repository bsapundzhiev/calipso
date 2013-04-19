#ifndef _CPO_FILE_H
#define _CPO_FILE_H

#ifdef _WIN32
#define DEVNULL	"NUL"
#else
#define DEVNULL	"/dev/null"
#endif

int cpo_file_open(const char *filename, int flags);

#endif
