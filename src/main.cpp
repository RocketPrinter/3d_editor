#include <set>
#include "object.h"
#include "serialization.h"
#include "menu.h"
#define RAYGUI_IMPLEMENTATION
#include "lib/raygui.h"
#include "misc.h"

void handle_input(World &world);

int main()
{
    ray::InitWindow(screenWidth, screenHeight, "Editor 3D");
    ray::SetTargetFPS(60);

    World world;

    Menu menu{};
    menu.world = &world;
    world.menu = &menu;

    if (not deserialize(world)) {
        world.objects.push_back(Object::new_cube());
    }

    for(Object* obj : world.objects){
        menu.addToMenu(obj);
    }

    // Main game loop
    while (!ray::WindowShouldClose())
    {
        ray::BeginDrawing();
        ClearBackground(ray::RAYWHITE);

        handle_input(world);

        world.render();

        menu.showMenu();

        reset_draw_debug();
        ray::EndDrawing();
    }

    ray::CloseWindow();

    return 0;
}

void handle_input(World &world) {

    world.camera.input_movement();

    // adding and removing from selection
    if (ray::IsMouseButtonDown(ray::MOUSE_BUTTON_LEFT) && !ray::IsKeyDown(ray::KEY_LEFT_SHIFT)) world.raycast_and_modify_selection(false);
    if (ray::IsMouseButtonDown(ray::MOUSE_BUTTON_RIGHT)) world.raycast_and_modify_selection(true);

    // changing selection mode
    if (ray::IsKeyPressed(ray::KEY_ONE))
        world.selection_mode = SelectionMode::Vertex,   world.menu->menuActions[2]->text = "   Sel mode: vertex",   world.selection.clear();
    if (ray::IsKeyPressed(ray::KEY_TWO))
        world.selection_mode = SelectionMode::Triangle, world.menu->menuActions[2]->text = "   Sel mode: triangle", world.selection.clear();
    if (ray::IsKeyPressed(ray::KEY_THREE))
        world.selection_mode = SelectionMode::Object,   world.menu->menuActions[2]->text = "   Sel mode: object",   world.selection.clear();

    // changing operation
    if (world.selection_mode == SelectionMode::Object) {
        if (ray::IsKeyPressed(ray::KEY_FOUR))
            world.operation = Operation::Translate, world.menu->menuActions[2]->text = "   Op: translate", world.selection.clear();
        if (ray::IsKeyPressed(ray::KEY_FIVE))
            world.operation = Operation::Rotate,    world.menu->menuActions[2]->text = "   Op: rotate",    world.selection.clear();
        if (ray::IsKeyPressed(ray::KEY_SIX))
            world.operation = Operation::Scale,     world.menu->menuActions[2]->text = "   Op: scale",     world.selection.clear();
    }

    // clearing selection
    if (ray::IsKeyPressed(ray::KEY_Q))
        world.selection.clear();

    // debug mode
    if (ray::IsKeyPressed(ray::KEY_SLASH)) world.debug_render = !world.debug_render;

    // ijklp;
    ray::Vector3 input3d{};
    if (ray::IsKeyDown(ray::KEY_J))input3d.x -= 0.02;
    if (ray::IsKeyDown(ray::KEY_L))input3d.x += 0.02;
    if (ray::IsKeyDown(ray::KEY_SEMICOLON))input3d.y -= 0.02;
    if (ray::IsKeyDown(ray::KEY_P))input3d.y += 0.02;
    if (ray::IsKeyDown(ray::KEY_I))input3d.z -= 0.02;
    if (ray::IsKeyDown(ray::KEY_K))input3d.z += 0.02;

    switch (world.selection_mode) {
        case SelectionMode::Vertex:
            for (auto kvp: world.selection)
                for (auto i: kvp.second) {
                    auto &p = kvp.first->vertices[i];
                    p = ray::Vector3Add(p, input3d);
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
                    p = ray::Vector3Add(p, input3d);
                }
            }
            break;

        case SelectionMode::Object:
            for (auto kvp: world.selection) {
                Object *object = kvp.first;

                debug_text(object->name.c_str());
                debug_text(ray::TextFormat("position: %s", v3_to_text(object->position)));
                debug_text(ray::TextFormat("rotation: %s", v3_to_text(ray::QuaternionToEuler(object->rotation))));
                debug_text(ray::TextFormat("scale: %s", v3_to_text(object->scale)));

                switch (world.operation) {
                    case Operation::Translate:
                        object->position = ray::Vector3Add(object->position, input3d);
                        break;
                    case Operation::Rotate:
                        object->rotation = ray::QuaternionMultiply(ray::QuaternionFromEuler(input3d.x, input3d.y, input3d.z), object->rotation);
                        break;
                    case Operation::Scale:
                        object->scale = ray::Vector3Add(object->scale, input3d);
                        break;
                }
            }
            break;
    }
}