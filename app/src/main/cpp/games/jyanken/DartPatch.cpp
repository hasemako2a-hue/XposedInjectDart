
#include <cstdint>
#include <unistd.h>
#include <link.h>
#include <cstring>
#include <android/log.h>
#include <pthread.h>
#include <dobby.h>

#define TAG "WordPuzzlePatch"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

static constexpr uint32_t NOP_INSTRUCTION = 0xD503201F;

static constexpr uintptr_t PATCH_OFFSETS[] = {
    0x8bd330,
    0x8bd380,
    0x8bd3dc
};

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

            long pageSize = sysconf(_SC_PAGESIZE);
            if (pageSize <= 0)
                return 0;

            uintptr_t minVaddr = UINTPTR_MAX;

            for (int i = 0; i < info->dlpi_phnum; i++) {
                const ElfW(Phdr)& phdr = info->dlpi_phdr[i];

                if (phdr.p_type == PT_LOAD &&
                    phdr.p_vaddr < minVaddr) {
                    minVaddr = phdr.p_vaddr;
                }
            }

            if (minVaddr == UINTPTR_MAX)
                return 0;

            uintptr_t alignedVaddr =
                minVaddr & ~(static_cast<uintptr_t>(pageSize) - 1);

            ctx->base = info->dlpi_addr + alignedVaddr;

            return 1;
        },
        &context
    );

    return context.base;
}

static bool PatchNop(uintptr_t address, uintptr_t offset) {
    uint32_t nop = NOP_INSTRUCTION;

    uint32_t original =
        *reinterpret_cast<volatile uint32_t*>(address);

    LOGI("Offset: 0x%lx, original: 0x%08X",
         static_cast<unsigned long>(offset), original);

    MemoryOperationError result = DobbyCodePatch(
        reinterpret_cast<void*>(address),
        reinterpret_cast<uint8_t*>(&nop),
        sizeof(nop)
    );

    if (result != kMemoryOperationSuccess) {
        LOGE("Patch failed at offset 0x%lx: %d",
             static_cast<unsigned long>(offset),
             static_cast<int>(result));
        return false;
    }

    uint32_t patched =
        *reinterpret_cast<volatile uint32_t*>(address);

    if (patched != NOP_INSTRUCTION) {
        LOGE("Verification failed at offset 0x%lx: 0x%08X",
             static_cast<unsigned long>(offset), patched);
        return false;
    }

    LOGI("NOP patch succeeded at offset 0x%lx",
         static_cast<unsigned long>(offset));

    return true;
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

    LOGI("libapp base: %p", reinterpret_cast<void*>(base));

    constexpr size_t count =
        sizeof(PATCH_OFFSETS) / sizeof(PATCH_OFFSETS[0]);

    int successCount = 0;

    for (size_t i = 0; i < count; i++) {
        uintptr_t offset = PATCH_OFFSETS[i];
        uintptr_t target = base + offset;

        if (PatchNop(target, offset)) {
            successCount++;
        }
    }

    LOGI("Patch completed: %d/%zu",
         successCount, count);
}

extern "C" void StartDartPatchThread() {
    pthread_t thread;

    if (pthread_create(
            &thread,
            nullptr,
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
