#ifdef IOS_PORT
// iOS: entry point está em ios_entry.cpp
#else
#include <jni.h>
#include "hooks.h"
extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) return -1;
    InitHooks();
    return JNI_VERSION_1_6;
}
#endif
