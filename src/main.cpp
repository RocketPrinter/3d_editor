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
    while (!ray::WindowShouldClose())    // Detect window close button or ESC key
    {
        ray::BeginDrawing();
        ClearBackground(ray::RAYWHITE);

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
        editing = (editing + 1) % 7;
    }

    ray::Vector3 input{};
    if (ray::IsKeyDown(ray::KEY_A))input.x -= 0.02;
    if (ray::IsKeyDown(ray::KEY_D))input.x += 0.02;
    if (ray::IsKeyDown(ray::KEY_W))input.y -= 0.02;
    if (ray::IsKeyDown(ray::KEY_S))input.y += 0.02;
    if (ray::IsKeyDown(ray::KEY_Q))input.z -= 0.02;
    if (ray::IsKeyDown(ray::KEY_E))input.z += 0.02;

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
            cam.position = ray::Vector3Add(cam.position, input);
            break;
        case 1:
            cam.target = ray::Vector3Add(cam.target, input);
            break;
        case 2:
            cam.up = ray::Vector3Add(cam.up, input);
            break;
        case 3:
            cam.fov += input.x;
            break;
        case 4:
            cube.position = ray::Vector3Add(cube.position, input);
            break;
        case 5:
            cube.rotation = ray::QuaternionMultiply(ray::QuaternionFromEuler(input.x,input.y,input.z), cube.rotation);
            break;
        case 6:
            cube.scale = ray::Vector3Add(cube.scale, input);
            break;
    }

    debug_text(ray::TextFormat("cam position: %s", v3_to_text(cam.position)), editing == 0 ? ray::GREEN : ray::GRAY);
    debug_text(ray::TextFormat("cam target: %s", v3_to_text(cam.target)), editing == 1 ? ray::GREEN : ray::GRAY);
    debug_text(ray::TextFormat("cam up: %s", v3_to_text(cam.up)), editing == 2 ? ray::GREEN : ray::GRAY);
    debug_text(ray::TextFormat("cam fov: %f", cam.fov * 180. / PI), editing == 3 ? ray::GREEN : ray::GRAY);

    debug_text(ray::TextFormat("cube #%d position: %s", cube_index, v3_to_text(cube.position)),
               editing == 4 ? ray::GREEN : ray::GRAY);
    debug_text(ray::TextFormat("cube #%d rotation: %s", cube_index, v3_to_text(ray::QuaternionToEuler(cube.rotation))),
               editing == 5 ? ray::GREEN : ray::GRAY);
    debug_text(ray::TextFormat("cube #%d scale: %s", cube_index, v3_to_text(cube.scale)),
               editing == 6 ? ray::GREEN : ray::GRAY);
}