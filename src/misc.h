#pragma once

#include <cmath>
namespace ray {
    #include "raylib.h"
    #include "raymath.h"
}

const int screenWidth = 800;
const int screenHeight = 450;

const char* v3_to_text(ray::Vector3 v3);
const char* trig_to_text(ray::Vector3 trig[3]);