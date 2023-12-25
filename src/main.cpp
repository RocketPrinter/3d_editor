#include <vector>
#include "misc.h"
#include "rendering.h"

void update_draw_frame();     // Update and Draw one frame
void test_render();

int main()
{
    ray::InitWindow(screenWidth, screenHeight, "Editor 3D");

    ray::SetTargetFPS(60);

    // Main game loop
    while (!ray::WindowShouldClose())    // Detect window close button or ESC key
    {
        update_draw_frame();
    }

    ray::CloseWindow();

    return 0;
}

void update_draw_frame()
{

    ray::BeginDrawing();

    ClearBackground(ray::RAYWHITE);

    test_render();

    ray::EndDrawing();
}

struct Trig {
    // counter-clockwise winding order
    ray::Vector3 vertices[3]{};
    ray::Color col{};
};

ray::Vector3 translation{}, rotation{}, scale{1,1,1}; int editing=1;

void test_render() {
    {
        if (ray::IsKeyPressed(ray::KEY_SPACE))
            editing = (editing + 1) % 3;

        ray::Vector3 &edit = (editing==0?translation:(editing==1?rotation:scale));
        if (ray::IsKeyDown(ray::KEY_S))edit.x -= 0.02;
        if (ray::IsKeyDown(ray::KEY_W))edit.x += 0.02;
        if (ray::IsKeyDown(ray::KEY_A))edit.y -= 0.02;
        if (ray::IsKeyDown(ray::KEY_D))edit.y += 0.02;
        if (ray::IsKeyDown(ray::KEY_Q))edit.z -= 0.02;
        if (ray::IsKeyDown(ray::KEY_E))edit.z += 0.02;
    }

    std::vector<Trig> cube {
        Trig{ray::Vector3{-0.5, -0.5, 0.5},ray::Vector3{0.5, 0.5, 0.5},ray::Vector3{0.5, -0.5, 0.5}, ray::MAROON},
        Trig{ray::Vector3{-0.5, -0.5, 0.5},ray::Vector3{-0.5, 0.5, 0.5}, ray::Vector3{0.5, 0.5, 0.5}, ray::RED},

        Trig{ray::Vector3{0.5, -0.5, 0.5},ray::Vector3{0.5, 0.5, -0.5},ray::Vector3{0.5, -0.5, -0.5}, ray::YELLOW},
        Trig{ray::Vector3{0.5, -0.5, 0.5},ray::Vector3{0.5, 0.5, 0.5},ray::Vector3{0.5, 0.5, -0.5}, ray::GOLD},

        Trig{ray::Vector3{-0.5, -0.5, 0.5},ray::Vector3{0.5, -0.5, -0.5},ray::Vector3{-0.5, -0.5, -0.5}, ray::DARKBLUE},
        Trig{ray::Vector3{-0.5, -0.5, 0.5},ray::Vector3{0.5, -0.5, 0.5},ray::Vector3{0.5, -0.5, -0.5}, ray::BLUE},

        Trig{ray::Vector3{-0.5, -0.5, -0.5},ray::Vector3{0.5, -0.5, -0.5},ray::Vector3{0.5, 0.5, -0.5}, ray::GREEN},
        Trig{ray::Vector3{-0.5, -0.5, -0.5},ray::Vector3{0.5, 0.5, -0.5},ray::Vector3{-0.5, 0.5, -0.5}, ray::LIME},

        Trig{ray::Vector3{-0.5, -0.5, 0.5},ray::Vector3{-0.5, -0.5, -0.5},ray::Vector3{-0.5, 0.5, -0.5}, ray::PURPLE},
        Trig{ray::Vector3{-0.5, -0.5, 0.5},ray::Vector3{-0.5, 0.5, -0.5}, ray::Vector3{-0.5, 0.5, 0.5}, ray::VIOLET},

        Trig{ray::Vector3{-0.5, 0.5, 0.5},ray::Vector3{-0.5, 0.5, -0.5},ray::Vector3{0.5, 0.5, -0.5}, ray::BROWN},
        Trig{ray::Vector3{-0.5, 0.5, 0.5},ray::Vector3{0.5, 0.5, -0.5}, ray::Vector3{0.5, 0.5, 0.5}, ray::BEIGE},
    };

    std::vector<RenderTrig> render_trigs;

    // transformations are multiplied left to right
    ray::Matrix transformations = ray::MatrixIdentity();

    // local scale
    transformations = ray::MatrixMultiply(transformations, ray::MatrixScale(scale.x, scale.y, scale.z));
    // local rotation
    transformations = ray::MatrixMultiply(transformations,  ray::MatrixRotateXYZ(rotation));
    // aspect ration fix
    transformations = ray::MatrixMultiply(transformations, ray::MatrixScale(screenHeight / (float) screenWidth,1., 1.));
    // global translation
    transformations = ray::MatrixMultiply(transformations, ray::MatrixTranslate(translation.y, -translation.x, translation.z-1));

    for (Trig &t: cube) {
        for (int i=0;i<3;i++) {
            t.vertices[i] = ray::Vector3Transform(t.vertices[i], transformations);
        }
        render_trigs.push_back(RenderTrig(t.vertices[0],t.vertices[1],t.vertices[2], t.col));
    }

    render(render_trigs, true);

    ray::DrawText(ray::TextFormat("cube translation: %s", v3_to_text(translation)), 20, 380, 10, editing==0?ray::GREEN:ray::GRAY);
    ray::DrawText(ray::TextFormat("cube rotation: %s", v3_to_text(rotation)), 20, 400, 10, editing==1?ray::GREEN:ray::GRAY);
    ray::DrawText(ray::TextFormat("cube scale: %s", v3_to_text(scale)), 20, 420, 10, editing==2?ray::GREEN:ray::GRAY);
}