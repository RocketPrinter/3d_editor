#pragma once
#include <string>
#include <vector>
#include <optional>
#include <map>
#include <set>
#include "misc.h"
#include "rendering.h"

struct Object;
struct RaycastResult;

enum class SelectionMode { Vertex, Triangle, Object };
using Selection = std::map<Object*, std::set<int>>;
const ray::Color SELECTION_COLOR = ray::ORANGE;
const float SELECTION_FREQUENCY=1;

struct Object {
    std::string name{"New object"};

    ray::Vector3 position{}, scale{1,1,1};
    ray::Quaternion rotation = ray::QuaternionIdentity();

    std::vector<ray::Vector3> vertices{};
    std::vector<int> triangle_indexes{}; // CCW winding order
    std::vector<ray::Color> triangle_colors{};

    std::vector<Object> children{};

    ray::Matrix get_model_matrix();
    // casts a Ray in the object and it's children and returns the result, coordinates are in world space
    std::optional<RaycastResult> raycast(Ray r, SelectionMode mode, ray::Matrix &parent_transform);
    void add_to_render(Renderer &renderer, ray::Matrix &parent_transform, SelectionMode selection_mode, Selection &selection, float selection_color_factor);

    static Object new_triangle();
    static Object new_cube();
    static Object new_cylinder(int nr_vertices = 12);
    static Object new_iso_sphere(); // todo:
};

struct CameraSettings {
    // camera is orbiting around the target
    ray::Vector3 target{0,0,0};
    float yaw=PI/5, pitch=-PI/4.2, distance = 4, fov = PI/2.;

    void input_movement();
    ray::Matrix get_view_projection_matrix();
    Ray ray_from_mouse_position(int x, int y);
};

struct World {
    CameraSettings camera{};
    std::vector<Object> objects{};

    SelectionMode selection_mode = SelectionMode::Triangle;
    Selection selection{};

    bool debug_render = false;

    void raycast_and_add_to_selection(int x, int y);
    void raycast_and_remove_from_selection(int x, int y);

    void render();
};

struct RaycastResult {
    float distance=MAXFLOAT;
    Object* obj = 0;
    ray::Vector3 hit_pos{};
    int index=0; // can be a vertex or triangle index depending on RaycastMode
};