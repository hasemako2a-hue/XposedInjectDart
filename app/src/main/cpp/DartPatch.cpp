
#include <android/log.h>
#include <link.h>
#include <unistd.h>
#include <pthread.h>

#include <cstdint>
#include <cstring>
#include <atomic>
#include <climits>

#include "dobby.h"

#define TAG "DartPatch"

#define LOGI(...) \
    __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

#define LOGE(...) \
    __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

// Blutterで取得した命令オフセット
static constexpr uintptr_t PATCH_OFFSET = 0x8bd330;

// 元の命令: b.le #0x8bd340
static constexpr uint32_t EXPECTED_INSTRUCTION = 0x5400008D;

// ARM64 NOP
static constexpr uint32_t NOP_INSTRUCTION = 0xD503201F;

static std::atomic<bool> started{false};


// libapp.so のベースアドレスを取得
struct ModuleInfo {
    uintptr_t base = 0;
};

static int FindModule(
    dl_phdr_info* info,
    size_t size,
    void* data)
{
    auto* result = static_cast<ModuleInfo*>(data);

    if (!info->dlpi_name)
        return 0;

    const char* name = strrchr(info->dlpi_name, '/');
    name = name ? name + 1 : info->dlpi_name;

    if (strcmp(name, "libapp.so") != 0)
        return 0;

    uintptr_t min_vaddr = UINTPTR_MAX;

    long page_size = sysconf(_SC_PAGESIZE);

    for (int i = 0; i < info->dlpi_phnum; i++) {

        const auto& phdr = info->dlpi_phdr[i];

        if (phdr.p_type != PT_LOAD)
            continue;

        uintptr_t vaddr =
            static_cast<uintptr_t>(phdr.p_vaddr);

        vaddr &= ~(static_cast<uintptr_t>(page_size) - 1);

        if (vaddr < min_vaddr)
            min_vaddr = vaddr;
    }

    if (min_vaddr == UINTPTR_MAX)
        return 0;

    result->base =
        static_cast<uintptr_t>(info->dlpi_addr) + min_vaddr;

    return 1;
}


static uintptr_t GetLibAppBase()
{
    ModuleInfo info{};

    dl_iterate_phdr(FindModule, &info);

    return info.base;
}


// NOPパッチ
static bool ApplyPatch(uintptr_t base)
{
    uintptr_t target = base + PATCH_OFFSET;

    uint32_t original = 0;

    memcpy(
        &original,
        reinterpret_cast<void*>(target),
        sizeof(original)
    );

    LOGI("libapp base: %p", reinterpret_cast<void*>(base));
    LOGI("patch address: %p", reinterpret_cast<void*>(target));
    LOGI("original instruction: 0x%08X", original);

    // 想定した命令以外は書き換えない
    if (original != EXPECTED_INSTRUCTION) {
        LOGE("Unexpected instruction. Aborted.");
        return false;
    }

    uint32_t nop = NOP_INSTRUCTION;

    auto result = DobbyCodePatch(
        reinterpret_cast<void*>(target),
        reinterpret_cast<uint8_t*>(&nop),
        sizeof(nop)
    );

    if (result != kMemoryOperationSuccess) {
        LOGE("DobbyCodePatch failed: %d", result);
        return false;
    }

    __builtin___clear_cache(
        reinterpret_cast<char*>(target),
        reinterpret_cast<char*>(target + 4)
    );

    uint32_t verify = 0;

    memcpy(
        &verify,
        reinterpret_cast<void*>(target),
        sizeof(verify)
    );

    LOGI("patched instruction: 0x%08X", verify);

    return verify == NOP_INSTRUCTION;
}


// libapp.so のロード待機
static void* PatchThread(void*)
{
    LOGI("Waiting for libapp.so...");

    for (int i = 0; i < 1500; i++) {

        uintptr_t base = GetLibAppBase();

        if (base != 0) {

            LOGI("libapp.so detected");

            if (ApplyPatch(base)) {
                LOGI("Patch completed");
            } else {
                LOGE("Patch failed");
            }

            return nullptr;
        }

        usleep(20000);
    }

    LOGE("libapp.so detection timeout");

    return nullptr;
}


// JNI_OnLoad から呼び出す
extern "C"
void StartDartPatchThread()
{
    if (started.exchange(true))
        return;

    pthread_t thread;

    int result = pthread_create(
        &thread,
        nullptr,
        PatchThread,
        nullptr
    );

    if (result != 0) {
        LOGE("pthread_create failed: %d", result);
        started.store(false);
        return;
    }

    pthread_detach(thread);

    LOGI("Patch thread started");
}
