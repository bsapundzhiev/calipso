#include <string.h>
#include <jni.h>
//
//https://code.google.com/p/awesomeguy/wiki/JNITutorial
//
extern "C" {
#include "calipso.h"
#include "compat.h"

JNIEXPORT jstring JNICALL
Java_com_bsapundzhiev_calipso_CalipsoJNIWrapper_stringFromJNI
	(JNIEnv *env, jobject obj)
{	
	return env->NewStringUTF("Calipso is running");
}

JNIEXPORT void JNICALL
Java_com_bsapundzhiev_calipso_CalipsoJNIWrapper_startCalipsoServer
    (JNIEnv *env, jobject obj, jstring filePath)
{
    calipso_config_t *config = NULL;
    const char * path = env->GetStringUTFChars(filePath , 0 ) ;

    TRACE( "init server %s\n", path );
    calipso_init();
#ifdef USE_SSL
    calipso_ssl_init();
#endif
    /* init coniguration */
    config = calipso_config_alloc();
    TRACE( "Log config\n" );
    config_parse_file(config, path);
    calipso_set_config(config);
    TRACE( "end config\n" );

    /* init signal handlers */
    TRACE( "init signal handlers\n" );
    //calipso_init_all_signal_handlers();

    /* loading modules */
    TRACE( "loading modules\n" );
    calipso_modules_init(config, CPO_OK);
    /* HOOKS init all modules */
    TRACE( "HOOKS init all modules\n" );
    calipso_trigger_hook(HOOK_INIT);
    calipso_trigger_hook(HOOK_CONFIGURE);

    //calipso_write_pidfile(getpid());
    //user = config_get_option(config, "user", NULL);
    //if (user) {
    //    calipso_setup_server_user(user);
    //}

    TRACE("cpo_events_init\n");
    cpo_events_init();
    /* config should not be used anymore*/
    //config_unalloc(calipso->config);
    //env->ReleaseStringUTFChars(jstr, cstr); 
    //calipso->config = NULL;
    TRACE("main loop\n");
    cpo_events_loop();
}

}