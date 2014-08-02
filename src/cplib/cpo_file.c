/* cpo_file file  */

#include <fcntl.h>
#include "cpo_file.h"

int cpo_file_open(const char *filename, int flags) {

#ifdef _WIN32
	int fd = open(filename, O_RDONLY, _O_RANDOM | _O_SEQUENTIAL);
	_setmode( fd, _O_BINARY);
#else
	int fd = open(filename, O_RDONLY | O_NONBLOCK | flags, 0644);
#endif

	return fd;
}
