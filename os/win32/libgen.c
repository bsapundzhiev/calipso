#include <string.h>
#include <stdlib.h>
#include "libgen.h"
#include "compat.h"

char *dirname(char *path) 
{
   static char dir[_MAX_DIR];
   char fname[_MAX_FNAME];
   _splitpath(remove_trailing_slash( path ), NULL, dir, fname, NULL);
    return dir;
}

char *basename(char *path) 
{
   char dir[_MAX_DIR];
   static char fname[_MAX_FNAME];
    _splitpath(path, NULL, dir, fname, NULL);
    return fname;
}
