#pragma once

namespace config {
    // Visuals
    inline bool esp = false;
    inline bool esp_boxes = false;
    inline bool esp_skeletons = false;
    inline bool esp_names = false;
    inline bool esp_weapons = false;
    inline bool esp_camera = false;
    inline bool esp_reload = false;

    // Aimbot
    inline bool aimbot = false;
    inline float aim_fov = 10.0f;
    inline float aim_smooth = 5.0f;
    inline bool aim_visible_check = false;
    inline bool draw_fov = false;

    // Triggerbot
    inline bool triggerbot = false;

    // Misc
    // (Bhop removed for stability)

    void Save();
    void Load();
}
