#pragma once
#include <string>
#include <vector>
#include <optional>
#include <map>
#include <set>
#include "rendering.h"
#include "misc.h"

struct Object;
struct World;
struct Menu;
struct RaycastResult;

enum class SelectionMode { Vertex, Triangle, Object };
enum class Operation {Translate, Rotate, Scale};
using Selection = std::map<Object*, std::set<int>>;
const ray::Color SELECTION_COLOR = ray::ORANGE;
const ray::Color UNSELECTED_VERTEX_COLOR = ray::DARKGRAY;
const float SELECTION_FREQUENCY=1.8;

struct Object {
    std::string name{"New object"};

    bool is_visible = true;
    ray::Vector3 position{}, scale{1,1,1};
    ray::Quaternion rotation = ray::QuaternionIdentity();

    std::vector<ray::Vector3> vertices{};
    std::vector<int> triangle_indexes{}; // CCW winding order
    std::vector<ray::Color> triangle_colors{};

    std::vector<Object*> children{};

    ray::Matrix get_model_matrix();
    // casts a Ray in the object and it's children and returns the result, coordinates are in world space
    std::optional<RaycastResult> raycast(Ray r, SelectionMode mode, ray::Matrix &parent_transform);
    void add_to_render(Renderer &renderer, ray::Matrix &parent_transform, float selection_color_factor, World &world);

    static Object* new_triangle();
    static Object* new_plane(int divisions_x=4, int divisions_z=4);
    static Object* new_cube();
    static Object* new_cylinder(int nr_vertices = 16);
    static Object* new_cone(int nr_vertices = 16);
    static Object* new_sphere(int meridians=16, int parallels=8);
};

struct CameraSettings {
    // camera is orbiting around the target
    ray::Vector3 target{0,0,0};
    float yaw=PI/5, pitch=-PI/4.2, distance = 4, fov = PI/2.;

    ray::Vector3 get_position();
    ray::Matrix get_view_projection_matrix();
    Ray ray_from_mouse_position();
    void input_movement();
};

struct World {
    CameraSettings camera{};
    std::vector<Object*> objects{};

    SelectionMode selection_mode = SelectionMode::Object;
    Operation operation = Operation::Translate; // only matters if selection_mode is Object
    Selection selection{};
    Menu *menu;
    bool show_vertices = false;
    bool debug_render = false;
    ray::Color paint_color = ray::RED;
    // list of points to be drawn during next render() call
    std::vector<RenderPoint> point_queue{};

    void raycast_and_modify_selection(bool remove_from_selection=false);
    void addNewObject(Object* object);
    void render();
};

struct RaycastResult {
    float distance=MAXFLOAT;
    Object* obj = 0;
    ray::Vector3 hit_pos{};
    int index=0; // can be a vertex or triangle index depending on RaycastMode
};