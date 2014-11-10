#include "compat.h"

#include <string.h>
#include <stdlib.h>

extern char **environ;
/*bionic impl*/
int clearenv(void)
{
    char **P = environ;
    if (P != NULL) {
        for (; *P; ++P)
            *P = NULL;
    }

    return 0;
}