/* calpso log util
 *
 * Copyright (C) 2012 Borislav Sapundzhiev
 *
 * This file is released under the terms of GPL v2 and any later version.
 *
 */

#include "calipso.h"
#include "cplib.h"
#include "cpo_log.h"
#include <stdarg.h>

#define LOG_ROTATE_SIZE_LIMIT 	((1024 * 1024) * 5)
#define LOG_ROTATE_INC_LIMIT 	100

/* error log: [_date_] [error] [client _IP_] MSG: path */

static const char *log_types[] = { "access", " warn", "error", " note" };

cpo_log_t * cpo_log_alloc()
{
    cpo_log_t *log = malloc(sizeof(cpo_log_t));
    if (log == NULL)
        return NULL;

    log->fp_log_access = log->fp_log_error = NULL;

    log->level = LOG_ERROR;

    return log;
}

void cpo_log_unalloc(cpo_log_t *log)
{
    cpo_log_close(log);
    free(log);
}

int cpo_log_set_level(cpo_log_t *log, u_int level)
{
    return (log->level = level) != 0;
}

static int make_log_date(char *buf, int len, time_t t)
{
    struct tm * tm = NULL;
    if (t == 0) {
        time_t tt = time(NULL);
        tm = localtime(&tt);
    } else {
        tm = localtime(&t);
    }

    return strftime(buf, len - 1, "%d/%b/%Y:%H:%M:%S %z", tm);
}

static void cpo_log_rotate_logfile(cpo_log_t *log, const char *logfilename)
{
    u_int i;
    struct stat st;
    int override = 0;

    if (stat(logfilename, &st) == 0 && st.st_size > LOG_ROTATE_SIZE_LIMIT) {

        for (i = 1; i <= LOG_ROTATE_INC_LIMIT; i++) {
            char newname[64];
            override = 0;
            if (i == LOG_ROTATE_INC_LIMIT) {
                override = 1;
            }
            cpo_snprintf(newname, sizeof(newname), "%s.%d", logfilename, i);

            if (access(newname, F_OK) == -1 || override) {
                rename(logfilename, newname);
                break;
            }
        }
    }
}

int cpo_log_open(cpo_log_t * log)
{
    /* rotate old log files */
    cpo_log_rotate_logfile(log, LOG_FILE_ACCESS);
    cpo_log_rotate_logfile(log, LOG_FILE_ERROR);

    log->fp_log_access = fopen(LOG_FILE_ACCESS, "a+");
    if (!log->fp_log_access) {
        perror("fopen");
        return CPO_ERR;
    }

    log->fp_log_error = fopen(LOG_FILE_ERROR, "a+");
    if (!log->fp_log_error) {
        perror("fopen");
        return CPO_ERR;
    }

    return CPO_OK;
}

void cpo_log_close(cpo_log_t * log)
{
    if (log->fp_log_access != NULL) {
        fclose(log->fp_log_access);
    }

    if (log->fp_log_error != NULL) {
        fclose(log->fp_log_error);
    }
}

static FILE *cpo_log_get_fp(cpo_log_t * log, int type)
{
    FILE *fp = NULL;
    if (LOG_ACCESS == type) {
        fp = log->fp_log_access;
    } else {
        fp = log->fp_log_error;
    }

    return fp;
}

void cpo_log_print(cpo_log_t * log, int type, const char *fmt, ...)
{
    va_list list;
    const char *p, *r;
    int e;
    long l;
    FILE *fp = NULL;
    long length;

    fp = cpo_log_get_fp(log, type);

    if (NULL == fp)
        return;

    fseek(fp, 0L, SEEK_END);
    length = ftell(fp);

    if (length > LOG_ROTATE_SIZE_LIMIT) {
        cpo_log_close(log);
        cpo_log_open(log);
        fp = cpo_log_get_fp(log, type);
    }

    if (LOG_ACCESS != type) {
        char date[32];
        make_log_date(date, sizeof(date), 0);
        fprintf(fp, "[%s] [%s] ", date, log_types[type]);
    }

    va_start(list, fmt);
    vfprintf(fp, fmt, list);

    /*for ( p = fmt ; *p ; ++p ) {

     if ( *p != '%' ) {
     fputc( *p, fp );
     continue;
     }

     switch ( *++p ) {
     case 's': {
     r = va_arg( list, char * );
     fprintf(fp,"%s", r);
     continue;
     }

     case 'u':
     case 'd': {
     e = va_arg( list, int );
     fprintf(fp,"%d", e);
     continue;
     }

     case 'l': {
     l = va_arg( list, long );
     fprintf(fp,"%lu", l);
     continue;
     }
     case 'x':
     case 'X': {
     e = va_arg( list, int );
     fprintf(fp,"%X", e);
     continue;
     }

     default:
     fputc( *p, fp );
     }
     }*/

    va_end(list);

    fputc('\n', fp);

    if (LOG_ACCESS != type) {
        fflush(log->fp_log_error);
        //fflush(log->fp_log_access);
    }
}

/* commont HTTP log format */
void cpo_log_write_access_log(cpo_log_t * log, calipso_request_t * request)
{
    char date[32] = { '\0' };
    const char *user_agent = NULL;
    const char *client_addr = NULL;

    if (request == NULL || log == NULL)
        return;

    user_agent = calipso_request_get_header_value(request, "User-Agent");
    if (!user_agent) {
        user_agent = "-";
    }

    if (request->client != NULL) {
        client_addr = calipso_client_remote_ip(request->client);
    }

    make_log_date(date, sizeof(date), request->request_time);

    cpo_log_print(log, LOG_ACCESS, "[%s] - %s [%s] \"%s %s %s\" %d %d %s",
                  client_addr ? client_addr : "-",
                  request->user ? request->user : "-", date, request->method,
                  request->uri, request->version, request->reply->status,
                  request->reply->bytes_sent, user_agent);
}

