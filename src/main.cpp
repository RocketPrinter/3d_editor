#include <vector>
#include <iostream>
#include "misc.h"
#include "rendering.h"
#include "object.h"

void test_render(World &world);
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

        test_render(world);
        test_config(world);

        reset_draw_debug();
        ray::DrawFPS(20,20);
        ray::EndDrawing();
    }

    ray::CloseWindow();

    return 0;
}

void test_render(World &world) {
    std::vector<RenderTrig> render_trigs;

    Object &cube = world.objects[0];

    // transformations are multiplied left to right
    ray::Matrix transformations = ray::MatrixIdentity();
    // model matrix of cube
    transformations = ray::MatrixMultiply(transformations, cube.get_model_matrix());
    // view + projection
    transformations = ray::MatrixMultiply(transformations, world.camera.get_view_projection_matrix());


    std::vector<ray::Vector3> transformed_vertices{};
    for (auto v: cube.vertices) {
        // homogeneous coordinates
        ray::Vector4 v4 = ray::Vector4Transform(ray::Vector4FromVector3(v, 1.), transformations);
        // perspective divide/normalization
        ray::Vector3 result = ray::Vector3{v4.x/v4.w, v4.y/v4.w, v4.z/v4.w};
        transformed_vertices.push_back(result);
    }

    int trig_count = cube.triangle_indexes.size() / 3;
    for (int i=0;i<trig_count;i++) {
        render_trigs.push_back(RenderTrig{transformed_vertices[cube.triangle_indexes[i*3]], transformed_vertices[cube.triangle_indexes[i*3 + 1]], transformed_vertices[cube.triangle_indexes[i*3 + 2]], cube.triangle_colors[i]});
    }

    render(render_trigs, true);

    draw_debug(ray::TextFormat("%s becomes %s",v3_to_text(cube.vertices[3]),v3_to_text(transformed_vertices[3])));
    draw_debug(ray::TextFormat("%s becomes %s",v3_to_text(cube.vertices[4]),v3_to_text(transformed_vertices[4])));
}

int editing=0;
void test_config(World &world) {
    auto &cam = world.camera;
    Object &cube = world.objects[0];

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

    draw_debug(ray::TextFormat("cam position: %s", v3_to_text(cam.position)), editing==0?ray::GREEN:ray::GRAY);
    draw_debug(ray::TextFormat("cam target: %s", v3_to_text(cam.target)), editing==1?ray::GREEN:ray::GRAY);
    draw_debug(ray::TextFormat("cam up: %s", v3_to_text(cam.up)), editing==2?ray::GREEN:ray::GRAY);
    draw_debug(ray::TextFormat("cam fov: %f", cam.fov*180./PI), editing==3?ray::GREEN:ray::GRAY);

    draw_debug(ray::TextFormat("cube position: %s", v3_to_text(cube.position)), editing==4?ray::GREEN:ray::GRAY);
    draw_debug(ray::TextFormat("cube rotation: %s", v3_to_text(ray::QuaternionToEuler(cube.rotation))), editing==5?ray::GREEN:ray::GRAY);
    draw_debug(ray::TextFormat("cube scale: %s", v3_to_text(cube.scale)), editing==6?ray::GREEN:ray::GRAY);
}