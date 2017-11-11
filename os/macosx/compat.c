#include "compat.h"
#include <crt_externs.h>

int clearenv()
{
    size_t size;
    char ***result = _NSGetEnviron ();
    char **env = *result;

    while (env[0] != NULL) {
        size = 0;
        while (env[0][size] != '=')
            size++;
        size++;
        {
            char expression[size];
            strncpy (expression, env[0], size);
            expression[size - 1] = 0;
            unsetenv (expression);
        }
    }

    return 0;
}