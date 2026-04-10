#include "features.h"
#include "../sdk/offsets.h"
#include "../sdk/config.h"
#include <Windows.h>

namespace features {
    void RunTriggerbot() {
        if (!config::triggerbot) return;

        uintptr_t clientBase = (uintptr_t)GetModuleHandleA("client.dll");
        if (!clientBase) return;

        uintptr_t localPlayerPawn = *(uintptr_t*)(clientBase + sdk::offsets::client_dll::dwLocalPlayerPawn);
        if (!IsValidPtr(localPlayerPawn)) return;

        int localHealth = *(int*)(localPlayerPawn + sdk::schemas::client_dll::C_BaseEntity::m_iHealth);
        if (localHealth <= 0) return; // Must be alive

        uint8_t localTeam = *(uint8_t*)(localPlayerPawn + sdk::schemas::client_dll::C_BaseEntity::m_iTeamNum);
        int entIndex = *(int*)(localPlayerPawn + sdk::schemas::client_dll::C_CSPlayerPawn::m_iIDEntIndex);

        if (entIndex > 0 && entIndex < 16384) {
            uintptr_t entityList = *(uintptr_t*)(clientBase + sdk::offsets::client_dll::dwEntityList);
            if (!IsValidPtr(entityList)) return;

            uintptr_t listEntry = *(uintptr_t*)(entityList + 0x8 * ((entIndex & 0x7FFF) >> 9) + 0x10);
            if (!IsValidPtr(listEntry)) return;

            uintptr_t pawn = *(uintptr_t*)(listEntry + 0x70 * (entIndex & 0x1FF));
            if (!IsValidPtr(pawn)) return;

            int health = *(int*)(pawn + sdk::schemas::client_dll::C_BaseEntity::m_iHealth);
            uint8_t team = *(uint8_t*)(pawn + sdk::schemas::client_dll::C_BaseEntity::m_iTeamNum);

            if (team != localTeam && health > 0) {
                // Simulate Mouse Left Click
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                Sleep(10);
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                Sleep(100); // Wait for next shot
            }
        }
    }
}
