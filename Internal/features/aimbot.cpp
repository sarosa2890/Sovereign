#include "features.h"
#include "../sdk/offsets.h"
#include "../sdk/math.h"
#include "../sdk/config.h"
#include "../imgui/imgui.h"
#include <Windows.h>
#include <cmath>

namespace features {
    void RunAimbot() {
        if (!config::aimbot) return;

        uintptr_t clientBase = (uintptr_t)GetModuleHandleA("client.dll");
        if (!clientBase) return;

        uintptr_t localPlayerPawn = *(uintptr_t*)(clientBase + sdk::offsets::client_dll::dwLocalPlayerPawn);
        if (!localPlayerPawn) return;

        uint8_t localTeam = *(uint8_t*)(localPlayerPawn + sdk::schemas::client_dll::C_BaseEntity::m_iTeamNum);
        
        uintptr_t entityList = *(uintptr_t*)(clientBase + sdk::offsets::client_dll::dwEntityList);
        if (!entityList) return;

        view_matrix_t viewMatrix = *(view_matrix_t*)(clientBase + sdk::offsets::client_dll::dwViewMatrix);
        
        ImGuiIO& io = ImGui::GetIO();
        int width = (int)io.DisplaySize.x;
        int height = (int)io.DisplaySize.y;

        if (width <= 0 || height <= 0) return; // Prevent division by zero if game window is minimized
        
        float centerX = width / 2.0f;
        float centerY = height / 2.0f;
        
        // Calculate FOV in pixels safely
        float fovRadius = 0.0f;
        float tanHalfFov = tanf(config::aim_fov * (3.1415926535f / 180.0f) / 2.0f);
        float tanHalf90 = tanf(90.0f * (3.1415926535f / 180.0f) / 2.0f);
        if (tanHalf90 != 0.0f) {
             fovRadius = (tanHalfFov / tanHalf90) * centerX;
        }

        float closestDist = 10000.0f;
        Vector3 bestScreenPos = { 0, 0, 0 };
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
                
                if (!isSpotted && spottedByMask == 0) continue;
            }

            uintptr_t gameSceneNode = *(uintptr_t*)(pawn + sdk::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);
            if (!gameSceneNode) continue;

            uintptr_t boneArray = *(uintptr_t*)(gameSceneNode + 0x1E0); // m_modelState + 0x80
            if (!boneArray) continue;

            // Bone 6 is usually the head in CS2
            Vector3 targetHead = *(Vector3*)(boneArray + 6 * 32);

            Vector3 screenPos;
            if (math::WorldToScreen(targetHead, screenPos, viewMatrix, width, height)) {
                
                // Prevent NaN or Infinity from breaking distance calculation
                if (std::isnan(screenPos.x) || std::isnan(screenPos.y) || std::isinf(screenPos.x) || std::isinf(screenPos.y)) continue;

                float dist = sqrtf(powf(screenPos.x - centerX, 2) + powf(screenPos.y - centerY, 2));
                
                if (dist < closestDist && dist <= fovRadius) {
                    closestDist = dist;
                    bestScreenPos = screenPos;
                    foundTarget = true;
                }
            }
        }

        if (foundTarget) {
            float smooth = config::aim_smooth;
            if (smooth < 1.0f) smooth = 1.0f; // Prevent division by zero
            
            float deltaX = bestScreenPos.x - centerX;
            float deltaY = bestScreenPos.y - centerY;

            // Apply smoothing
            deltaX /= smooth;
            deltaY /= smooth;

            // Strict safety caps to prevent game/mouse driver from crashing
            if (deltaX > 50.0f) deltaX = 50.0f;
            if (deltaX < -50.0f) deltaX = -50.0f;
            if (deltaY > 50.0f) deltaY = 50.0f;
            if (deltaY < -50.0f) deltaY = -50.0f;

            if (std::isnan(deltaX) || std::isnan(deltaY) || std::isinf(deltaX) || std::isinf(deltaY)) return;

            // Move the mouse
            mouse_event(MOUSEEVENTF_MOVE, (DWORD)(int)deltaX, (DWORD)(int)deltaY, 0, 0);
        }
    }
}
