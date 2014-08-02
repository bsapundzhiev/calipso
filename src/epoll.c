/* epoll.c - level-trgered event module
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

//Linux epoll
#include <sys/epoll.h>

static int kdpfd;
static struct epoll_event pfds[MAX_EVENTS];

static int epoll_add_conn(cpo_event_t * event, int nfds);
static int epoll_del_conn(cpo_event_t * event);
static int epoll_process(int nfds);
static int epoll_init();
static int epoll_done();
// epoll module
cpo_events_t cpo_epoll_events = { 
	"epoll_events", 
	epoll_add_conn,
	epoll_del_conn, 
	epoll_process, 
	epoll_init, 
	epoll_done,
};

static int epoll_get_event_fd(cpo_event_t * event)
{
    int fd = 0;
    if (event->data) {
        if (event->type == EVENT_LISTENER)
            fd = ((calipso_socket_t *) event->data)->lsocket;
        else if (event->type == EVENT_CONNECTION)
            fd = ((calipso_client_t *) event->data)->csocket;
    }

    return fd;
}

int epoll_add_conn(cpo_event_t * event, int nfds __attribute__((unused)))
{
    struct epoll_event ev;

    int fd = epoll_get_event_fd(event);
    if (fd == 0)
        return CPO_ERR;

    ev.data.fd = fd;
    ev.data.ptr = event;
    ev.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET;

    event->active = 1;

    if (event->type == EVENT_CONNECTION
            && ((calipso_client_t *) event->data)->done) {
        //&& ((calipso_client_t *)event->data)->request) {
        ev.events |= EPOLLOUT;
    }

    if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, fd, &ev) < 0) {

        if (errno == EEXIST) {
            //printf("ERROR: EPOLL_CTL_ADD client: %d due %s\n", fd, strerror(errno));
            epoll_ctl(kdpfd, EPOLL_CTL_MOD, fd, &ev);
        }
    }

    return OK;
}

int epoll_del_conn(cpo_event_t * event)
{
    //struct epoll_event ev;
    int fd = epoll_get_event_fd(event);
    if (fd == 0)
        return CPO_ERR;

    epoll_ctl(kdpfd, EPOLL_CTL_DEL, fd, pfds);

    return OK;
}

int epoll_process(int nfds)
{
    int i;
    calipso_socket_t *listener;
    calipso_client_t *client;

    if ((nfds = epoll_wait(kdpfd, pfds, nfds, -1)) < 0) {
        if (errno == EINTR) {
            printf("poll interupted\n");
            return CPO_ERR;
        }

        TRACE("poll error %s\n", strerror(errno));
        exit(1);
    }

    for (i = 0; i < nfds; ++i) {

        if (pfds[i].events & (EPOLLIN | EPOLLOUT)) {

            cpo_event_t * event = pfds[i].data.ptr;
            assert(event != NULL);
            if ((pfds[i].events & EPOLLIN) && event->type == EVENT_LISTENER) {

                listener = event->data;

                if (calipso_socket_accept_client(listener) == NULL) {
                    TRACE("FIXME: maxconn reached\n");
                }

            } else if (event->type == EVENT_CONNECTION) {

                client = event->data;
                assert(client != NULL);
                if ((pfds[i].events & EPOLLIN)) {
                    TRACE("x1\n");
                    client->pending_bytes = calipso_client_sent_data(client);

                    if (client->pending_bytes == 0) {

                        epoll_del_conn(event);
                        calipso_client_disconnect(client);
                        continue;

                    } else {

                        assert(event->handler_read != NULL);
                        event->handler_read(client);
                    }
                    TRACE("x2\n");
                }

                if ((pfds[i].events & EPOLLOUT)) {
                    TRACE("in 1\n");
                    assert(event->handler_write != NULL);

                    if (event->handler_write(client)) {

                        epoll_del_conn(event);
                        calipso_client_disconnect(client);
                        TRACE("out 2 disconn\n");
                        continue;

                    }
                    TRACE("out 3 again\n");
                }
            }

            epoll_del_conn(event);
        }
    }

    return OK;
}

int epoll_init()
{
    kdpfd = epoll_create(MAX_EVENTS);

    if (kdpfd == -1) {
        perror("epoll_create");
        exit(EXIT_FAILURE);
    }

    return OK;
}

int epoll_done()
{
    return NOK;
}

