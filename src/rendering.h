#pragma once
#include "misc.h"
#include <vector>

struct RenderContext {
    // todo
};

struct RenderTrig {
    // counter-clockwise winding order
    ray::Vector3 vertices[3]{};
    ray::Color col{};

    // axis aligned bounding box
    ray::Vector3 aabb_min{}, aabb_max{};
    // (C-A)X(B-A)
    ray::Vector3 cross{};

    // triangle dependency tree
    bool drawn = false;
    std::vector<int> draw_dependencies{};

    RenderTrig() = default;
    RenderTrig(const ray::Vector3 v1, const ray::Vector3 v2, const ray::Vector3 v3, ray::Color c);
};

// uses painter's algorithm
// assumes that no triangles intersect nor there are "cycles" where t1 covers t2 which covers t3 which covers t1
// for this purpose clip space is defined as: [-1, 1] on x and y axis and [-inf, 0] on z axis
void render(std::vector<RenderTrig> trigs, bool draw_stats);

