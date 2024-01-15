#include <set>
#include "object.h"
#include "serialization.h"
#include "menu.h"
#define RAYGUI_IMPLEMENTATION
#include "lib/raygui.h"
#include "misc.h"

void test_config(World &world);

static World world{};


void fillMenu(Menu &menu){
//    menu.clearMenu();
    for(Object* obj : world.objects){
        menu.addToMenu(obj);
    }
}
int main()
{
    ray::InitWindow(screenWidth, screenHeight, "Editor 3D");
    ray::SetTargetFPS(60);
    Menu menu{};
    menu.world = &world;
    world.menu = &menu;
    if (not deserialize(world)) {
        world.objects.push_back(Object::new_cube());
    }
    fillMenu(*world.menu);
    // Main game loop
    while (!ray::WindowShouldClose())
    {
        ray::BeginDrawing();
        ClearBackground(ray::RAYWHITE);

        if (ray::IsKeyPressed(ray::KEY_SLASH)) world.debug_render = !world.debug_render;
        if (ray::IsMouseButtonDown(ray::MOUSE_BUTTON_LEFT) && !ray::IsKeyDown(ray::KEY_LEFT_SHIFT)) world.raycast_and_modify_selection(false);
        if (ray::IsMouseButtonDown(ray::MOUSE_BUTTON_RIGHT)) world.raycast_and_modify_selection(true);
        world.camera.input_movement();

        world.render();
//        test_config(world);

        reset_draw_debug();

        menu.showMenu();
        ray::EndDrawing();
    }

    ray::CloseWindow();

    return 0;
}

int editing=0, new_obj_type=3;
void test_config(World &world) {
    if (ray::IsKeyPressed(ray::KEY_SPACE))
        editing = (editing + 1) % 3;

    if (ray::IsKeyPressed(ray::KEY_TAB))
        new_obj_type = (new_obj_type + 1) % 6;

    if (ray::IsKeyPressed(ray::KEY_GRAVE)) {
        world.selection_mode = (SelectionMode)(((int)world.selection_mode + 1 ) % 3);
        world.selection.clear();
    }

    if (ray::IsKeyPressed(ray::KEY_N)) {
        Object* obj;
        switch (new_obj_type) {
            case 0:
                obj = Object::new_triangle();
                break;
            case 1:
                obj = Object::new_plane();
                break;
            case 2:
                obj = Object::new_cube();
                break;
            case 3:
                obj = Object::new_cylinder();
                break;
            case 4:
                obj = Object::new_cone();
                break;
            case 5:
                obj = Object::new_sphere();
                break;
        }
        world.objects.push_back(obj);
    }

    ray::Vector3 input{};
    if (ray::IsKeyDown(ray::KEY_J))input.x -= 0.02;
    if (ray::IsKeyDown(ray::KEY_L))input.x += 0.02;
    if (ray::IsKeyDown(ray::KEY_I))input.y -= 0.02;
    if (ray::IsKeyDown(ray::KEY_K))input.y += 0.02;
    if (ray::IsKeyDown(ray::KEY_U))input.z -= 0.02;
    if (ray::IsKeyDown(ray::KEY_O))input.z += 0.02;

    switch (new_obj_type) {
        case 0:debug_text("new obj: triangle");break;
        case 1:debug_text("new obj: quad"    );break;
        case 2:debug_text("new obj: cube"  );break;
        case 3:debug_text("new obj: cylinder");break;
        case 4:debug_text("new obj: cone"    );break;
        case 5:debug_text("new obj: sphere"  );break;
    }

    switch (world.selection_mode) {
        case SelectionMode::Vertex:  debug_text("sel mode: vertex"  );break;
        case SelectionMode::Triangle:debug_text("sel mode: triangle");break;
        case SelectionMode::Object:  debug_text("sel mode: object"  );break;
    }

    switch (world.selection_mode) {
        case SelectionMode::Vertex:
            for (auto kvp: world.selection)
                for (auto i: kvp.second) {
                    auto &p = kvp.first->vertices[i];
                    p = ray::Vector3Add(p, input);
                }
            break;

        case SelectionMode::Triangle:
            for (auto kvp: world.selection) {
                std::set<int> unique_vertices{};
                for (auto i: kvp.second) {
                    unique_vertices.insert(kvp.first->triangle_indexes[i*3  ]);
                    unique_vertices.insert(kvp.first->triangle_indexes[i*3+1]);
                    unique_vertices.insert(kvp.first->triangle_indexes[i*3+2]);
                }
                for (auto i: unique_vertices) {
                    auto &p = kvp.first->vertices.at(i);
                    p = ray::Vector3Add(p, input);
                }
            }
            break;

        case SelectionMode::Object:
            switch (editing) {
                case 0:
                    debug_text("editing position");
                    break;
                case 1:
                    debug_text("editing rotation");
                    break;
                case 2:
                    debug_text("editing scale");
                    break;
            }

            for (auto kvp: world.selection) {
                Object *object = kvp.first;

                switch (editing) {
                    case 0:
                        object->position = ray::Vector3Add(object->position, input);
                        break;
                    case 1:
                        object->rotation = ray::QuaternionMultiply(ray::QuaternionFromEuler(input.x, input.y, input.z), object->rotation);
                        break;
                    case 2:
                        object->scale = ray::Vector3Add(object->scale, input);
                        break;
                }
            }
            break;
    }
}