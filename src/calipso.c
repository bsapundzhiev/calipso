/* calipso.c
 *
 * Copyright (C) 2007 Borislav Sapundzhiev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

#include "cplib.h"
#include "calipso.h"

#define LISTENERS_BUCKETS_SIZE 	12
#define HANDLERS_BUCKETS_SIZE	128

calipso_t *calipso = NULL;

static int calipso_load_log_default (calipso_t * calipso);
static int calipso_load_env ();

int
calipso_init (void)
{
    int i;
    calipso = xmalloc (sizeof (calipso_t));

    if (!calipso) {
        return CPO_ERR;
    }

    calipso->pool = cpo_pool_create (CALIPSO_DEFAULT_POOLSIZE);

    calipso->config = NULL;
    calipso->log = NULL;
    calipso->listeners = hash_table_create (LISTENERS_BUCKETS_SIZE, NULL);
    calipso->m_handler = hash_table_create (HANDLERS_BUCKETS_SIZE, NULL);

    for (i = 0; i < NR_HOOKS; i++) {

        calipso->m_hook[i] = list_new ();
    }

    calipso_load_env ();

    time (&(calipso->start_time));
#ifndef WP8
    calipso_load_log_default (calipso);
#endif
    calipso_set_uid (getuid ());
    calipso_set_gid (getgid ());

    calipso_set_pw (NULL);
    calipso_set_gr (NULL);

    return CPO_OK;
}

static void
calipso_unalloc_listeners (hash_t * listeners)
{
    hash_size i;
    for (i = 0; i < hash_table_get_size (listeners); ++i) {

        hash_node_t *node = calipso->listeners->nodes[i];

        while (node) {
            printf ("unalloc listener %s \n", node->key);
            calipso_socket_unalloc (node->data);
            node = node->next;
        }
    }

    hash_table_destroy (calipso->listeners);
}

void
calipso_destroy ()
{
    int i;
    List *p;
    TRACE ("calipso free resources\n");

    calipso_trigger_hook (HOOK_END);

    if (calipso->config) {
        config_unalloc (calipso->config);
    }

    if (calipso->m_handler)
        hash_table_destroy (calipso->m_handler);

    if (calipso->listeners) {
        calipso_unalloc_listeners (calipso->listeners);
    }

    for (i = 0; i < NR_HOOKS; i++) {
        p = calipso->m_hook[i];
        if (p) {
            list_delete (p);
            free (p);
        }
    }

    if (calipso->log) {
        cpo_log_unalloc (calipso->log);
    }

    free (calipso->mod_table);

    cpo_pool_destroy (calipso->pool);

    free (calipso);

    TRACE ("END.\n");
}

static int
calipso_load_log_default (calipso_t * calipso)
{
    cpo_log_t *log = cpo_log_alloc ();

    if (!log)
        return CPO_ERR;

    if (calipso->config) {
        log->file_access = (char *) config_get_option (calipso->config,
                           "acces_log", NULL);
        log->file_error = (char *) config_get_option (calipso->config,
                          "error_log", NULL);
    }

    if (!log->file_access)
        log->file_access = LOG_FILE_ACCESS;

    if (!log->file_error)
        log->file_error = LOG_FILE_ERROR;

    cpo_log_set_level (log, LOG_ERROR);

    calipso->log = log;

    return cpo_log_open (calipso->log);
}

static int
calipso_load_env ()
{
    /* XXX: mod_env - reg env vars */
    clearenv ();
#ifdef _WIN32
    setenv ("PATH", "C:\\WINDOWS;C:\\WINDOWS\\System32", 1);
#else
    setenv ("LD_LIBRARY_PATH", "/usr/local/lib:/usr/lib", 1);
    setenv ("PATH", "/usr/local/bin:/usr/bin:/bin", 1);
#endif

    return CPO_OK;
}

int
calipso_set_pool (calipso_pool_t * pool)
{
    return ((calipso->pool = pool) != NULL);
}

calipso_pool_t *
calipso_get_pool (void)
{
    return (calipso->pool);
}

int
calipso_set_config (calipso_config_t * config)
{
    return ((calipso->config = config) != NULL);
}

calipso_config_t *
calipso_get_config (void)
{

    return (calipso->config);
}

int
calipso_set_listeners_hash (hash_t * hash)
{
    return ((calipso->listeners = hash) != NULL);
}

hash_t *
calipso_get_listeners_hash (void)
{
    return (calipso->listeners);
}

List *
calipso_get_listeners_list ()
{
    u_int i;
    hash_node_t *node;
    hash_t *listener_hash = calipso_get_listeners_hash ();

    List *listeners_list = list_new ();
    for (i = 0; i < hash_table_get_size (listener_hash); ++i) {

        node = listener_hash->nodes[i];
        while (node) {
            list_append (listeners_list, node->data);
            node = node->next;
        }
    }

    return listeners_list;
}

int
calipso_add_listener (calipso_socket_t * listener)
{
    char key[6];
    hash_t *hash_entry;

    cpo_itoa (key, listener->lsocket);
    hash_entry = calipso_get_listeners_hash ();

    if (hash_entry) {

        hash_table_update (hash_entry, key, listener);
    }

    return CPO_OK;
}

List *
calipso_get_m_hook (int hook)
{
    return calipso->m_hook[hook];
}

int
calipso_set_handler_hash (hash_t * hash)
{
    return ((calipso->m_handler = hash) != NULL);
}

hash_t *
calipso_get_handler_hash (void)
{
    return (calipso->m_handler);
}

int
calipso_register_handler (char *type, int (*f) (calipso_request_t *))
{
    void *p = hash_table_get_data (calipso->m_handler, type);

    if (p) {
        TRACE ("handler for %s is already set!\n", type);
    } else {
        hash_table_insert (calipso->m_handler, type, f);
    }

    return CPO_OK;
}

void *
calipso_get_handler (char *type)
{
    int (*f) (void *);

    f = hash_table_get_data (calipso->m_handler, type);

    if (f == NULL)
        return (calipso_get_handler ("*/*"));
    return f;
}

/* modules */
int
calipso_set_module_table (calipso_mod_t * mod_table)
{
    return ((calipso->mod_table = mod_table) != NULL);
}

calipso_mod_t *
calipso_get_module_table (void)
{
    return (calipso->mod_table);
}

/* procs */
int
calipso_set_nprocesses_array (btree_t * array)
{
    return ((calipso->nprocesses = array) != NULL);
}

int
calipso_set_current_process (calipso_process_t * process)
{
    return ((calipso->currentproc = process) != NULL);
}

calipso_process_t *
calipso_get_current_process (void)
{
    return (calipso->currentproc);
}

int
calipso_set_mprocess (calipso_process_t * process)
{
    return ((calipso->mprocess = process) != NULL);
}

int
calipso_set_mprocess_model (int (*mproc_model) (void))
{
    return ((calipso->mproc_model = mproc_model) != 0);
}

int (*calipso_get_mprocess_model (void)) (void)
{
    return (calipso->mproc_model);
}

int
calipso_set_nprocess_model (int (*nproc_model) (void))
{
    return ((calipso->nproc_model = nproc_model) != 0);
}

int (*calipso_get_nprocess_model (void)) (void)
{
    return (calipso->nproc_model);
}

/* internal */
char *
calipso_get_server_string (calipso_config_t * config)
{
    if(config) {/*TODO:*/}
    return ("Calipso/" VERSION);
}

int
calipso_set_pw (void *pw)
{
    return ((calipso->pw = pw) != NULL);
}

int
calipso_set_gr (void *gr)
{
    return ((calipso->gr = gr) != NULL);
}

int
calipso_set_uid (uid_t uid)
{
    return ((calipso->uid = uid));
}

int
calipso_set_gid (gid_t gid)
{
    return ((calipso->gid = gid));
}

int
calipso_get_uid ()
{
    return (calipso->uid);
}

int
calipso_get_gid ()
{
    return (calipso->gid);
}

char *
calipso_get_name (void)
{
    return ("Calipso");
}

char *
calipso_get_version (void)
{
    return (VERSION);
}

cpo_log_t *
calipso_get_log ()
{
    return calipso->log;
}
