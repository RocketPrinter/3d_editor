#include "object.h"
#include "misc.h"

void test_config(World &world);

int main()
{
    ray::InitWindow(screenWidth, screenHeight, "Editor 3D");
    ray::SetTargetFPS(60);

    World world{};
    world.objects.push_back(Object::new_cube());
    // Main game loop
    while (!ray::WindowShouldClose())
    {
        ray::BeginDrawing();
        ClearBackground(ray::RAYWHITE);

        if (ray::IsKeyPressed(ray::KEY_SLASH)) world.debug_render = !world.debug_render;
        if (ray::IsMouseButtonDown(ray::MOUSE_BUTTON_LEFT) && !ray::IsKeyDown(ray::KEY_LEFT_SHIFT)) world.raycast_and_add_to_selection(ray::GetMouseX(), ray::GetMouseY());
        if (ray::IsMouseButtonDown(ray::MOUSE_BUTTON_RIGHT)) world.raycast_and_add_to_selection(ray::GetMouseX(), ray::GetMouseY());
        world.camera.input_movement();

        world.render();
        test_config(world);

        reset_draw_debug();
        ray::DrawFPS(20,20);
        ray::EndDrawing();
    }

    ray::CloseWindow();

    return 0;
}

int editing=0, cube_index=0;
void test_config(World &world) {
    auto &cam = world.camera;
    Object &cube = world.objects[cube_index];

    if (ray::IsKeyPressed(ray::KEY_SPACE)) {
        editing = (editing + 1) % 3;
    }

    ray::Vector3 input{};
    if (ray::IsKeyDown(ray::KEY_J))input.x -= 0.02;
    if (ray::IsKeyDown(ray::KEY_L))input.x += 0.02;
    if (ray::IsKeyDown(ray::KEY_I))input.y -= 0.02;
    if (ray::IsKeyDown(ray::KEY_K))input.y += 0.02;
    if (ray::IsKeyDown(ray::KEY_U))input.z -= 0.02;
    if (ray::IsKeyDown(ray::KEY_O))input.z += 0.02;

    for (int i=0; i <= world.objects.size() && i < 10; i++) {
        if (ray::IsKeyPressed(ray::KEY_ZERO + i)) {
            if (i == world.objects.size()) {
                world.objects.push_back(Object::new_cube());
            }
            cube_index = i;
        }
    }

    switch (editing) {
        case 0:
            cube.position = ray::Vector3Add(cube.position, input);
            break;
        case 1:
            cube.rotation = ray::QuaternionMultiply(ray::QuaternionFromEuler(input.x,input.y,input.z), cube.rotation);
            break;
        case 2:
            cube.scale = ray::Vector3Add(cube.scale, input);
            break;
    }

    debug_text(ray::TextFormat("cube #%d position: %s", cube_index, v3_to_text(cube.position)),
               editing == 0 ? ray::GREEN : ray::GRAY);
    debug_text(ray::TextFormat("cube #%d rotation: %s", cube_index, v3_to_text(ray::QuaternionToEuler(cube.rotation))),
               editing == 1 ? ray::GREEN : ray::GRAY);
    debug_text(ray::TextFormat("cube #%d scale: %s", cube_index, v3_to_text(cube.scale)),
               editing == 2 ? ray::GREEN : ray::GRAY);
}