#include "calipso.h"
#include "timer.h"
#include "synchapi.h"

#define _SECOND 10000000

VOID CALLBACK TimerFinished(LPVOID lpArg, DWORD dwTimerLowValue, DWORD dwTimerHighValue )  
{
	calipso_client_t * client = (calipso_client_t *) lpArg;
	//TRACE("signal_handler client @ %p : %d\n", client, client->csocket);
	if(client) {
		//calipso_client_disconnect(client);
		shutdown(client->csocket, SHUT_RDWR);
	}
}

timer_t tmr_alrm_create(calipso_client_t *client, int sec)
{
	timer_t timerHandle = (HANDLE)OpenWaitableTimerW(NULL, FALSE, L"CalipsoTimer");
	__int64 qwDueTime= (sec * _SECOND) * -1;
    LARGE_INTEGER   liDueTime;

    // Copy the relative time into a LARGE_INTEGER.
    liDueTime.LowPart  = (DWORD) ( qwDueTime & 0xFFFFFFFF );
    liDueTime.HighPart = (LONG)  ( qwDueTime >> 32 );

    SetWaitableTimer(
			 timerHandle,
             &liDueTime,
			 0, // signaled once
             TimerFinished,
			 client,
			 FALSE );

	return timerHandle;
}

void tmr_alrm_kill(timer_t timerid)
{
	CancelWaitableTimer(timerid);
}

void tmr_alrm_reset(calipso_client_t *client, int sec)
{
	tmr_alrm_kill(client->ctmr);
	client->ctmr = tmr_alrm_create(client, sec);
}
