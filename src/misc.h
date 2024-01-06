#pragma once

#include <cmath>
namespace ray {
    #include "raylib.h"
    #include "raymath.h"

    inline Vector4 Vector4Transform(Vector4 v, Matrix mat) {
        return QuaternionTransform(v, mat);
    }
    inline Vector4 Vector4FromVector3(Vector3 v, float w) {
        return Vector4{v.x,v.y,v.z,w};
    }

    // checks that the components aren't NaN or inf
    inline bool Vector3Finite(Vector3 v) {
        return isfinite(v.x) && isfinite(v.y) && isfinite(v.z);
    }
}

const int screenWidth = 800;
const int screenHeight = 450;

const char* v2_to_text(ray::Vector2 v);
const char* v3_to_text(ray::Vector3 v);
const char* v4_to_text(ray::Vector4 v);
const char* color_to_text(ray::Color col);
const char* trig_to_text(ray::Vector3 v0, ray::Vector3 v1, ray::Vector3 v2);

void debug_text(const char *text, ray::Color color = ray::GRAY);
void reset_draw_debug();