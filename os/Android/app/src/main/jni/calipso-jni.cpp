/* calipso-jni.cpp - exports
 *
 * Copyright (C) 2014 Borislav Sapundzhiev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */

#include <string.h>
#include <jni.h>
///
/// http://www3.ntu.edu.sg/home/ehchua/programming/java/JavaNativeInterface.html
///
extern "C" {
#include "calipso.h"
#include "compat.h"


JNIEXPORT jstring JNICALL
Java_com_bsapundzhiev_calipso_CalipsoJNIWrapper_getCurrentWorkingDir
	(JNIEnv *env, jobject obj)
{
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    return env->NewStringUTF(cwd);
}

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

    TRACE("Init server %s\n", path );
    calipso_init();
#ifdef USE_SSL
    calipso_ssl_init();
#endif
    /* init coniguration */
    config = calipso_config_alloc();
    TRACE("Log config\n");
    config_parse_file(config, path);
    calipso_set_config(config);
    /* init signal handlers */
    TRACE("init signal handlers\n" );
    //calipso_init_all_signal_handlers();

    /* loading modules */
    TRACE("loading modules\n" );
    calipso_modules_init(config, CPO_OK);
    /* HOOKS init all modules */
    TRACE("HOOKS init all modules\n" );
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
    config_unalloc(calipso->config);
    calipso->config = NULL;
    //env->ReleaseStringUTFChars(jstr, cstr);
    TRACE("main loop\n");
    cpo_events_loop();
}

}