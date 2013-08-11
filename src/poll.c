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

#define	FDTYPE_FREE	0x0
#define	FDTYPE_LISTENER	0x1
#define	FDTYPE_CLIENT	0x2

static struct pollfd *pfds;
static struct _fdtype *fdtype;

static int poll_add_conn(cpo_event_t * event, int nfds);
static int poll_del_conn(cpo_event_t * event);
static int poll_process(int nfds);
static int poll_init();
static int poll_done();

cpo_events_t cpo_poll_events = {
	"poll_events",
	poll_add_conn,
	poll_del_conn,
	poll_process,
	poll_init,
	poll_done,
};

int poll_add_conn(cpo_event_t * event , int nfds)
{
	int fd =0;
	

	if(event->type == EVENT_LISTENER) 
		fd = ((calipso_socket_t *)event->data)->lsocket;

	else if(event->type == EVENT_CONNECTION)
    	fd = ((calipso_client_t *)event->data)->csocket;
	
	if(fd == 0) return CPO_ERR;

	printf("add ndfs %d fd %d\n", nfds, fd);
	fdtype[fd].type 	= event->type;
    fdtype[fd].c    	= event;
    pfds[nfds].fd 		= fd;
    pfds[nfds].events	= POLLIN;

	if(event->type == EVENT_CONNECTION
		&& ((calipso_client_t *)event->data)->done) {  
		//&& ((calipso_client_t *)event->data)->request) {
		 pfds[nfds].events |= POLLOUT;
	}

	return CPO_OK;
}

int poll_del_conn(cpo_event_t * event)
{
	return CPO_OK;
}

int poll_process(int nfds)
{
	int pollret =0 ,i,nbytes;

	if ((pollret = poll(pfds, nfds, -1)) < 0) {

		if (errno == EINTR) {
			printf("poll interupted\n");
      		return CPO_ERR;
		}

       	TRACE("poll error %s\n", strerror(errno));    
        exit(1);
 	}

   	printf("pollret = %d ndfs= %d\n" , pollret , nfds);

	for (i = 0; i < nfds/*nfds && pollret*/; i++) {
		//--pollret;

    	if (pfds[i].revents & (POLLIN|POLLOUT)) {
			
			cpo_event_t * event = fdtype[ pfds[i].fd ].c;
			
			if ((pfds[i].revents & POLLIN) && 
				event->type == EVENT_LISTENER) {

				calipso_socket_t * listener = event->data;

				printf("lsocket: %d\n", listener->lsocket);
		        printf("state: %d\n", listener->state);

				if ( calipso_socket_accept_client(listener) == NULL ) {
		        	TRACE("FIXME: maxconn reached\n");            
		      	}

			} 
			else if(event->type == EVENT_CONNECTION) {

				calipso_client_t* client = event->data;
				printf("client %d\n", client->csocket);
				
				if (pfds[i].revents & POLLIN) {
                       
               		if ((nbytes = calipso_client_sent_data(client)) == 0) {
                    	printf("nbytes == 0 break; \n");
                   		calipso_client_disconnect(client);
						continue;
                  	}
					else  {
                    	TRACE("READING= %d\n", nbytes);
						client->pending_bytes = nbytes;
                       	//calipso_client_read(client, nbytes);
						assert(event->handler_read != NULL);
						event->handler_read(client);
                    }
                        
             	}

              	if (pfds[i].revents & POLLOUT) {
                	printf("POLLOUT\n");
                	//if (calipso_client_write_reply(client)) {
					assert(event->handler_write != NULL);
					if(event->handler_write(client)) {
                		//epoll_del_conn(event);
                    	calipso_client_disconnect(client);
			    		continue;
                    }

            	}

			}

		}		
	}//for

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
	printf("Clerar poll data\n");
	memset(pfds, 0, MAX_EVENTS * sizeof(struct pollfd));
	memset(fdtype, 0, MAX_EVENTS * sizeof(struct _fdtype));

	return NOK;
}

