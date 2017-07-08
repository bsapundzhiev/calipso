#include "calipso.h"
#include "compat.h"

int pread(unsigned int fd, char *buf, size_t count, off_t offset)
{
    if (_lseek(fd, offset, SEEK_SET) != offset) {
        return -1;
    }

    return read(fd, buf, count);
}

char *remove_trailing_slash(char *str)
{
    int len;
    char *p = str;
    len = strlen(p) - 1;

    if( !strcmp(p + len,"\\") || !strcmp(p + len, "/")) {
        p[len] = '\0';
    }
    return p;
}

int setenv(const char *name, const char *value, int overwrite)
{
    int result = 0;
    if (overwrite || !getenv(name)) {
        size_t length = strlen(name) + strlen(value) + 2;
        char *string = malloc(length);
        snprintf(string, length, "%s=%s", name, value);
        result = putenv(string);
        free(string);
    }
    return result;
}

/* getpagesize for windows */
long getpagesize (void)
{
    long pagesize = 0;
    if (! pagesize) {
        SYSTEM_INFO system_info;
        GetSystemInfo (&system_info);
        pagesize = system_info.dwPageSize;
    }
    return pagesize;
}

uid_t getuid()
{
    return 0;
}

uid_t getgid()
{
    return 0;
}

void worker_model()
{

}

void event_model_sched()
{
}

int daemon()
{
    return 0;
}

int clearenv()
{
    return 0;
}

int chroot(const char *chrootpath)
{
    return 0;
}

void heapdump( void )
{
    _HEAPINFO hinfo;
    int heapstatus;
    hinfo._pentry = NULL;
    while( ( heapstatus = _heapwalk( &hinfo ) ) == _HEAPOK ) {
        printf( "%6s block at %Fp of size %4.4X\n",
                ( hinfo._useflag == _USEDENTRY ? "USED" : "FREE" ),
                hinfo._pentry, hinfo._size );
    }

    switch( heapstatus ) {
    case _HEAPEMPTY:
        printf( "OK - empty heap\n" );
        break;
    case _HEAPEND:
        printf( "OK - end of heap\n" );
        break;
    case _HEAPBADPTR:
        printf( "ERROR - bad pointer to heap\n" );
        break;
    case _HEAPBADBEGIN:
        printf( "ERROR - bad start of heap\n" );
        break;
    case _HEAPBADNODE:
        printf( "ERROR - bad node in heap\n" );
        break;
    }
}
