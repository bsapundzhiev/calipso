#ifndef _PROCESS_H
#define _PROCESS_H


#define	PROCESS_TYPE_MPROCESS	0x1
#define	PROCESS_TYPE_LPROCESS	0x2
#define	PROCESS_TYPE_EPROCESS	0x4
#define	PROCESS_TYPE_NPROCESS	0x8
#define	PROCESS_TYPE_CGI	0x10


#define	PROCESS_STATE_NONE	0x0
#define	PROCESS_STATE_RUNNING	0x1
#define	PROCESS_STATE_STOPPED	0x2
#define	PROCESS_STATE_INACTIVE	0x4



calipso_process_t *
calipso_process_alloc(void);

int
calipso_process_set_pid(calipso_process_t *process, pid_t pid);

int
calipso_process_set_type(calipso_process_t *process, int8_t ptype);

int
calipso_process_clear_flags(calipso_process_t *process);

int
calipso_process_set_state(calipso_process_t *process, int32_t state);


#endif /*!_PROCESS_H*/

