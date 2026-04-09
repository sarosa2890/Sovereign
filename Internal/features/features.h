#pragma once
#include "../imgui/imgui.h"
#include <Windows.h>
#include <cstdint>

namespace features {
    inline bool IsValidPtr(uintptr_t ptr) {
        if (!ptr) return false;
        if (ptr < 0x10000 || ptr > 0x000F000000000000) return false;
        if (IsBadReadPtr((void*)ptr, sizeof(uintptr_t))) return false;
        return true;
    }

    void RenderESP();
    void RunAimbot();
    void RunTriggerbot();
    void RunMisc();
}
