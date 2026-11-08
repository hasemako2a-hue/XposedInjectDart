
#include <cstdint>
#include <cstring>
#include <unistd.h>
#include <link.h>
#include <android/log.h>
#include <pthread.h>
#include <dobby.h>

#define TAG "DartPatch"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

static constexpr uintptr_t ORIGIN_OFFSET = 0x9332e8;
static constexpr uintptr_t TARGET_OFFSET = 0x4fbbb0;

// ARM64 B imm26 を生成する
static bool EncodeBranch(uintptr_t origin, uintptr_t target,
                         uint32_t* instruction) {
    // B命令は4バイト境界の相対分岐
    int64_t delta = static_cast<int64_t>(target) -
                    static_cast<int64_t>(origin);

    if ((delta & 3) != 0) {
        LOGE("Branch target is not 4-byte aligned");
        return false;
    }

    // B命令の到達範囲は約 +/-128 MiB
    int64_t imm26 = delta >> 2;

    if (imm26 < -(1LL << 25) || imm26 >= (1LL << 25)) {
        LOGE("Branch target is out of range");
        return false;
    }

    *instruction = 0x14000000 |
                   (static_cast<uint32_t>(imm26) & 0x03FFFFFF);

    return true;
}

static uintptr_t FindLibappBase() {
    struct Context {
        uintptr_t base = 0;
    } context;

    dl_iterate_phdr(
        [](struct dl_phdr_info* info, size_t, void* data) -> int {
            if (!info->dlpi_name)
                return 0;

            const char* name = strrchr(info->dlpi_name, '/');
            name = name ? name + 1 : info->dlpi_name;

            if (strcmp(name, "libapp.so") != 0)
                return 0;

            auto* ctx = static_cast<Context*>(data);

            uintptr_t min_vaddr = UINTPTR_MAX;
            long page_size = sysconf(_SC_PAGESIZE);

            if (page_size <= 0)
                return 0;

            for (int i = 0; i < info->dlpi_phnum; i++) {
                const ElfW(Phdr)& phdr = info->dlpi_phdr[i];

                if (phdr.p_type == PT_LOAD &&
                    phdr.p_vaddr < min_vaddr) {
                    min_vaddr = phdr.p_vaddr;
                }
            }

            if (min_vaddr == UINTPTR_MAX)
                return 0;

            uintptr_t aligned_vaddr =
                min_vaddr & ~(static_cast<uintptr_t>(page_size) - 1);

            ctx->base = info->dlpi_addr + aligned_vaddr;
            return 1;
        },
        &context
    );

    return context.base;
}

static void PatchThread() {
    uintptr_t base = 0;

    for (int i = 0; i < 500; i++) {
        base = FindLibappBase();

        if (base)
            break;

        usleep(20000);
    }

    if (!base) {
        LOGE("libapp.so not found");
        return;
    }

    uintptr_t origin = base + ORIGIN_OFFSET;
    uintptr_t target = base + TARGET_OFFSET;

    LOGI("libapp base: %p", reinterpret_cast<void*>(base));
    LOGI("origin: %p", reinterpret_cast<void*>(origin));
    LOGI("target: %p", reinterpret_cast<void*>(target));

    uint32_t original = *reinterpret_cast<volatile uint32_t*>(origin);

    LOGI("original instruction: 0x%08X", original);

    uint32_t branch = 0;

    if (!EncodeBranch(origin, target, &branch)) {
        return;
    }

    LOGI("new B instruction: 0x%08X", branch);

    MemoryOperationError result = DobbyCodePatch(
        reinterpret_cast<void*>(origin),
        reinterpret_cast<uint8_t*>(&branch),
        sizeof(branch)
    );

    if (result != kMemoryOperationSuccess) {
        LOGE("DobbyCodePatch failed: %d", static_cast<int>(result));
        return;
    }

    uint32_t patched = *reinterpret_cast<volatile uint32_t*>(origin);

    LOGI("patched instruction: 0x%08X", patched);

    if (patched != branch) {
        LOGE("Patch verification failed");
        return;
    }

    LOGI("Branch patch succeeded");
}

extern "C" void StartDartPatchThread() {
    pthread_t thread;

    if (pthread_create(&thread, nullptr,
                       [](void*) -> void* {
                           PatchThread();
                           return nullptr;
                       },
                       nullptr) == 0) {
        pthread_detach(thread);
        LOGI("Patch thread started");
    } else {
        LOGE("Failed to create patch thread");
    }
}
