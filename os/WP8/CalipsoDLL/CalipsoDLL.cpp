// CalipsoDLL.cpp : Defines the exported functions for the DLL application.
//

#include <windows.h>
#include <Processthreadsapi.h>
#include "pch.h"
#include "CalipsoDLL.h"

extern "C" {

#include "calipso.h"
#include "timer.h"

}

#define CALIPSO_DEFAULT_CONFIG "calipso.conf"

int calipso_main() 
{
    /*load all modules true by default*/
    int load_mod_flag = 1;
    calipso_config_t *config = NULL;

	/* init server */
    calipso_init();
#ifdef USE_SSL
	calipso_ssl_init();
#endif

    /* init coniguration */
	TRACE("Calipso init...\n");
    config = calipso_config_alloc();    
    config_parse_file( config ,  CALIPSO_DEFAULT_CONFIG);
//#if 0
    calipso_set_config(config);

    /* init signal handlers */
    calipso_init_all_signal_handlers();

    /* loading modules */
    calipso_modules_init(config, load_mod_flag);
 
	
    /* HOOKS init all modules */
    calipso_trigger_hook(HOOK_INIT);
    calipso_trigger_hook(HOOK_CONFIGURE);
    //calipso_suppress_stdio();

    /* init events */
	cpo_events_init();
	/* config should not be used anymore*/
	config_unalloc(calipso->config);
	calipso->config = NULL;
	/* main loop */
	//cpo_events_loop();
//#endif
	return 0;
}

void calipso_start_thread() 
{
	cpo_events_loop();
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
