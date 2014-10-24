#ifndef _CPO_LOG_H
#define _CPO_LOG_H


#define LOG_FILE_ACCESS "access.log"
#define LOG_FILE_ERROR "error.log"

enum log_type { 
	LOG_ACCESS 	= 0, 
	LOG_WARNING	= 1, 
	LOG_ERROR	= 2, 
	LOG_NOTICE	= 3,
};

typedef struct cpo_log {
	u_int level;
	char * file_access;
	char * file_error;
	FILE *fp_log_access;
	FILE *fp_log_error;
	/* chunk_buf_t */
} cpo_log_t;

cpo_log_t * 
cpo_log_alloc();

void 
cpo_log_unalloc(cpo_log_t *log);

//void 
//cpo_log_rotate_logfile(cpo_log_t *log, const char *logfilename);

int  
cpo_log_set_level(cpo_log_t *log, u_int level);

int  
cpo_log_open(cpo_log_t * log);

void 
cpo_log_close(cpo_log_t * log);

void 
cpo_log_print(cpo_log_t * log, int type ,const char *fmt,...);

#include "core.h"
void 
cpo_log_write_access_log(cpo_log_t * log, calipso_request_t * request);

/* Error log types */
#define cpo_log_error(log, x, ...)\
	cpo_log_print(log, LOG_ERROR, "%s() => " x, __FUNCTION__, ##__VA_ARGS__)

#define cpo_log_error_trace(log, x, ...)\
	cpo_log_print(log, LOG_ERROR, "%s:%d:%s() => " x, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#define cpo_log_warning(log, x, ...)\
	cpo_log_print(log, LOG_WARNING, "%s:%d:%s() => " x, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#define cpo_log_notice(log, x, ...)\
	cpo_log_print(log, LOG_NOTICE, "%s:%d:%s() => " x, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#endif

