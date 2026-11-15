#pragma once

#include <cstdint>
#include <cstddef>
#include <unistd.h>

namespace PatchCommon {

using ThreadEntry = void* (*)(void*);

// libapp.so のロードベースを待機して取得する。
// offset は Frida の Module.base を基準にしたオフセットを想定。
bool WaitForModuleBase(
    const char* moduleName,
    uintptr_t* outBase,
    const char* logTag,
    int attempts = 500,
    useconds_t intervalUs = 20000
);

// ARM64 NOP (0xD503201F) を書き込む。
bool ApplyNop(
    uintptr_t moduleBase,
    uintptr_t offset,
    const char* logTag,
    bool checkExpected = false,
    uint32_t expectedInstruction = 0
);

// ARM64 B imm26 で originOffset から targetOffset へ分岐させる。
bool ApplyBranch(
    uintptr_t moduleBase,
    uintptr_t originOffset,
    uintptr_t targetOffset,
    const char* logTag,
    bool checkExpected = false,
    uint32_t expectedInstruction = 0
);

// detached pthread を開始する。
bool StartDetachedThread(ThreadEntry entry, void* argument, const char* logTag);

} // namespace PatchCommon
