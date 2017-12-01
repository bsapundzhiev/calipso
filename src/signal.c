/* signal.c - posix signal handlers
 *
 * Copyright (C) 2007 Borislav Sapundzhiev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

#include <signal.h>
#include "calipso.h"
#ifdef LINUX
#include <execinfo.h>
#endif
static void handle_sigterm(int sig);
static void handle_sigchld(int sig);
static void handle_sigsegv(int sig);
/**
 * set all signal handlers
 *TODO: error handling, fixes
 */
int calipso_init_all_signal_handlers()
{
    signal( SIGTERM, handle_sigterm);
    signal( SIGINT, handle_sigterm);
    signal( SIGSEGV, handle_sigsegv);

#ifndef _WIN32
    signal( SIGPIPE, SIG_IGN);
    signal( SIGHUP, handle_sigterm);
    signal( SIGUSR1, handle_sigterm);
    signal( SIGCHLD, handle_sigchld);
#endif

    return 0;
}

/**
 * SIGTERM handler
 */
static void handle_sigterm(int sig)
{
    TRACE("exit -> got SIG(%d) => server terminated\n", sig);
    calipso_set_exit_status();
}

/**
 * CHILD
 */
static void handle_sigchld(int sig)
{
    pid_t pid;
    int status;
//#ifdef _DEBUG
    TRACE("handle_sigchld SIG(%d)\n", sig);
//#endif
    for (;;) {

#ifdef _WIN32
        pid = _cwait(&status, -1, WAIT_CHILD);
#else
        pid = waitpid((pid_t) -1, &status, WNOHANG);
#endif
        /*none left*/
        if (pid == 0)
            break;

        if (pid < 0) {
            /*because of ptrace*/
            if (errno == EINTR)
                continue;
            break;
        }
    }
}

/**
 * sigfault handler
 */
static void handle_sigsegv(int sig)
{
#ifdef LINUX
    void *array[10];
    size_t size;
    size = backtrace(array, 10);
#endif
    printf("Oops, SIGSEGV: get last error %d -> '%s'\n", errno,
           strerror(errno));
    cpo_log_error(calipso->log, "Oops, SIGSEGV(%d): get last error %d -> '%s'\n",
                  sig, errno, strerror(errno));
#ifdef LINUX
    backtrace_symbols_fd(array, size, fileno(calipso->log->fp_log_error));
#endif
    exit(1);
}

