#pragma once
#include "structs.h"
#include <Windows.h>
#include <numbers>

namespace math {
    inline bool WorldToScreen(const Vector3& pos, Vector3& screen, view_matrix_t matrix, int width, int height) {
        float w = matrix[3][0] * pos.x + matrix[3][1] * pos.y + matrix[3][2] * pos.z + matrix[3][3];

        if (w < 0.01f)
            return false;

        float invw = 1.0f / w;
        screen.x = (matrix[0][0] * pos.x + matrix[0][1] * pos.y + matrix[0][2] * pos.z + matrix[0][3]) * invw;
        screen.y = (matrix[1][0] * pos.x + matrix[1][1] * pos.y + matrix[1][2] * pos.z + matrix[1][3]) * invw;

        float x = width / 2.0f;
        float y = height / 2.0f;

        x += 0.5f * screen.x * width + 0.5f;
        y -= 0.5f * screen.y * height + 0.5f;

        screen.x = x;
        screen.y = y;

        return true;
    }

    inline Vector3 CalcAngle(const Vector3& src, const Vector3& dst) {
        Vector3 angle;
        Vector3 delta = dst - src;
        float hyp = sqrtf(delta.x * delta.x + delta.y * delta.y);

        angle.x = atan2f(-delta.z, hyp) * (180.0f / (float)std::numbers::pi);
        angle.y = atan2f(delta.y, delta.x) * (180.0f / (float)std::numbers::pi);
        angle.z = 0.0f;

        return angle;
    }

    inline void NormalizeAngles(Vector3& angle) {
        if (angle.x > 89.0f) angle.x = 89.0f;
        if (angle.x < -89.0f) angle.x = -89.0f;

        while (angle.y > 180.0f) angle.y -= 360.0f;
        while (angle.y < -180.0f) angle.y += 360.0f;

        angle.z = 0.0f;
    }
}
