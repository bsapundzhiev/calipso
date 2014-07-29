/* timer.c - simple POSIX timer
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

#define TIMERSIGNAL SIGRTMIN //SIGALRM 

static timer_t create_timer(int signo, void *data)
{
    timer_t timerid;
    struct sigevent se;

    se.sigev_notify = SIGEV_SIGNAL;
    se.sigev_signo = signo;
    //se.sigev_value.sival_int = timer_nr;
    se.sigev_value.sival_ptr = data;

    if (timer_create(CLOCK_REALTIME, &se, &timerid) == -1) {
        perror("Failed to create timer!");
        return 0;
    }

    return timerid;
}

static void signal_handler(int sig, siginfo_t *si, void *ctx)
{
    calipso_client_t *client = (calipso_client_t *) si->si_value.sival_ptr;

    if (client) {
        TRACE("signal_handler client @ %p : %d\n", client, client->csocket);
        //calipso_client_disconnect(client);
        shutdown(client->csocket, SHUT_RDWR);
    }
}

static void set_timer(timer_t timerid, int seconds)
{
    sigset_t mask;
    struct itimerspec timervals;

    timervals.it_value.tv_sec = seconds;
    timervals.it_value.tv_nsec = 0;
    timervals.it_interval.tv_sec = seconds;
    timervals.it_interval.tv_nsec = 0;

    sigemptyset(&mask);
    sigaddset(&mask, TIMERSIGNAL);
    sigprocmask(SIG_SETMASK, &mask, NULL);

    if (timer_settime(timerid, 0, &timervals, NULL) == -1) {
        perror("Failed to start timer");
    }

    sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

static void install_sighandler(int signo,
                               void (*handler)(int, siginfo_t *, void*))
{
    struct sigaction sa;
    /* Setup the handler */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigaction(signo, &sa, NULL);
}

timer_t tmr_alrm_create(calipso_client_t *client, int sec)
{
    timer_t timerid = create_timer(TIMERSIGNAL, client);

    install_sighandler(TIMERSIGNAL, signal_handler);
    set_timer(timerid, sec);

    return timerid;
}

void tmr_alrm_kill(timer_t timerid)
{
    if (timerid) {
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
 timer_t timer1 = tmr_alrm_create(NULL, 5);
 timer_t timer2 = tmr_alrm_create(NULL, 5);

 while (1) {

 sleep(5);

 if(timer1 > 0 )
 {

 tmr_alrm_kill(timer1);
 timer1 = 0;
 }
 }

 return 0;
 }
 */
