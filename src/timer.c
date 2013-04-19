/* timer.c - timers
 *
 * Copyright (C) 2012 Borislav Sapundzhiev
 *         
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

#include <signal.h>
#include <time.h>
#include "timer.h"


int wait_fd(int fd, short stimeout)
{
	fd_set rfds;
	struct timeval tv;
	int retval;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	tv.tv_sec = stimeout;
	tv.tv_usec = 0;

    retval = select(fd + 1, &rfds, NULL, NULL, &tv);
  
    if (retval == -1) {
 		perror("select()");
	}
    else if (retval)
		printf("%s Data is available now.\n", __func__);
               /* FD_ISSET(0, &rfds) will be true. */
    else
		printf("%s No data within %d seconds.\n", __func__, stimeout);
 	/* > 0 if descriptor is readable */
	return retval;
}

static timer_t create_timer(int signo, void *data) 
{
    timer_t timerid;
    struct sigevent se;
    se.sigev_notify=SIGEV_SIGNAL;
    se.sigev_signo = signo;
	//se.sigev_value.sival_int = 0;
	se.sigev_value.sival_ptr = data;

    if (timer_create(CLOCK_REALTIME, &se, &timerid) == -1) {
        perror("Failed to create timer!");
        return 0;
    }
    return timerid;
}

static void signal_handler(int sig, siginfo_t *si, void *ctx)
{
	//int count = si->si_value.sival_int;
	calipso_client_t *client = (calipso_client_t *)si->si_value.sival_ptr;
	
	TRACE("signal_handler client @ %p : %d\n", client, client->csocket);
	if(client) {
		//calipso_client_disconnect(client);
		shutdown(client->csocket, SHUT_RDWR);
	}
	
	//tmr_alrm_reset(client->ctmr, 0);
}

static void set_timer(timer_t timerid, int seconds) 
{
    struct itimerspec timervals;

    timervals.it_value.tv_sec = seconds;
    timervals.it_value.tv_nsec = 0;
    timervals.it_interval.tv_sec = seconds;
    timervals.it_interval.tv_nsec = 0;

    if (timer_settime(timerid, 0, &timervals, NULL) == -1) {
        perror("Failed to start timer");
    }
    return;
}

static void install_sighandler(int signo, void(*handler)(int, siginfo_t *, void*)) 
{
    sigset_t set;
    struct sigaction sa;

    /* Setup the handler */ 
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;  
    sa.sa_sigaction = handler;
    sigaction(signo/*SIGRTMAX*/, &sa, NULL) ;

    /* Unblock the signal */
    sigemptyset(&set);
    sigaddset(&set, signo);
    sigprocmask(SIG_UNBLOCK, &set, NULL);

    return;
}

timer_t tmr_alrm_create(calipso_client_t *client, int sec)
{
	timer_t timerid = create_timer(SIGALRM, client);
	install_sighandler(SIGALRM, signal_handler);
	set_timer(timerid, sec);
 	return timerid;
}

void tmr_alrm_kill(timer_t timerid)
{ 
	if(timerid) {
   		timer_delete(timerid);
	}
}

void tmr_alrm_reset(calipso_client_t *client, int sec) 
{
	set_timer(client->ctmr, sec); 
}

/*
int main()
{
   
    timer_t timer1 = alrm_tmr_create(5);
    timer_t timer2 = alrm_tmr_create(5);

    while (1) sleep(5);
    return 0;
}
*/


