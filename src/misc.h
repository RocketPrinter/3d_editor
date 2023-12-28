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
}

const int screenWidth = 800;
const int screenHeight = 450;

const char* v3_to_text(ray::Vector3 v);
const char* v4_to_text(ray::Vector4 v);
const char* trig_to_text(ray::Vector3 trig[3]);