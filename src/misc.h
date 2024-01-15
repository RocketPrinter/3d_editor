#pragma once

#include <optional>
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

#ifdef _WIN32
// for some reason it's needed for the project to compile on windows
#include <limits>
#define MAXFLOAT  std::numeric_limits<float>::max();
#endif

const int screenWidth = 960;
const int screenHeight = 540;

/// DEBUG STUFF
const char* v2_to_text(ray::Vector2 v);
const char* v3_to_text(ray::Vector3 v);
const char* v4_to_text(ray::Vector4 v);
const char* color_to_text(ray::Color col);
const char* trig_to_text(ray::Vector3 v0, ray::Vector3 v1, ray::Vector3 v2);

void debug_text(const char *text, ray::Color color = ray::GRAY);
void reset_draw_debug();

/// GEOMETRY CONSTRUCTS
struct Ray {
    ray::Vector3 origin{}, direction{};

    static Ray from_points(ray::Vector3 a, ray::Vector3 b);
    ray::Vector3 point_at_distance(float t);
};

struct Plane {
    ray::Vector3 normal; float offset;

    // if > 0 then point is above the plane, == 0 is on the plane and < 0 is under
    float distance(ray::Vector3 point);
    float ray_intersection(Ray r);
};

struct Sphere {
    ray::Vector3 center;
    float radius;

    // returns the closest intersection point
    std::optional<float> ray_intersection(Ray r);
};

struct Triangle {
    ray::Vector3 v0,v1,v2;

    Plane get_plane();
    bool is_point_on_triangle(ray::Vector3 p);
    // returns a triangle that must be CCW
    Triangle ccw();
};

ray::Vector3 apply_transformation(ray::Vector3 v, ray::Matrix matrix);
ray::Vector2 clip_to_screen_space(ray::Vector3 v3);
ray::Color lerp_color(ray::Color a, ray::Color b, float t);