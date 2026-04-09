#include "features.h"
#include "../sdk/offsets.h"
#include "../sdk/math.h"
#include "../sdk/config.h"
#include <Windows.h>
#include <algorithm>

namespace features {
    void RunAimbot() {
        // No more key check, active as long as config::aimbot is true
        uintptr_t clientBase = (uintptr_t)GetModuleHandleA("client.dll");
        if (!clientBase) return;

        uintptr_t localPlayerPawn = *(uintptr_t*)(clientBase + sdk::offsets::client_dll::dwLocalPlayerPawn);
        if (!localPlayerPawn) return;

        uint8_t localTeam = *(uint8_t*)(localPlayerPawn + sdk::schemas::client_dll::C_BaseEntity::m_iTeamNum);
        
        uintptr_t gameSceneNodeLocal = *(uintptr_t*)(localPlayerPawn + sdk::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);
        if (!gameSceneNodeLocal) return;

        Vector3 localOrigin = *(Vector3*)(gameSceneNodeLocal + sdk::schemas::client_dll::CGameSceneNode::m_vecAbsOrigin);
        Vector3 viewOffset = *(Vector3*)(localPlayerPawn + sdk::schemas::client_dll::C_BaseModelEntity::m_vecViewOffset);
        Vector3 eyePos = localOrigin + viewOffset;

        Vector3 currentAngles = *(Vector3*)(clientBase + sdk::offsets::client_dll::dwViewAngles);

        uintptr_t entityList = *(uintptr_t*)(clientBase + sdk::offsets::client_dll::dwEntityList);
        if (!entityList) return;

        float closestFov = 10000.0f;
        Vector3 bestDelta = { 0, 0, 0 };
        bool foundTarget = false;

        for (int i = 1; i <= 64; i++) {
            uintptr_t listEntry = *(uintptr_t*)(entityList + 0x10 + 8 * (i >> 9));
            if (!listEntry) continue;

            uintptr_t controller = *(uintptr_t*)(listEntry + 0x70 * (i & 0x1FF));
            if (!controller) continue;

            uint32_t pawnHandle = *(uint32_t*)(controller + sdk::schemas::client_dll::CCSPlayerController::m_hPlayerPawn);
            if (!pawnHandle) continue;

            uintptr_t pawnEntry = *(uintptr_t*)(entityList + 0x10 + 8 * ((pawnHandle & 0x7FFF) >> 9));
            if (!pawnEntry) continue;

            uintptr_t pawn = *(uintptr_t*)(pawnEntry + 0x70 * (pawnHandle & 0x1FF));
            if (!pawn || pawn == localPlayerPawn) continue;

            int health = *(int*)(pawn + sdk::schemas::client_dll::C_BaseEntity::m_iHealth);
            if (health <= 0 || health > 100) continue;

            uint8_t team = *(uint8_t*)(pawn + sdk::schemas::client_dll::C_BaseEntity::m_iTeamNum);
            if (team == localTeam) continue;

            if (config::aim_visible_check) {
                uintptr_t entitySpottedState = pawn + sdk::schemas::client_dll::C_CSPlayerPawn::m_entitySpottedState;
                bool isSpotted = *(bool*)(entitySpottedState + sdk::schemas::client_dll::EntitySpottedState_t::m_bSpotted);
                uint32_t spottedByMask = *(uint32_t*)(entitySpottedState + sdk::schemas::client_dll::EntitySpottedState_t::m_bSpottedByMask);
                
                // If it's not spotted at all, skip it (simple visibility fallback without tracing)
                if (!isSpotted && spottedByMask == 0) continue;
            }

            uintptr_t gameSceneNode = *(uintptr_t*)(pawn + sdk::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);
            if (!gameSceneNode) continue;

            uintptr_t boneArray = *(uintptr_t*)(gameSceneNode + 0x1E0); // m_modelState + 0x80
            if (!boneArray) continue;

            // Bone 6 is usually the head in CS2
            Vector3 targetHead = *(Vector3*)(boneArray + 6 * 32);

            Vector3 angles = math::CalcAngle(eyePos, targetHead);
            math::NormalizeAngles(angles);

            Vector3 delta = angles - currentAngles;
            math::NormalizeAngles(delta);

            float fov = sqrtf(delta.x * delta.x + delta.y * delta.y);
            if (fov < closestFov && fov < config::aim_fov) {
                closestFov = fov;
                bestDelta = delta;
                foundTarget = true;
            }
        }

        if (foundTarget) {
            float smooth = config::aim_smooth;
            if (smooth < 1.0f) smooth = 1.0f; // Prevent division by zero
            
            Vector3 finalAngles = currentAngles + (bestDelta * (1.0f / smooth));
            math::NormalizeAngles(finalAngles);

            // Write to dwViewAngles
            *(Vector3*)(clientBase + sdk::offsets::client_dll::dwViewAngles) = finalAngles;
        }
    }
}
