#include "calipso.h"
#include "timer.h"

static void CALLBACK TimerProc(void* lpParametar, BOOLEAN TimerOrWaitFired)
{
    calipso_client_t * client = (calipso_client_t *) lpParametar;
    TRACE("signal_handler client @ %p : %d\n", client, client->csocket);
    if(client) {
        //calipso_client_disconnect(client);
        shutdown(client->csocket, SHUT_RDWR);
    }
}

timer_t tmr_alrm_create(calipso_client_t *client, int sec)
{
    timer_t timerHandle = 0;
    BOOL success = CreateTimerQueueTimer(
                       &timerHandle,
                       NULL,
                       TimerProc,
                       client,
                       (sec * 1000),
                       0,
                       WT_EXECUTEINTIMERTHREAD | WT_EXECUTEONLYONCE);
    return timerHandle;
}

void tmr_alrm_kill(timer_t timerid)
{
    DeleteTimerQueueTimer(NULL, timerid, NULL);
    //CloseHandle (timerid);
}

void tmr_alrm_reset(calipso_client_t *client, int sec)
{
    tmr_alrm_kill(client->ctmr);
    client->ctmr = tmr_alrm_create(client, sec);
}
