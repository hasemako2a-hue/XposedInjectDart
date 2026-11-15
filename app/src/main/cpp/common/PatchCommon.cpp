#include "PatchCommon.h"

#include <android/log.h>
#include <dobby.h>
#include <link.h>
#include <pthread.h>
#include <unistd.h>

#include <cstring>
#include <climits>
#include <cstdarg>

namespace PatchCommon {

namespace {
constexpr uint32_t kArm64Nop = 0xD503201F;

void LogI( const char* format, ...) {
    va_list args;
    va_start(args, format);
    __android_log_vprint(ANDROID_LOG_INFO, "DartPatch", format, args);
    va_end(args);
}

void LogE(const char* format, ...) {
    va_list args;
    va_start(args, format);
    __android_log_vprint(ANDROID_LOG_ERROR, "DartPatch", format, args);
    va_end(args);
}

struct FindContext {
    const char* wantedName;
    uintptr_t base;
};

int FindModuleCallback(dl_phdr_info* info, size_t, void* opaque) {
    if (!info->dlpi_name || !*info->dlpi_name) {
        return 0;
    }

    const char* basename = strrchr(info->dlpi_name, '/');
    basename = basename ? basename + 1 : info->dlpi_name;

    auto* context = static_cast<FindContext*>(opaque);
    if (strcmp(basename, context->wantedName) != 0) {
        return 0;
    }

    const long pageSize = sysconf(_SC_PAGESIZE);
    if (pageSize <= 0) {
        return 0;
    }

    uintptr_t minVaddr = UINTPTR_MAX;
    for (int i = 0; i < info->dlpi_phnum; ++i) {
        const ElfW(Phdr)& phdr = info->dlpi_phdr[i];
        if (phdr.p_type == PT_LOAD && phdr.p_vaddr < minVaddr) {
            minVaddr = phdr.p_vaddr;
        }
    }

    if (minVaddr == UINTPTR_MAX) {
        return 0;
    }

    const uintptr_t pageMask = static_cast<uintptr_t>(pageSize) - 1;
    const uintptr_t alignedVaddr = minVaddr & ~pageMask;

    // Frida の Module.base と同じく、最初の PT_LOAD のページ境界を基準にする。
    context->base = static_cast<uintptr_t>(info->dlpi_addr) + alignedVaddr;
    return 1;
}

bool IsAlignedInstructionAddress(uintptr_t address, const char* tag,
                                  uintptr_t offset) {
    if ((address & 0x3U) != 0) {
        LogE( "Unaligned instruction address: offset=0x%lx %s",
             static_cast<unsigned long>(offset),tag);
        return false;
    }
    return true;
}

bool WriteAndVerify(uintptr_t address, uint32_t replacement,
                    const char* tag, uintptr_t offset) {
    MemoryOperationError result = DobbyCodePatch(
        reinterpret_cast<void*>(address),
        reinterpret_cast<uint8_t*>(&replacement),
        sizeof(replacement)
    );

    if (result != kMemoryOperationSuccess) {
        LogE("DobbyCodePatch failed: offset=0x%lx result=%d %s",
             static_cast<unsigned long>(offset), static_cast<int>(result),tag);
        return false;
    }

    const uint32_t actual =
        *reinterpret_cast<volatile uint32_t*>(address);

    if (actual != replacement) {
        LogE("Verification failed: offset=0x%lx expected=0x%08X actual=0x%08X %s",
             static_cast<unsigned long>(offset), replacement, actual,tag);
        return false;
    }

    return true;
}

bool EncodeBranch(uintptr_t origin, uintptr_t target,
                  uint32_t* outInstruction, const char* tag) {
    const int64_t delta =
        static_cast<int64_t>(target) - static_cast<int64_t>(origin);

    if ((delta & 0x3) != 0) {
        LogE( "Branch target is not 4-byte aligned %s",tag);
        return false;
    }

    const int64_t imm26 = delta >> 2;
    if (imm26 < -(1LL << 25) || imm26 >= (1LL << 25)) {
        LogE("Branch target is out of B imm26 range %s",tag);
        return false;
    }

    *outInstruction =
        0x14000000U | (static_cast<uint32_t>(imm26) & 0x03FFFFFFU);
    return true;
}
} // namespace

bool WaitForModuleBase(const char* moduleName, uintptr_t* outBase,
                       const char* logTag, int attempts, useconds_t intervalUs) {
    if (!moduleName || !outBase || !logTag) {
        return false;
    }

    *outBase = 0;

    for (int i = 0; i < attempts; ++i) {
        FindContext context{moduleName, 0};
        dl_iterate_phdr(FindModuleCallback, &context);

        if (context.base != 0) {
            *outBase = context.base;
            LogI("%s base: %p %s", moduleName,
                 reinterpret_cast<void*>(context.base),logTag);
            return true;
        }

        usleep(intervalUs);
    }

    LogE("%s not found (attempts=%d) %s", moduleName, attempts,logTag);
    return false;
}

bool ApplyNop(uintptr_t moduleBase, uintptr_t offset, const char* logTag,
              bool checkExpected, uint32_t expectedInstruction) {
    const uintptr_t address = moduleBase + offset;

    if (!IsAlignedInstructionAddress(address, logTag, offset)) {
        return false;
    }

    const uint32_t original =
        *reinterpret_cast<volatile uint32_t*>(address);

    LogI("NOP offset=0x%lx original=0x%08X %s",
         static_cast<unsigned long>(offset), original,logTag);

    if (checkExpected && original != expectedInstruction) {
        LogE("Expected-instruction mismatch at 0x%lx: expected=0x%08X actual=0x%08X %s",
             static_cast<unsigned long>(offset), expectedInstruction, original,logTag);
        return false;
    }

    if (!WriteAndVerify(address, kArm64Nop, logTag, offset)) {
        return false;
    }

    LogI( "NOP patch succeeded at offset=0x%lx %s",
         static_cast<unsigned long>(offset),logTag);
    return true;
}

bool ApplyBranch(uintptr_t moduleBase, uintptr_t originOffset,
                 uintptr_t targetOffset, const char* logTag,
                 bool checkExpected, uint32_t expectedInstruction) {
    const uintptr_t origin = moduleBase + originOffset;
    const uintptr_t target = moduleBase + targetOffset;

    if (!IsAlignedInstructionAddress(origin, logTag, originOffset) ||
        !IsAlignedInstructionAddress(target, logTag, targetOffset)) {
        return false;
    }

    const uint32_t original =
        *reinterpret_cast<volatile uint32_t*>(origin);

    LogI( "B patch origin=0x%lx target=0x%lx original=0x%08X %s",
         static_cast<unsigned long>(originOffset),
         static_cast<unsigned long>(targetOffset), original,logTag);

    if (checkExpected && original != expectedInstruction) {
        LogE("Expected-instruction mismatch at origin 0x%lx: expected=0x%08X actual=0x%08X %s",
             static_cast<unsigned long>(originOffset), expectedInstruction, original,logTag);
        return false;
    }

    uint32_t branchInstruction = 0;
    if (!EncodeBranch(origin, target, &branchInstruction, logTag)) {
        return false;
    }

    LogI( "Encoded B instruction: 0x%08X %s", branchInstruction,logTag);

    if (!WriteAndVerify(origin, branchInstruction, logTag, originOffset)) {
        return false;
    }

    LogI("Branch patch succeeded: 0x%lx -> 0x%lx %s",
         static_cast<unsigned long>(originOffset),
         static_cast<unsigned long>(targetOffset),logTag);
    return true;
}

bool StartDetachedThread(ThreadEntry entry, void* argument, const char* logTag) {
    if (!entry) {
        LogE("Thread entry is null %s",logTag);
        return false;
    }

    pthread_t thread{};
    const int result = pthread_create(&thread, nullptr, entry, argument);
    if (result != 0) {
        LogE("pthread_create failed: %d %s", result,logTag);
        return false;
    }

    pthread_detach(thread);
    LogI( "Patch thread started %s",logTag);
    return true;
}

} // namespace PatchCommon
