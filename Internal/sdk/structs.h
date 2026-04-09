#pragma once
#include <cmath>

struct Vector3 {
    float x, y, z;

    Vector3 operator+(const Vector3& v) const { return { x + v.x, y + v.y, z + v.z }; }
    Vector3 operator-(const Vector3& v) const { return { x - v.x, y - v.y, z - v.z }; }
    Vector3 operator*(float f) const { return { x * f, y * f, z * f }; }

    float DistTo(const Vector3& v) const {
        return sqrtf(powf(v.x - x, 2) + powf(v.y - y, 2) + powf(v.z - z, 2));
    }
};

struct view_matrix_t {
    float* operator[](int index) {
        return matrix[index];
    }

    float matrix[4][4];
};
