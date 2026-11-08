#include <jni.h>
#include <android/log.h>

#define TAG "multiplication"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

extern "C" void StartDartPatchThread();

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGI("multiplication library loaded");

    StartDartPatchThread();

    return JNI_VERSION_1_6;
}