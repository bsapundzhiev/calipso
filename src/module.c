/* module.c - simple shared lib loader
 *
 * Copyright (C) 2007 Borislav Sapundzhiev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

/**
 * well modules will look like this:
 * cm_init
 * cm_process_call
 * cm_exit
 */

#include <stdio.h>
#include <sys/types.h>

#include "calipso.h"
#include "module.h"

static int
calipso_register_module_hook(void *module_hdl, int hook,
                             char const *function_name);
static void calipso_register_module(calipso_mod_t * m, const char *module_name,
                                    const char *path);
static calipso_mod_t *calipso_load_modules(calipso_config_t * config);

#ifdef _WIN32
static void calipso_win32_intializer(void *module_hdl)
{
    typedef void (*pt2f) (calipso_t *);

    pt2f init = (pt2f) Sys_LoadFunction(module_hdl, "CalipsoModWin32Initializer");
    if (!init) {
        TRACE("CalipsoModWin32Initializer not found\n");
        exit(1);
    }

    init(calipso);

}
#endif

static int calipso_register_module_hook(void *module_hdl, int hook,
                                        char const *function_name)
{
    char *error;
    void *p_addr = NULL;

#ifdef _WIN32
    calipso_win32_intializer(module_hdl);
#endif

    if (function_name == NULL) {
        switch (hook) {
        case HOOK_INIT:
            p_addr = Sys_LoadFunction(module_hdl, MODULE_INIT_FUNCTION);
            break;
        case HOOK_END:
            p_addr = Sys_LoadFunction(module_hdl, MODULE_EXIT_FUNCTION);
            break;
        case HOOK_CONFIGURE:
            p_addr = Sys_LoadFunction(module_hdl, MODULE_CONFIGURE_FUNCTION);
            break;
        default:
            TRACE("Bad hook");
            return 0;
        }
    } else {
        p_addr = Sys_LoadFunction(module_hdl, function_name);
    }

    if ((error = Sys_LibraryError()) != NULL) {
        fprintf(stderr, "%s: error: %s\n", __func__, error);
        exit(-1);
    }

    calipso_register_hook(hook, p_addr);

    return CPO_OK;
}

static void calipso_register_module(calipso_mod_t * m, const char *module_name,
                                    const char *path)
{
    m->handler = Sys_LoadLibrary(path);

    if (!m->handler) {
	TRACE("dlerror= %s - terminate\n",Sys_LibraryError());
        fprintf(stderr, "%s():%d dlerror= %s - terminate\n", __func__, __LINE__,
                Sys_LibraryError());
        exit(-1);
    }

    m->mod_name = module_name;

    /* Clear any existing error */
    Sys_LibraryError();
}

static calipso_mod_t *calipso_load_modules(calipso_config_t * config)
{
    int nr_modules = 0;

    calipso_mod_t *module_table = calloc(MAX_LOADED_MODULES,
                                         sizeof(calipso_mod_t));

    /* init module table */
    for (config = list_get_first_entry(config); config != NULL; config =
                list_get_next_entry(config)) {

        conf_ctx_t *conf = list_get_entry_value(config);

        if (conf && strcmp(conf->option, "load_module") == 0) {

            char *name = strtok(conf->value, ":");
            char *path = strtok(NULL, ":");
            TRACE("load_module: %s\n", path);

            if (MAX_LOADED_MODULES > nr_modules) {
                calipso_register_module(&module_table[nr_modules], name, path);
                nr_modules++;
            } else {
                fprintf(stderr, "MAX_LOADED_MODULES = %d reached\n",
                        MAX_LOADED_MODULES);
            }
        }
    }

    return module_table;
}

int calipso_modules_init(calipso_config_t * conf, int realy_load)
{
    calipso_mod_t *mt = calipso_load_modules(conf);
    if (mt == NULL)
        return CPO_ERR;

    calipso_set_module_table(mt);

    /* show modules */
    if (!realy_load)
        fprintf(stderr, "\n---Calipso modules table:\n");

    for (; mt && mt->mod_name != NULL; mt++) {

        if (realy_load) {
            calipso_register_module_hook(mt->handler, HOOK_INIT,
                                         MODULE_INIT_FUNCTION);
            calipso_register_module_hook(mt->handler, HOOK_END,
                                         MODULE_EXIT_FUNCTION);
        } else {
            fprintf(stderr, "mod_name: %-15s mod_handler: %p\n", mt->mod_name,
                    mt->handler);
        }
    }

    if (!realy_load) {
        fprintf(stderr, "---End.\n");
        calipso_destroy();
        exit(0);
    }

    return CPO_OK;
}

cpo_bool calipso_modules_is_module(const char *module_name)
{
    calipso_mod_t *mt = calipso_get_module_table();
    if (mt == NULL)
        return CPO_ERR;

    for (; mt && mt->mod_name != NULL; mt++) {

        if (!strcmp(mt->mod_name, module_name)) {
            return CPO_OK;
        }
    }

    return NOK;
}

