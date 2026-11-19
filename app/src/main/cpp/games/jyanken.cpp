#include "../common/PatchCommon.h"

namespace {
constexpr char kModuleName[] = "libapp.so";
constexpr bool kCheckExpectedInstruction = false;
constexpr uint32_t kExpectedInstruction = 0x00000000;
    
constexpr char kLogTag[] = "jyanken";    
constexpr uintptr_t kPatchParams[] = {
    0x8bd330,
    0x8bd380,
    0x8bd3dc
};

void* PatchThread(void*) {
    uintptr_t base = 0;
    if (!PatchCommon::WaitForModuleBase(
            kModuleName, &base, kLogTag)) {
        return nullptr;
    }

    constexpr size_t count = sizeof(kPatchParams)/ sizeof(kPatchParams[0]) ;
    for (size_t i = 0; i < count; ++i) {
        PatchCommon::ApplyNop(
            base,
            kPatchParams[i],
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
