/* main.c - init
 *
 * Copyright (C) 2007 Borislav Sapundzhiev
 *         
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

#include "calipso.h"
#ifdef USE_SSL
#include "cpo_io_ssl.h"
#endif

#ifdef _WIN32
#include "getopt.h"
#define CALIPSO_DEFAULT_CONFIG "calipso.conf"
#else
#define CALIPSO_DEFAULT_CONFIG "../doc/calipso.conf"
#endif

#define CALIPSO_PID_FILE	"/var/run/calipso.pid"

static void usage(int);
static void calipso_suppress_stdio(void);
static int calipso_setup_server_user(const char *user); 
static int calipso_write_pidfile(pid_t pid);
static void print_info();

int main (int argc, char **argv)
{
    int opt, i;
    /*load all modules true by default*/
    int load_mod_flag = 1;
    int deamonize = 0;
	int check_config = 0;
	int show_info = 0;
    const char *configfile = CALIPSO_DEFAULT_CONFIG;
    calipso_config_t *config = NULL;
	const char * user = NULL;

    while ((opt = getopt(argc, argv, "f:dMCSlhvi?")) != -1) {

        switch (opt) {
        case 'c':
        case 'f':
            	configfile = optarg;
            break;
        case 'd':
            	fprintf(stderr, "starting calipso httpd...");
            	deamonize = 1;
            break;
        case 'M':
            	fprintf(stderr, "loaded modules:\n");
            	load_mod_flag = 0;
            break;
        case 'v':
            	usage(1);
            break;
		case 'C':
				check_config = 1;
			break;
		case 'S':
				calipso_suppress_stdio();
			break;
		case 'i':
				show_info = 1;
			break;
        case 'h':
        case '?':
        default:
            usage(0);
        }
    }

    /* init server */
    calipso_init();
#ifdef USE_SSL
	calipso_ssl_init();
#endif
    /* init coniguration */
    config = calipso_config_alloc();    
    config_parse_file( config , configfile );
    
	if(check_config) {
		config_list_dump(config);
		exit(0);
	}

    calipso_set_config(config);

    /* init signal handlers */
    calipso_init_all_signal_handlers();

    /* loading modules */
    calipso_modules_init(config, load_mod_flag);
    /* debug hooks */
    if (show_info) {
		print_info();
        for (i = 0; i < NR_HOOKS; i++ )
        	calipso_hook_dump(i);
		calipso_destroy();
        exit(0);
    }
	
    /* HOOKS init all modules */
    calipso_trigger_hook(HOOK_INIT);
    calipso_trigger_hook(HOOK_CONFIGURE);

    if (deamonize) {

        if (daemon(0, 1) < 0) {
            TRACE("daemon() call failure.");
			exit(EXIT_FAILURE);
		}
        else {
           calipso_write_pidfile( getpid() );
		}

        calipso_suppress_stdio();
    }

	user = config_get_option( config, "user", NULL);
	if(user) {
		calipso_setup_server_user(user);
	}

    /* init events */
	cpo_events_init();
	/* config should not be used anymore*/
	config_unalloc(calipso->config);
	calipso->config = NULL;
	/* main loop */
	cpo_events_loop();

    return 0;
}

void calipso_suppress_stdio()
{
    /* close stdin and stdout, as they are not needed */
    /* move stdin to /dev/null */
    int fd;
    if (-1 != (fd = open(DEVNULL, O_RDONLY))) {
        close(STDIN_FILENO);
        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    /* move stdout to /dev/null */
    if (-1 != (fd = open(DEVNULL, O_WRONLY))) {
        close(STDOUT_FILENO);
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

	fflush(stdin);
	fflush(stdout);
}

int calipso_setup_server_user(const char *user)
{
#ifndef _WIN32
	struct passwd *pw;
	/*
	 * XXX: Valgrind shows memory leak on getpwnam 
	 */
	pw = getpwnam( user );
	if (pw == NULL) {
		printf("Error: Unknown username '%s'\n", user);	
		exit(EXIT_FAILURE);
	}

	if ( setuid( pw->pw_uid ) != 0 ) {
		perror("setuid()");
		if(EPERM == errno) {
			printf("continue with current uid %d\n", getuid());
		}
		else exit(EXIT_FAILURE);
	} else { 
		printf("server sarted as user: %s UID: %d == %d (getuid %d)\n", 
				pw->pw_name , pw->pw_uid, geteuid() , getuid());
	}
#endif
	return CPO_OK;
}

int calipso_write_pidfile(pid_t pid)
{
	FILE *fp = fopen(CALIPSO_PID_FILE, "w+");
	if(fp) {		
		fprintf(fp, "%d", pid);
		fclose(fp);
		return CPO_OK;
	}
	
	return CPO_ERR;
}

static void print_info()
{	
	printf("CALIPSO_DEFAULT_POOLSIZE %d\n", CALIPSO_DEFAULT_POOLSIZE);
	printf("INPUTBUFSZ: %d pagesize %d\n", INPUTBUFSZ, getpagesize());
	printf("listener hash table(size= %d buckets)...\n",
          hash_table_get_size(calipso->listeners) );
  	printf("m_handler hash table(size= %d buckets)...\n",
           hash_table_get_size(calipso->m_handler) );
    printf("calipso hooks = %d \n", calipso_hook_get_NR_HOOKS());
}

void usage(int version)
{
    if (version) {
        printf(	"Calipso - lightweight http server (%s, Ver:%s, Date:%s)\n", OS, VERSION, __DATE__);
        printf(	"Autor: (c) 2007,2013 Borislav Sapundzhiev <BSapundjiev@gmail.com>\n");
        printf( "License:This program is released under the terms of the GPLv2\n\n");
        exit(EXIT_FAILURE);
    }

    printf("\nUsage:\tcalipso [-f <file> -CMdHSh]\n");
    printf(	"\nOptions:\n"
            "\t-f <config> - configuration file\n"
			"\t-C - check config file for errors\n"
            "\t-M - list loaded modules\n"
            "\t-d - service mode (deamon)\n"
            "\t-i - show core values and\n"
			"\t-S - disable console output\n"
            "\t-v - version\n"
            "\t-h - this page\n"
            "\n");

    exit(EXIT_FAILURE);
}

