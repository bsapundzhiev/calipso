#ifndef _COMPAT_H
#define _COMPAT_H
/* POSIX compat */
#include <stdint.h>
/* types */
typedef _w64 int	ssize_t;
typedef int  		pid_t;
typedef int  		uid_t;
typedef int  		gid_t;
typedef void*		timer_t;
typedef uint16_t	u_int16_t;
typedef uint32_t	socklen_t;
typedef unsigned int	u_int;
/* !types */

#define __func__		__FUNCTION__
#define inline 			__inline

//#define sprintf 	_sprintf
#define snprintf 		_snprintf
#define	strdup			_strdup
#define strcasecmp 		_stricmp  
#define strncasecmp 	_strnicmp
#define getpid			_getpid

//socket
#define SHUT_RDWR		2 //SD_BOTH
#define ioctl 			ioctlsocket
#define EWOULDBLOCK 	EAGAIN //WSAEWOULDBLOCK


#ifndef S_ISDIR
#define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#endif

#ifndef S_ISREG
#define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#endif

#define access _access
#define F_OK 0x0 // 00 Existence only
//02 Write-only
//04 Read-only
//06 Read and write

#define	STDIN_FILENO	fileno(stdin)
#define STDOUT_FILENO	fileno(stdout)

#ifndef VERSION
#define	VERSION "0.1.0"
#endif

#ifdef _WIN32
#define OS 	"Win32"
#endif

int pread(unsigned int fd, char *buf, size_t count, off_t offset);
int setenv(const char *name, const char *value, int overwrite);
uid_t getgid();
uid_t getuid();
int daemon();
int clearenv();
int chroot(const char *chrootpath);
void heapdump( void );
char *remove_trailing_slash(char *str);
long getpagesize (void);

#endif

