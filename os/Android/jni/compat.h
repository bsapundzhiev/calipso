#ifndef __COMPATH_H
#define __COMPATH_H

#define OS 	"Android"//__ANDROID_API__

#define USE_POLL	1
#define USE_EPOLL	1

#include <android/log.h>
//#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,"Calipso",__VA_ARGS__)
#define  TRACE(...)  __android_log_print(ANDROID_LOG_DEBUG,"Calipso",__VA_ARGS__)

int clearenv();

#endif