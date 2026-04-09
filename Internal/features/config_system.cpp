#include "../sdk/config.h"
#include <fstream>
#include <iostream>
#include <string>
#include <Windows.h>
#include <shlobj.h>

namespace config {
    std::string GetConfigPath() {
        char path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path))) {
            return std::string(path) + "\\neon_cs2.cfg";
        }
        return "C:\\neon_cs2.cfg";
    }

    void Save() {
        std::ofstream file(GetConfigPath());
        if (file.is_open()) {
            file << esp << "\n";
            file << esp_boxes << "\n";
            file << esp_skeletons << "\n";
            file << esp_names << "\n";
            file << esp_weapons << "\n";
            file << esp_camera << "\n";
            file << esp_reload << "\n";
            file << aimbot << "\n";
            file << aim_fov << "\n";
            file << aim_smooth << "\n";
            file << aim_visible_check << "\n";
            file << draw_fov << "\n";
            file << triggerbot << "\n";
            file.close();
        }
    }

    void Load() {
        std::ifstream file(GetConfigPath());
        if (file.is_open()) {
            file >> esp;
            file >> esp_boxes;
            file >> esp_skeletons;
            file >> esp_names;
            file >> esp_weapons;
            file >> esp_camera;
            file >> esp_reload;
            file >> aimbot;
            file >> aim_fov;
            file >> aim_smooth;
            file >> aim_visible_check;
            file >> draw_fov;
            file >> triggerbot;
            file.close();
        }
    }
}

