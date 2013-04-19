
#include "calipso.h"
#include "_process.h"

calipso_process_t *
calipso_process_alloc(void)
{
    calipso_process_t *process = xmalloc(sizeof(calipso_process_t));

    process->pool = cpo_pool_create(CALIPSO_DEFAULT_POOLSIZE);
    process->proc_time = 0;
    process->pstate = PROCESS_STATE_NONE;

    return (process);
}

void
calipso_process_unalloc(calipso_process_t *process)
{
    cpo_pool_destroy(process->pool);
   
    free(process);
}

int
calipso_process_set_pid(calipso_process_t *process, pid_t pid)
{
    return ((process->pid = pid));
}

int
calipso_process_set_type(calipso_process_t *process, int8_t ptype)
{
    return ((process->ptype = ptype));
}

int
calipso_process_set_socket(calipso_process_t *process, int psocket)
{
    return ((process->psocket = psocket));
}

int
calipso_process_set_control_socket(calipso_process_t *process, int pcontrolsocket)
{
    return ((process->pcontrolsocket = pcontrolsocket));
}

int
calipso_process_clear_flags(calipso_process_t *process)
{
    return ((process->pflags = 0));
}

int
calipso_process_set_flags(calipso_process_t *process, int32_t flags)
{
    return ((process->pflags |= (flags)));
}

int
calipso_process_unset_flags(calipso_process_t *process, int32_t flags)
{
    return ((process->pflags &= ~(flags)));
}

int
calipso_process_set_state(calipso_process_t *process, int32_t state)
{
    return ((process->pstate = state));
}

int
calipso_process_clear_state(calipso_process_t *process)
{
    return ((process->pflags = 0));
}

int
calipso_process_unset_state(calipso_process_t *process, int32_t flags)
{
    return ((process->pstate &= ~(flags)));
}


calipso_pool_t *
calipso_process_get_pool(calipso_process_t *process)
{
    return (process->pool);
}

int
calipso_process_get_state(calipso_process_t *process)
{
    return (process->pstate);
}

int
calipso_process_set_time(calipso_process_t *process, time_t proc_time)
{
    return ((process->proc_time = proc_time)!=0);
}

time_t
calipso_process_get_time(calipso_process_t *process)
{
    return (process->proc_time);
}

int
calipso_process_get_socket(calipso_process_t *process)
{
    return (process->psocket);
}

int
calipso_process_get_control_socket(calipso_process_t *process)
{
    return (process->pcontrolsocket);
}

pid_t
calipso_process_get_pid(calipso_process_t *process)
{
    return (process->pid);
}
