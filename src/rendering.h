#pragma once
#include <vector>
#include <memory>
#include "misc.h"

struct Plane {
    ray::Vector3 normal; float offset;

    // if > 0 then point is above the plane, == 0 is on the plane and < 0 is under
    float distance(ray::Vector3 point);
    // we assume that the line does intersect
    ray::Vector3 line_intersection(ray::Vector3 a, ray::Vector3 b);
};

struct Triangle {
    ray::Vector3 v0,v1,v2;
    ray::Color col = ray::RED;

    Plane get_plane();
    // ensures that triangle is CCW
    Triangle ccw();
};

// a binary space partitioning tree is used to accurately z-sort the triangles
// each node divides the world using a plane, triangles can be above the plane, below the plane, intersecting the plane or on the plane
class BSPNode {
    Plane plane;

    // triangles that are on the plane, 3 vertices = 1 triangle
    std::vector<ray::Vector3> trig_vertices;
    std::vector<ray::Color> trig_colors;

    std::unique_ptr<BSPNode> below{}, above{};

    // helper functions
    void add_above(Triangle trig, Plane p);
    void add_below(Triangle trig, Plane p);

public:
    BSPNode(Triangle trig, Plane plane);

    void add(Triangle trig, Plane p);
    void draw();
    void draw_debug(int previous);
};

class Renderer {
    std::unique_ptr<BSPNode> root{};
public:
    // clips triangles, checks their winding order and adds them to the BSP tree if visible on the screen
    // for this purpose clip space is defined as: [-1, 1] on x and y axis and [-inf, 0] on z axis (right handed)
    bool cull_or_add_to_bsp_tree(Triangle trig);
    // draws the BSP tree using Painter's algorithm, triangles further away are drawn first
    void draw(bool debug=false);
};

ray::Vector2 clip_to_screen_space(ray::Vector3 v3);