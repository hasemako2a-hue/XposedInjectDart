#include <jni.h>

extern "C" void StartGamePatchThread();

extern "C" JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM*, void*) {
    StartGamePatchThread();
    return JNI_VERSION_1_6;
}
