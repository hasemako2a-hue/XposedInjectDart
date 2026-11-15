#include "../../common/PatchCommon.h"

namespace {
constexpr char kModuleName[] = "libapp.so";
constexpr bool kCheckExpectedInstruction = false;
constexpr uint32_t kExpectedInstruction = 0x00000000;
    
constexpr char kLogTag[] = "BranchPatch";    
constexpr uintptr_t kPatchParams[][] = {
    {0x8afdf8, 0x8c5784},
    {0x8ca3a8, 0x8c5784},
    {0x8c5b34, 0x8c591c}
};

void* PatchThread(void*) {
    uintptr_t base = 0;
    if (!PatchCommon::WaitForModuleBase(
            kModuleName, &base, kLogTag)) {
        return nullptr;
    }

    constexpr size_t count = sizeof(kPatchParams) ;
    for (size_t i = 0; i < count; ++i) {
        PatchCommon::ApplyBranch(
            base,
            kPatchParams[i][0],
            kPatchParams[i][1],
            kLogTag,
            kCheckExpectedInstruction,
            kExpectedInstruction
        );
    }
    return nullptr;
}
} // namespace

extern "C" void StartGamePatchThread() {
    PatchCommon::StartDetachedThread(PatchThread, nullptr, kLogTag);
}
