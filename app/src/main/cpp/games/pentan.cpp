#include "../common/PatchCommon.h"

#include <dobby.h>
#include <android/log.h>
#include <cstdint>
#include <cstring>
#include <jni.h>
#include <atomic>

namespace {

constexpr char kModuleName[] = "libapp.so";
constexpr char kLogTag[] = "Vec2Hook";
constexpr uintptr_t kHookOffset = 0x555988;

// Java側から更新する座標
std::atomic<float> g_x{-20.0f};
std::atomic<float> g_y{250.0f};

void OnVec2Hook(void* address, DobbyRegisterContext* ctx) {
#if defined(__aarch64__)

    // Frida: this.context.x2
    const uintptr_t x2 =
        static_cast<uintptr_t>(ctx->general.regs.x2);

    const uintptr_t vec2Ptr = x2 - 1;

    const uintptr_t heapBase =
        x2 & 0xffffffff00000000ULL;

    uint32_t compressedStorage = 0;

    std::memcpy(
        &compressedStorage,
        reinterpret_cast<const void*>(vec2Ptr + 0x08),
        sizeof(compressedStorage)
    );

    const uintptr_t storagePtr =
        (heapBase | static_cast<uintptr_t>(compressedStorage)) - 1;

    const uintptr_t xAddr = storagePtr + 0x18;
    const uintptr_t yAddr = storagePtr + 0x1C;

    const float x = g_x.load(std::memory_order_relaxed);
    const float y = g_y.load(std::memory_order_relaxed);

    std::memcpy(reinterpret_cast<void*>(xAddr), &x, sizeof(x));
    std::memcpy(reinterpret_cast<void*>(yAddr), &y, sizeof(y));

#endif
}

void* PatchThread(void*) {
    uintptr_t base = 0;
    if (!PatchCommon::WaitForModuleBase(
            kModuleName, &base, kLogTag)) {
        return nullptr;
    }
    void* target = reinterpret_cast<void*>(base + kHookOffset);

    const int result = DobbyInstrument(target, OnVec2Hook);

    __android_log_print(
        ANDROID_LOG_INFO,
        kLogTag,
        "DobbyInstrument: target=%p result=%d",
        target,
        result
    );

    return nullptr;
}
} // namespace

extern "C" JNIEXPORT void JNICALL
Java_jp_mack_xposedinjectdart_Inject_nativeSetCoordinates(
    JNIEnv*,
    jclass,
    jfloat x,
    jfloat y
) {
    g_x.store(x, std::memory_order_relaxed);
    g_y.store(y, std::memory_order_relaxed);
}

extern "C" void StartGamePatchThread() {
    PatchCommon::StartDetachedThread(PatchThread, nullptr, kLogTag);
}