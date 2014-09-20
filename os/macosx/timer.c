/*
 * pthread stopwatch timer
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#include "timer.h"

typedef void (*tmr_callback)(void*);

typedef struct {
    int             interval;
    pthread_t       thread;
    pthread_mutex_t lock;
    pthread_cond_t  cond;
    long int        waits;
    tmr_callback	callback;
    void *info;
} tmr_thread_arg;

int timeval_subtract (result, x, y)
struct timeval *result, *x, *y;
{
    /* Perform the carry for the later subtraction by updating y. */
    if (x->tv_usec < y->tv_usec) {
        int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
        y->tv_usec -= 1000000 * nsec;
        y->tv_sec += nsec;
    }
    if (x->tv_usec - y->tv_usec > 1000000) {
        int nsec = (y->tv_usec - x->tv_usec) / 1000000;
        y->tv_usec += 1000000 * nsec;
        y->tv_sec -= nsec;
    }

    /* Compute the time remaining to wait.
       tv_usec is certainly positive. */
    result->tv_sec = x->tv_sec - y->tv_sec;
    result->tv_usec = x->tv_usec - y->tv_usec;

    /* Return 1 if result is negative. */
    return x->tv_sec < y->tv_sec;
}

static void *tmr_thread_exec_and_wait(void* args)
{
    tmr_thread_arg *proc = (tmr_thread_arg*) args;
    struct timeval tv, later, interval;
    struct timespec ts = {0,0};
    int rc, count =0;

    pthread_mutex_lock(&proc->lock);
    while ( count != proc->interval) {

        gettimeofday(&tv, NULL);

        ts.tv_sec = time(NULL) + proc->waits / 1000;
        ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (proc->waits % 1000);
        ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
        ts.tv_nsec %= (1000 * 1000 * 1000);

        rc = pthread_cond_timedwait(&proc->cond, &proc->lock, &ts);

        gettimeofday(&later,NULL);

        if (rc == 0) {
            printf("singaled\n");
            break;
        } else if (rc == ETIMEDOUT) {

            if (proc->callback) {
                proc->callback(proc->info);
            }

        }

        timeval_subtract(&interval,&later,&tv);
        printf("[sec=%ld usec=%ld] %ld\n", interval.tv_sec, interval.tv_usec, proc->thread);

        count++;
    }

    pthread_mutex_unlock(&proc->lock);

    printf("thread_exit %ld\n", proc->thread);
    pthread_cond_destroy(&proc->cond);
    pthread_mutex_destroy(&proc->lock);
    free(args);
    pthread_exit(NULL);
    return NULL;
}

static int tmr_init_proc(tmr_thread_arg *proc, void *(*routine) (void *))
{
    //pthread_attr_t attr;
    //
    printf("try to init proc\n");
    //pthread_attr_init(&attr);
    //pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    //
    pthread_mutex_init(&proc->lock, NULL);
    pthread_cond_init(&proc->cond, NULL);

    pthread_create(&proc->thread, NULL/*&attr*/, routine, (void*) proc);
    //pthread_attr_destroy(&attr);

    return 1;
}

static void signal_handler(void* info)
{
    //int count = si->si_value.sival_int;
    calipso_client_t *client = (calipso_client_t *)info;

    TRACE("signal_handler client @ %p : %d\n", client, client->csocket);
    if (client) {
        //calipso_client_disconnect(client);
        shutdown(client->csocket, SHUT_RDWR);
    }

    //tmr_alrm_reset(client->ctmr, 0);
}

void  test_callback(void* info)
{
    printf("test_callback %d\n", *(int*)&info );
}

timer_t tmr_alrm_create(calipso_client_t *client, int sec)
{
    tmr_thread_arg *proc_arg = malloc(sizeof(tmr_thread_arg));

    //if(!proc_arg )
    //  return 0;

    proc_arg->interval = 1;
    proc_arg->waits = 1000L * sec;
    proc_arg->callback = signal_handler;
    proc_arg->info = client;

    if (!tmr_init_proc(proc_arg, tmr_thread_exec_and_wait))
        printf("Fail to init proc \n");

    return proc_arg->cond;
}

void tmr_alrm_reset(calipso_client_t *client, int sec)
{
    tmr_alrm_kill(client->ctmr);

    client->ctmr = tmr_alrm_create(client, sec);
}

void tmr_alrm_kill(timer_t timerid)
{
    pthread_cond_signal(&timerid);
}

#if 0
int main(int argc, char** argv)
{
    tmr_thread_arg proc_arg;

    proc_arg.interval = 1;
    proc_arg.waits = 1000L;
    proc_arg.callback = test_callback;
    proc_arg.info = (void*)0x1;
    if (!tmr_init_proc(&proc_arg, tmr_thread_exec_and_wait))
        printf("Fail to init proc \n");
    sleep(1);
//   pthread_mutex_lock(&proc_arg.lock);
//   pthread_cond_signal(&proc_arg.cond);
//   pthread_mutex_unlock(&proc_arg.lock);
    while (1) {
        sleep(1);
    }

    sleep(4);
    //pthread_join(proc_a.thread, (void*) &result_a);
    return (EXIT_SUCCESS);
}
#endif
