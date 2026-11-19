#include "../common/PatchCommon.h"

namespace {
constexpr char kModuleName[] = "libapp.so";
constexpr bool kCheckExpectedInstruction = false;
constexpr uint32_t kExpectedInstruction = 0x00000000;
    
constexpr char kLogTag[] = "slider";    
constexpr uintptr_t kPatchParams[][2] = {
    {0x8b0588,0x8c5f14},
    {0x8cab74,0x8c5f14},
    {0x8c6300,0x8c60e8}
};

void* PatchThread(void*) {
    uintptr_t base = 0;
    if (!PatchCommon::WaitForModuleBase(
            kModuleName, &base, kLogTag)) {
        return nullptr;
    }

    constexpr size_t count = sizeof(kPatchParams)/ sizeof(kPatchParams[0]) ;
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
