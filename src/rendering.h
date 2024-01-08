#pragma once
#include <vector>
#include <memory>
#include "misc.h"

struct RenderPoint;

// a binary space partitioning tree is used to accurately z-sort the triangles
// each node divides the world using a plane, triangles can be above the plane, below the plane, intersecting the plane or on the plane
struct BSPNode {
    Plane plane;

    // triangles that are on the plane, 3 vertices = 1 triangle
    std::vector<ray::Vector3> trig_vertices;
    std::vector<ray::Color> trig_colors;

    std::unique_ptr<BSPNode> below{}, above{};

    // helper functions
    void add_above(Triangle trig, Plane p, ray::Color col);
    void add_below(Triangle trig, Plane p, ray::Color col);

    BSPNode(Triangle trig, Plane plane, ray::Color col);

    void add(Triangle trig, Plane p, ray::Color col);
    void draw();
    void draw_debug(int previous);
};

struct Renderer {
    std::unique_ptr<BSPNode> root{};

    // clips triangles, checks their winding order and adds them to the BSP tree if visible on the screen
    // for this purpose clip space is defined as: [-1, 1] on x and y axis and [-inf, 0] on z axis (right handed)
    bool cull_or_add_to_bsp_tree(Triangle trig, ray::Color col);
    void add_point(const ray::Matrix &mvp_matrix, RenderPoint point);
    // draws the BSP tree using Painter's algorithm, triangles further away are drawn first
    void draw(bool debug=false);
};

ray::Vector2 clip_to_screen_space(ray::Vector3 v3);

struct RenderPoint {
    ray::Vector3 pos{};
    float size=0.2;
    ray::Color col=ray::RED;
};