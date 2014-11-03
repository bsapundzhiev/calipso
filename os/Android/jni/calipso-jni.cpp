#include <string.h>
#include <jni.h>

extern "C" {

      JNIEXPORT jstring JNICALL
      Java_com_example_Hellojnicpp_stringFromJNI
      (JNIEnv *env, jobject obj)
      {
            return env->NewStringUTF("Hello from C++ over JNI!");
      }
}