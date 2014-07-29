/* hooks.c - server hooks
 *
 * Copyright (C) 2007 Borislav Sapundzhiev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

#include <stdarg.h>
#include "calipso.h"
#include "hooks.h"

const char *hook_name[] = {

    "HOOK_LOAD_DYNAMIC", "HOOK_INIT", "HOOK_CONFIGURE", "HOOK_CHROOT",
    "HOOK_PRIVILEGES", "HOOK_CONNECT", "HOOK_DISCONNECT", "HOOK_REQUEST",
    "HOOK_TRANSLATE", "HOOK_RESOURCE", "HOOK_AUTH_ID_CHECK",
    "HOOK_AUTH_ACCESS_CHECK", "HOOK_ACCESS_CHECK", "HOOK_MIME",
    "HOOK_REPLY", "HOOK_LOG", "HOOK_END",
};

int calipso_hook_get_NR_HOOKS()
{
    return NR_HOOKS;
}

int calipso_trigger_hook(int hook, ...)
{
    int ret;
    List *p;
    List *list_entry;
    calipso_client_t *client;
    calipso_request_t *request;
    va_list ap;

    va_start(ap, hook);
    p = calipso_get_m_hook(hook);

    if (/* p->size == 0 && */p == NULL && hook != HOOK_REPLY) {
        va_end(ap);
        TRACE("BUG: bad hook\n");
        return (0);
    }

    //printf("TODO: triger hook:  %d %p\n",hook,p);

    switch (hook) {
    case HOOK_LOAD_DYNAMIC:
    case HOOK_CONFIGURE:
    case HOOK_INIT:
    case HOOK_CHROOT:
    case HOOK_PRIVILEGES:
    case HOOK_END:

        list_entry = list_get_first_entry(p);
        while (list_entry) {
            if (list_entry->data)
                ((int (*)(void)) list_get_entry_value(list_entry))();
            list_entry = list_get_next_entry(list_entry);
        }
        break;

    case HOOK_CONNECT:
    case HOOK_DISCONNECT:

        list_entry = list_get_first_entry(p);
        client = va_arg(ap, calipso_client_t *);
        while (list_entry) {
            if (list_entry->data)
                ((int (*)(calipso_client_t *)) list_get_entry_value(list_entry))(
                    client);
            list_entry = list_get_next_entry(list_entry);
        }
        break;

    case HOOK_REQUEST:
    case HOOK_TRANSLATE:
    case HOOK_RESOURCE:
    case HOOK_MIME:
    case HOOK_LOG:

        list_entry = list_get_first_entry(p);
        request = va_arg(ap, calipso_request_t *);
        while (list_entry) {
            if (list_entry->data)
                ((int (*)(calipso_request_t *)) list_get_entry_value(list_entry))(
                    request);
            list_entry = list_get_next_entry(list_entry);
        }
        break;

    case HOOK_ACCESS_CHECK:
        list_entry = list_get_first_entry(p);
        request = va_arg(ap, calipso_request_t *);
        /* NEEDS_FIX - no need to loop through all callbacks if one fails */
        while (list_entry) {
            if (list_entry->data)
                ((int (*)(calipso_request_t *)) list_get_entry_value(list_entry))(
                    request);
            list_entry = list_get_next_entry(list_entry);
        }
        break;

    case HOOK_REPLY:
        request = va_arg(ap, calipso_request_t *);
        ret = calipso_request_handler(request);
        va_end(ap);
        return (ret);

    } //!switch

    va_end(ap);

    return CPO_OK;
}

void calipso_register_hook(int hook, void *callback_fn)
{
    List *hook_ptr = calipso_get_m_hook(hook);

    if (hook_ptr && callback_fn) {

        list_append(hook_ptr, callback_fn);
    }
}

/* debug */
const char *get_hook_byid(const unsigned int id)
{
    if (NR_HOOKS < id && ARRAYSZ(hook_name) < id) {
        return NULL;
    }

    return hook_name[id];
}

void calipso_hook_dump(int hook)
{
    void *p;
    int i = 0;
    List *hook_ptr;
    List *list_entry;

    hook_ptr = calipso_get_m_hook(hook);
    list_entry = list_get_first_entry(hook_ptr);

    while (list_entry != NULL) {
        p = list_get_entry_value(list_entry);
        printf("<[%d] nr_hook [%s], h_addr: %p>\n", i, get_hook_byid(hook), p);
        list_entry = list_get_next_entry(list_entry);
        i++;
    }
}

