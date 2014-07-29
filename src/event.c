/* event.c - Events common
 *
 * Copyright (C) 2012 Borislav Sapundzhiev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

#include "calipso.h"
#include "event.h"

static cpo_events_t * cpo_events = NULL;

/* event */
cpo_event_t * cpo_event_alloc(unsigned char type)
{
    cpo_event_t *ev = (cpo_event_t *) malloc(sizeof(cpo_event_t));
    if (ev == NULL)
        return NULL;

    ev->type = type;
    ev->data = NULL;
    ev->handler_read = ev->handler_write = NULL;
    ev->active = NOK;
    return ev;
}

void cpo_event_unalloc(cpo_event_t * event)
{
    free(event);
    event = NULL;
}

int cpo_event_handle_event(cpo_event_t * event)
{
    if (event->active) {
        cpo_events->event_del_conn(event);
        event->active = 0;
    }

    return CPO_OK;
}

/* events */
static int cpo_events_pollset(List* listeners)
{
    int nfds = 0;
    List * listeners_entry;
    dllist_t * client_entry;
    queue_t * client_list;
    calipso_socket_t *listener;
    calipso_client_t *client;

    /* for each listener */
    for (listeners_entry = list_get_first_entry(listeners); listeners_entry;
            listeners_entry = list_get_next_entry(listeners_entry)) {

        listener = (calipso_socket_t *) list_get_entry_value(listeners_entry);

        if ((listener->state & SOCKET_STATE_ACTIVE)) {

            assert(listener->event != NULL);
            if (listener->event) {
                cpo_events->event_add_conn(listener->event, nfds);
                nfds++;
            }

            client_list = calipso_socket_get_client_list(listener);
            /* for each conn */
            for (client_entry = client_list->head; client_entry != NULL;
                    client_entry = client_entry->next) {

                client = (calipso_client_t *) client_entry->data;

                assert(listener->event != NULL);
                if (client->event) {
                    cpo_events->event_add_conn(client->event, nfds);
                    nfds++;
                }
            }

        }

    }

    return nfds;
}

void cpo_events_init()
{
    calipso_config_t *config = calipso_get_config();
    const char *server_model = config_get_option(config, "ServerModel", NULL);

    if (server_model) {

        if (!strcasecmp(server_model, "EpollModel")) {
#ifdef USE_EPOLL
            cpo_events = &cpo_epoll_events;
#endif
        }

        if (!strcasecmp(server_model, "PollModel")) {
#ifdef USE_POLL
            cpo_events = &cpo_poll_events;
#endif
        }

		if(!strcasecmp(server_model, "KqueueModel")) {
#ifdef USE_KQUEUE
			cpo_events = &cpo_kqueue_events;
#endif
		}
    } else {
        printf("Unknown event model %s\n", server_model);
        exit(EXIT_FAILURE);
    }
}

void cpo_events_loop()
{
    int nfds = 0;
    List* listeners = calipso_get_listeners_list();

    if (!cpo_events) {
        TRACE("No event model\n");
        exit(EXIT_FAILURE);
    }

    cpo_events->event_init();

    while (OK != cpo_events->event_done()) {

        nfds = cpo_events_pollset(listeners);

        cpo_events->event_process(nfds);
    }
}

