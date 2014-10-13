/* poll.c - edge-trgered event module
 *
 * Copyright (C) 2007 Borislav Sapundzhiev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

#ifndef _WIN32
#include <sys/poll.h>
#endif
#include "calipso.h"
#include "event.h"

struct _fdtype {
    int type;
    void *c;
};

static struct pollfd *pfds;
static struct _fdtype *fdtype;

static int poll_add_conn(cpo_event_t * event, int nfds);
static int poll_del_conn(cpo_event_t * event);
static int poll_process(int nfds);
static int poll_init();
static int poll_done();
static int poll_get_fd(cpo_event_t * event);

cpo_events_t cpo_poll_events = { 
	"poll_events", 
	poll_add_conn, 
	poll_del_conn,
	poll_process, 
    poll_init, 
    poll_done,
};

static int poll_get_fd(cpo_event_t * event)
{
    int fd = 0;

    if (event->type == EVENT_LISTENER)
        fd = ((calipso_socket_t *) event->data)->lsocket;

    else if (event->type == EVENT_CONNECTION)
        fd = ((calipso_client_t *) event->data)->csocket;

    return fd;
}

int poll_add_conn(cpo_event_t * event, int nfds)
{
	int fd = poll_get_fd(event);

    if (fd == 0)
        return CPO_ERR;

    fdtype[fd].type = event->type;
    fdtype[fd].c = event;
    pfds[nfds].fd = fd;
    pfds[nfds].events = POLLIN;

    if (event->type == EVENT_CONNECTION
            && ((calipso_client_t *) event->data)->done) {
        //&& ((calipso_client_t *)event->data)->request) {
        pfds[nfds].events |= POLLOUT;
    }

    return CPO_OK;
}

int poll_del_conn(cpo_event_t * event)
{
	int fd = poll_get_fd(event);

    if (fd == 0)
        return CPO_ERR;
	fdtype[fd].type = 0;
    fdtype[fd].c = NULL;
    return CPO_OK;
}

int poll_process(int nfds)
{
    int pollret = 0, i, nbytes;

    if ((pollret = poll(pfds, nfds, -1)) < 0) {

        if (errno == EINTR) {
            printf("poll interupted\n");
            return CPO_ERR;
        }

        TRACE("poll error %s\n", strerror(errno));
        exit(1);
    }

    for (i = 0; i < nfds/*nfds && pollret*/; i++) {
        //--pollret;

        if (pfds[i].revents & (POLLIN | POLLOUT)) {

            cpo_event_t * event = fdtype[pfds[i].fd].c;

            if ((pfds[i].revents & POLLIN) && event->type == EVENT_LISTENER) {

                calipso_socket_t * listener = event->data;
                if (calipso_socket_accept_client(listener) == NULL) {
                    TRACE("FIXME: maxconn reached\n");
                }

            } else if (event->type == EVENT_CONNECTION) {

                calipso_client_t* client = event->data;
              

                if (pfds[i].revents & POLLIN) {

                    if ((nbytes = calipso_client_sent_data(client)) == 0) {
                        
                        calipso_client_disconnect(client);
                        continue;
                    } else {
                        
                        client->pending_bytes = nbytes;
                        assert(event->handler_read != NULL);
                        event->handler_read(client);
                    }

                }

                if (pfds[i].revents & POLLOUT) {

                    assert(event->handler_write != NULL);
                    if (event->handler_write(client)) {
                        calipso_client_disconnect(client);
                        continue;
                    }

                }

            }
            
			poll_del_conn(event);
        }
    } //for

    return CPO_OK;
}

int poll_init()
{
    pfds = malloc( MAX_EVENTS * sizeof(struct pollfd));
    fdtype = malloc( MAX_EVENTS * sizeof(struct _fdtype));

    return CPO_OK;
}

int poll_done()
{
    memset(pfds, 0, MAX_EVENTS * sizeof(struct pollfd));
    //memset(fdtype, 0, MAX_EVENTS * sizeof(struct _fdtype));
    return NOK;
}

