/*
 * master process 
 */
#include "calipso.h"
#include "_process.h"

int
mprocess_init(void)
{
    calipso_process_t *proc = calipso_process_alloc();

    calipso_process_set_pid(proc, getpid());
    calipso_process_set_type(proc, PROCESS_TYPE_MPROCESS);
    calipso_process_clear_flags(proc);
    calipso_process_set_state(proc, PROCESS_STATE_RUNNING);

    calipso_set_mprocess(proc);
    calipso_set_current_process(proc);

    //calipso_process_set_time(proc, calipso_get_start_time());

    return (1);
}

int
mprocess(void)
{
    int (*mprocess_model)(void);

    mprocess_model = calipso_get_mprocess_model();
    return (mprocess_model());
}
