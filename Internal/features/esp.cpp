#include "features.h"
#include "../sdk/offsets.h"
#include "../sdk/math.h"
#include "../sdk/config.h"
#include <Windows.h>
#include <iostream>
#include <string>

namespace features {

    void DrawBoneLine(ImDrawList* drawList, uintptr_t boneArray, int bone1, int bone2, view_matrix_t viewMatrix, int width, int height) {
        Vector3 pos1 = *(Vector3*)(boneArray + bone1 * 32);
        Vector3 pos2 = *(Vector3*)(boneArray + bone2 * 32);
        
        Vector3 screen1, screen2;
        if (math::WorldToScreen(pos1, screen1, viewMatrix, width, height) &&
            math::WorldToScreen(pos2, screen2, viewMatrix, width, height)) {
            drawList->AddLine(ImVec2(screen1.x, screen1.y), ImVec2(screen2.x, screen2.y), IM_COL32(255, 255, 255, 255), 1.5f);
        }
    }

    void RenderESP() {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        
        uintptr_t clientBase = (uintptr_t)GetModuleHandleA("client.dll");
        if (!clientBase) return;

        uintptr_t entityList = *(uintptr_t*)(clientBase + sdk::offsets::client_dll::dwEntityList);
        if (!entityList) return;

        uintptr_t localPlayerPawn = *(uintptr_t*)(clientBase + sdk::offsets::client_dll::dwLocalPlayerPawn);
        if (!localPlayerPawn) return;

        uint8_t localTeam = *(uint8_t*)(localPlayerPawn + sdk::schemas::client_dll::C_BaseEntity::m_iTeamNum);
        view_matrix_t viewMatrix = *(view_matrix_t*)(clientBase + sdk::offsets::client_dll::dwViewMatrix);

        ImGuiIO& io = ImGui::GetIO();
        int width = (int)io.DisplaySize.x;
        int height = (int)io.DisplaySize.y;

        for (int i = 1; i <= 64; i++) {
            uintptr_t listEntry = *(uintptr_t*)(entityList + 0x10 + 8 * (i >> 9));
            if (!listEntry) continue;

            uintptr_t controller = *(uintptr_t*)(listEntry + 0x70 * (i & 0x1FF)); // STRIDE IS 0x70!
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
            if (team == localTeam) continue; // Enemy only for now

            uintptr_t gameSceneNode = *(uintptr_t*)(pawn + sdk::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);
            if (!gameSceneNode) continue;

            Vector3 origin = *(Vector3*)(gameSceneNode + sdk::schemas::client_dll::CGameSceneNode::m_vecAbsOrigin);
            Vector3 headPos = origin;
            headPos.z += 72.0f; // Approx head height

            Vector3 screenOrigin, screenHead;
            bool onScreenOrigin = math::WorldToScreen(origin, screenOrigin, viewMatrix, width, height);
            bool onScreenHead = math::WorldToScreen(headPos, screenHead, viewMatrix, width, height);
            
            if (onScreenOrigin && onScreenHead) {
                float boxHeight = screenOrigin.y - screenHead.y;
                float boxWidth = boxHeight / 2.0f;

                if (config::esp_boxes) {
                    drawList->AddRect(
                        ImVec2(screenHead.x - boxWidth / 2.0f, screenHead.y),
                        ImVec2(screenHead.x + boxWidth / 2.0f, screenOrigin.y),
                        IM_COL32(255, 0, 0, 255)
                    );

                    // Health bar
                    float healthFrac = health / 100.0f;
                    drawList->AddRectFilled(
                        ImVec2(screenHead.x - boxWidth / 2.0f - 5.0f, screenHead.y + (boxHeight * (1.0f - healthFrac))),
                        ImVec2(screenHead.x - boxWidth / 2.0f - 2.0f, screenOrigin.y),
                        IM_COL32(0, 255, 0, 255)
                    );
                }

                if (config::esp_skeletons) {
                    uintptr_t boneArray = *(uintptr_t*)(gameSceneNode + 0x1E0); // m_modelState + 0x80
                    if (boneArray) {
                        DrawBoneLine(drawList, boneArray, 6, 5, viewMatrix, width, height);
                        DrawBoneLine(drawList, boneArray, 5, 4, viewMatrix, width, height);
                        DrawBoneLine(drawList, boneArray, 4, 0, viewMatrix, width, height);
                        DrawBoneLine(drawList, boneArray, 5, 8, viewMatrix, width, height);
                        DrawBoneLine(drawList, boneArray, 8, 9, viewMatrix, width, height);
                        DrawBoneLine(drawList, boneArray, 9, 10, viewMatrix, width, height);
                        DrawBoneLine(drawList, boneArray, 5, 13, viewMatrix, width, height);
                        DrawBoneLine(drawList, boneArray, 13, 14, viewMatrix, width, height);
                        DrawBoneLine(drawList, boneArray, 14, 15, viewMatrix, width, height);
                        DrawBoneLine(drawList, boneArray, 0, 22, viewMatrix, width, height);
                        DrawBoneLine(drawList, boneArray, 22, 23, viewMatrix, width, height);
                        DrawBoneLine(drawList, boneArray, 23, 24, viewMatrix, width, height);
                        DrawBoneLine(drawList, boneArray, 0, 25, viewMatrix, width, height);
                        DrawBoneLine(drawList, boneArray, 25, 26, viewMatrix, width, height);
                        DrawBoneLine(drawList, boneArray, 26, 27, viewMatrix, width, height);
                    }
                }

                if (config::esp_names) {
                    char* playerName = (char*)(controller + 0x6F8); // m_iszPlayerName
                    if (playerName && playerName[0]) {
                        ImVec2 textSize = ImGui::CalcTextSize(playerName);
                        drawList->AddText(ImVec2(screenHead.x - textSize.x / 2.0f, screenHead.y - 15.0f), IM_COL32(255, 255, 255, 255), playerName);
                    }
                }

                // Fetch weapon and reload state
                uintptr_t pClippingWeapon = *(uintptr_t*)(pawn + sdk::schemas::client_dll::C_CSPlayerPawn::m_pClippingWeapon);
                if (pClippingWeapon) {
                    if (config::esp_weapons) {
                        uintptr_t pIdentity = *(uintptr_t*)(pClippingWeapon + 0x10);
                        if (pIdentity) {
                            uintptr_t pWeaponName = *(uintptr_t*)(pIdentity + 0x20); // m_designerName
                            if (pWeaponName) {
                                char* weaponName = (char*)pWeaponName;
                                // Trim "weapon_" prefix if exists
                                if (strncmp(weaponName, "weapon_", 7) == 0) {
                                    weaponName += 7;
                                }
                                ImVec2 textSize = ImGui::CalcTextSize(weaponName);
                                drawList->AddText(ImVec2(screenOrigin.x - textSize.x / 2.0f, screenOrigin.y + 5.0f), IM_COL32(200, 200, 200, 255), weaponName);
                            }
                        }
                    }

                    if (config::esp_reload) {
                        bool isReloading = *(bool*)(pClippingWeapon + 0x19E4); // m_bInReload
                        if (isReloading) {
                            const char* reloadText = "* RELOADING *";
                            ImVec2 textSize = ImGui::CalcTextSize(reloadText);
                            drawList->AddText(ImVec2(screenOrigin.x - textSize.x / 2.0f, screenOrigin.y + 20.0f), IM_COL32(255, 165, 0, 255), reloadText);
                        }
                    }
                }

                if (config::esp_camera) {
                    Vector3 eyeAngles = *(Vector3*)(pawn + sdk::schemas::client_dll::C_CSPlayerPawn::m_angEyeAngles);

                    
                    // Convert Pitch/Yaw to Forward Vector
                    float pitch = eyeAngles.x * (3.1415926535f / 180.0f);
                    float yaw = eyeAngles.y * (3.1415926535f / 180.0f);
                    
                    Vector3 forward;
                    forward.x = cosf(pitch) * cosf(yaw);
                    forward.y = cosf(pitch) * sinf(yaw);
                    forward.z = -sinf(pitch);

                    Vector3 eyePos = origin;
                    eyePos.z += 65.0f; // Eye level approx

                    Vector3 endPos = eyePos + (forward * 50.0f); // 50 units forward
                    Vector3 screenEye, screenEnd;

                    if (math::WorldToScreen(eyePos, screenEye, viewMatrix, width, height) &&
                        math::WorldToScreen(endPos, screenEnd, viewMatrix, width, height)) {
                        drawList->AddLine(ImVec2(screenEye.x, screenEye.y), ImVec2(screenEnd.x, screenEnd.y), IM_COL32(255, 0, 255, 255), 1.5f);
                    }
                }
            }
        }
    }
}
