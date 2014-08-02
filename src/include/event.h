/* event.h - Events common
 *
 * Copyright (C) 2012 Borislav Sapundzhiev
 *         
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 */

#ifndef _CPO_EVENT_H
#define _CPO_EVENT_H

enum event_type {
	EVENT_LISTENER,
	EVENT_CONNECTION,
};

#define MAX_EVENTS 1024

/* forward declatarion */
//typedef struct 	s_client calipso_client_t;

typedef struct cpo_event_s {
	unsigned char type;						/* type of data */
	void *data;								/* link to connection */
	int (*handler_read)(void* );	/* event handler read*/
	int (*handler_write)(void*);/* event handler write*/
	unsigned int active;					/* is event active */
} cpo_event_t;

cpo_event_t * cpo_event_alloc(unsigned char type);
void cpo_event_unalloc(cpo_event_t * event);

typedef struct cpo_events_s {
	char *name;
	int (*event_add_conn)(cpo_event_t*, int);
	int (*event_del_conn)(cpo_event_t*);
	int (*event_process)(int);
	int (*event_init)();
	int (*event_done)();
} cpo_events_t;

void cpo_events_loop();
void cpo_events_init();

extern cpo_events_t cpo_epoll_events;
extern cpo_events_t cpo_poll_events;
extern cpo_events_t cpo_kqueue_events;
//extern cpo_events_t * cpo_events;

#endif

