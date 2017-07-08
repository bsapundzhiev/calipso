/**
 * well modules will look like this:
 * cm_init
 * cm_process_call
 * cm_exit
 */

#include <calipso.h>

int pm_init()
{
    TRACE("register module HOOK_INIT here\n");
    return 0;
}

int pm_configure()
{
    TRACE("register module HOOK_CONFIGURE here\n");
    return 0;
}

int pm_exit()
{
    TRACE("register module HOOK_EXIT here\n");
    return 0;
}

