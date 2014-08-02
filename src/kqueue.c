/* kqueue.c - edge-trgered event module
 *
 * Copyright (C) 2014 Borislav Sapundzhiev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

#include "calipso.h"
#include "event.h"


static struct kevent *pfds;
static int _kqueue;
static struct kevent evlist[MAX_EVENTS];

static int kqueue_add_conn(cpo_event_t * event, int nfds);
static int kqueue_del_conn(cpo_event_t * event);
static int kqueue_process(int nfds);
static int kqueue_init();
static int kqueue_done();

cpo_events_t cpo_kqueue_events = {
    "kqueue_events",
    kqueue_add_conn,
    kqueue_del_conn,
    kqueue_process,
    kqueue_init,
    kqueue_done,
};

int kqueue_get_event_fd(cpo_event_t * event)
{
    int fd = 0;
    if (event->type == EVENT_LISTENER)
        fd = ((calipso_socket_t *)event->data)->lsocket;

    else if (event->type == EVENT_CONNECTION)
        fd = ((calipso_client_t *)event->data)->csocket;

    return fd;
}

int kqueue_add_conn(cpo_event_t * event , int nfds)
{
    int fd = kqueue_get_event_fd(event);
    if (fd == 0) return CPO_ERR;

    printf("add ndfs %d fd %d\n", nfds, fd);

    pfds[nfds].ident 	= fd;
    pfds[nfds].flags 	= EV_ADD | EV_ENABLE;
    pfds[nfds].filter	= EVFILT_READ;
    pfds[nfds].udata 	= event;

    if (event->type == EVENT_CONNECTION
            && ((calipso_client_t *)event->data)->done) {

        pfds[nfds].filter = EVFILT_WRITE;
    }

    //EV_SET(&pfds[nfds], fd, pfds[nfds].filter, pfds[nfds].flags, 0, 0, 0);

    return CPO_OK;
}

int kqueue_del_conn(cpo_event_t * event)
{
    struct kevent ke1;
    int res;
    int fd = kqueue_get_event_fd(event);
    if (fd == 0) return CPO_ERR;

    TRACE("del conn %d\n", fd);

    bzero(&ke1,sizeof(ke1));
    ke1.ident = fd;
    ke1.flags = EV_DELETE;
    ke1.filter = EVFILT_WRITE;
    res = kevent(_kqueue, &ke1, 1, NULL, 0, NULL);

    return CPO_OK;
}

int kqueue_process(int nfds)
{
    int kqueueret =0, i, nbytes;

    kqueueret = kevent(_kqueue, pfds, nfds, evlist, nfds, NULL);

    if (kqueueret <= 0) {

        if (errno == EINTR) {
            printf("kqueue interupted\n");
            return CPO_ERR;
        }

        TRACE("kqueue error %s\n", strerror(errno));
        //exit(1);
    }

    printf("kqueue ret = %d ndfs= %d\n" , kqueueret , nfds);

    for (i = 0; i < /*nfds &&*/ kqueueret; i++) {
        //--kqueueret;

        if ( evlist[i].filter == EVFILT_READ
                ||  evlist[i].filter == EVFILT_WRITE) {

            cpo_event_t * event = evlist[i].udata;

            if ( evlist[i].filter & EVFILT_READ  &&
                    event->type == EVENT_LISTENER) {

                calipso_socket_t * listener = event->data;

                printf("lsocket: %d\n", listener->lsocket);
                printf("state: %d\n", listener->state);

                if ( calipso_socket_accept_client(listener) == NULL ) {
                    TRACE("FIXME: maxconn reached\n");
                }

            } else if (event->type == EVENT_CONNECTION) {

                calipso_client_t* client = event->data;
                printf("client %d\n", client->csocket);

                if (evlist[i].filter == EVFILT_READ) {

                    if ((nbytes = calipso_client_sent_data(client)) == 0) {
                        printf("nbytes == 0 break; \n");
                        kqueue_del_conn(event);
                        calipso_client_disconnect(client);
                        continue;
                    } else  {
                        TRACE("READING= %d\n", nbytes);
                        client->pending_bytes = nbytes;
                        //calipso_client_read(client, nbytes);
                        assert(event->handler_read != NULL);
                        event->handler_read(client);
                    }

                }

                if (evlist[i].filter == EVFILT_WRITE ) {
                    printf("EVFILT_WRITE\n");

                    assert(event->handler_write != NULL);
                    if (event->handler_write(client)) {
                        kqueue_del_conn(event);
                        calipso_client_disconnect(client);
                        continue;
                    }

                }

            }

        }
    }//for

    return CPO_OK;
}

int kqueue_init()
{
    _kqueue = kqueue();
    if (_kqueue == -1) {
        perror("kqueue");
        exit(EXIT_FAILURE);
    }

    pfds = malloc( MAX_EVENTS * sizeof(struct kevent));

    return CPO_OK;
}

int kqueue_done()
{
    printf("Clear kqueue data\n");
    memset(pfds, 0, MAX_EVENTS * sizeof(struct kevent));
    return NOK;
}
