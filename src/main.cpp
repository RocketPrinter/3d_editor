#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>
namespace ray {
    #include "raylib.h"
    #include "raymath.h"
}

const int screenWidth = 800;
const int screenHeight = 450;

void update_draw_frame();     // Update and Draw one frame
void test_render();
const char* v3_to_text(ray::Vector3 v3);
const char* trig_to_text(ray::Vector3 trig[3]);

int main()
{
    ray::InitWindow(screenWidth, screenHeight, "Editor 3Dws");

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

struct RenderTrig {
    // counter-clockwise winding order
    ray::Vector3 vertices[3]{};
    ray::Color col{};

    // axis aligned bounding box
    ray::Vector3 aabb_min{}, aabb_max{};
    // (C-A)X(B-A)
    ray::Vector3 cross{};

    // triangle dependency tree
    bool drawn = false;
    std::vector<int> draw_dependencies{};

    RenderTrig() = default;

    RenderTrig(const ray::Vector3 v[3], ray::Color c) {
        vertices[0] = v[0];
        vertices[1] = v[1];
        vertices[2] = v[2];

        aabb_min = ray::Vector3Min(v[0], ray::Vector3Min(v[1],v[2]));
        aabb_max = ray::Vector3Max(v[0], ray::Vector3Max(v[1],v[2]));

        cross = ray::Vector3CrossProduct(
                    ray::Vector3Subtract(v[2], v[0]),
                    ray::Vector3Subtract(v[1], v[0])
                );

        col = c;
    }
};

void render(std::vector<RenderTrig> trigs, bool draw_stats);

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
        Trig{{ray::Vector3{-0.5, -0.5, 0.5},ray::Vector3{0.5, 0.5, 0.5},ray::Vector3{0.5, -0.5, 0.5}, }, ray::MAROON},
        Trig{{ray::Vector3{-0.5, -0.5, 0.5},ray::Vector3{-0.5, 0.5, 0.5}, ray::Vector3{0.5, 0.5, 0.5},  }, ray::RED},

        Trig{{ray::Vector3{0.5, -0.5, 0.5},ray::Vector3{0.5, 0.5, -0.5},ray::Vector3{0.5, -0.5, -0.5}, }, ray::YELLOW},
        Trig{{ray::Vector3{0.5, -0.5, 0.5},ray::Vector3{0.5, 0.5, 0.5},ray::Vector3{0.5, 0.5, -0.5}, }, ray::GOLD},

        Trig{{ray::Vector3{-0.5, -0.5, 0.5},ray::Vector3{0.5, -0.5, -0.5},ray::Vector3{-0.5, -0.5, -0.5}, }, ray::DARKBLUE},
        Trig{{ray::Vector3{-0.5, -0.5, 0.5},ray::Vector3{0.5, -0.5, 0.5},ray::Vector3{0.5, -0.5, -0.5}, }, ray::BLUE},

        Trig{{ray::Vector3{-0.5, -0.5, -0.5},ray::Vector3{0.5, -0.5, -0.5},ray::Vector3{0.5, 0.5, -0.5}, },  ray::GREEN},
        Trig{{ray::Vector3{-0.5, -0.5, -0.5},ray::Vector3{0.5, 0.5, -0.5},ray::Vector3{-0.5, 0.5, -0.5},  }, ray::LIME},

        Trig{{ray::Vector3{-0.5, -0.5, 0.5},ray::Vector3{-0.5, -0.5, -0.5},ray::Vector3{-0.5, 0.5, -0.5}, }, ray::PURPLE},
        Trig{{ray::Vector3{-0.5, -0.5, 0.5},ray::Vector3{-0.5, 0.5, -0.5}, ray::Vector3{-0.5, 0.5, 0.5}, }, ray::VIOLET},

        Trig{{ray::Vector3{-0.5, 0.5, 0.5},ray::Vector3{-0.5, 0.5, -0.5},ray::Vector3{0.5, 0.5, -0.5}, }, ray::BROWN},
        Trig{{ray::Vector3{-0.5, 0.5, 0.5},ray::Vector3{0.5, 0.5, -0.5}, ray::Vector3{0.5, 0.5, 0.5}, }, ray::BEIGE},
    };

    std::vector<RenderTrig> render_trigs;
    // for some reason transformations are multiplied left to right

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
        render_trigs.push_back(RenderTrig(t.vertices, t.col));
    }

    render(render_trigs, true);

    ray::DrawText(ray::TextFormat("cube translation: %s", v3_to_text(translation)), 20, 380, 10, editing==0?ray::GREEN:ray::GRAY);
    ray::DrawText(ray::TextFormat("cube rotation: %s", v3_to_text(rotation)), 20, 400, 10, editing==1?ray::GREEN:ray::GRAY);
    ray::DrawText(ray::TextFormat("cube scale: %s", v3_to_text(scale)), 20, 420, 10, editing==2?ray::GREEN:ray::GRAY);
}

void _render_recursively(std::vector<RenderTrig> &trigs, int index);

// uses painter's algorithm
// assumes that no triangles intersect nor there are "cycles" where t1 covers t2 which covers t3 which covers t1
// for this purpose clip space is defined as: [-1, 1] on x and y axis and [-inf, 0] on z axis
void render(std::vector<RenderTrig> trigs, bool draw_stats) {
    int clipped_triangles = 0, aabb_intersections = 0,  intersections = 0, dependencies = 0;

    // culls triangles that are outside the clip area or their front face isn't visible, O(n)
    {
        int j = 0;
        for (int i=0; i < trigs.size(); i++, j++) {
            RenderTrig &trig = trigs[i];

            if (// clip space culling
                trig.aabb_min.z > 0 || trig.aabb_max.x < -1 || trig.aabb_min.x > 1 || trig.aabb_max.y < -1 || trig.aabb_min.y > 1 ||
                // CW winding order culling
                ray::Vector3DotProduct(trig.cross,ray::Vector3{0,0,1}) <= 0
                ) {
                // triangle is culled
                j--, clipped_triangles++;
            }
            else if (i != j) {
                // triangle is shifted left
                trigs [j] = trigs [i];
            }
        }

        trigs.resize(j);
    }

    // sorts triangles based on their minimum z coordinate, optimises the next step
    std::sort(trigs.begin(), trigs.end(), [](const RenderTrig &a, const RenderTrig &b) {
        return a.aabb_min.z < b.aabb_min.z;
    });

    // z interval intersections
    for (int i=0; i < trigs.size(); i++) {
        RenderTrig &trig = trigs[i];

        for (int j=i+1; j < trigs.size(); j++) {
            RenderTrig &other_trig = trigs[j];
            if (other_trig.aabb_min.z > other_trig.aabb_max.z)
                break;

            // aabb test
            if (!(trig.aabb_max.z > other_trig.aabb_min.z
            && (trig.aabb_max.x > other_trig.aabb_min.x && other_trig.aabb_max.x > trig.aabb_min.x)
            && (trig.aabb_max.y > other_trig.aabb_min.y && other_trig.aabb_max.y > trig.aabb_min.y)
            )) continue;
            aabb_intersections++;

            // determining if other_trig is above or below trig
            // todo: not sufficient
            int front = 0, behind = 0;
            for (int k=0;k<3;k++) {
                float dot = ray::Vector3DotProduct(trig.cross, ray::Vector3Subtract(other_trig.vertices[k],trig.vertices[0]));
                if (dot > 0) front++;
                else if (dot < 0) behind++;
            }
            std::cout << '\n';

            if (front == 3) {
                dependencies++;
                // other_trig is in front of trig
                other_trig.draw_dependencies.push_back(i);

            } else if (behind == 3) {
                dependencies++;
                // trig is in front of other_trig
                trig.draw_dependencies.push_back(j);
            } else {
                // triangles must be intersecting, not good!
                intersections++;
                //std::cout << "intersection between i=" << i << " " << trig_to_text(trig.vertices) <<  " and " << " j=" << j << " " <<  trig_to_text(other_trig.vertices)  << " front=" << front << " behind=" << behind << '\n';
            }
        }
    }

    // actual rendering using the dependency tree
    for (int i = 0; i< trigs.size(); i++) {
        _render_recursively(trigs, i);
    }

    // draw statistics
    if (!draw_stats) return;

    ray::DrawText(ray::TextFormat("clipped triangles: %u", clipped_triangles), 20, 20, 10, ray::GRAY);
    ray::DrawText(ray::TextFormat("aabb_intersections: %u", aabb_intersections), 20, 40, 10, ray::GRAY);
    ray::DrawText(ray::TextFormat("dependencies: %u", dependencies), 20, 60, 10, ray::GRAY);
    ray::DrawText(ray::TextFormat("intersections: %u", intersections), 20, 80, 10, ray::RED);
}

ray::Vector2 clip_to_screen_space(ray::Vector3 v3) {
    return ray::Vector2 {
            (v3.x + 1) * screenWidth / 2,
            (v3.y + 1) * screenHeight / 2,
    };
}

void _render_recursively(std::vector<RenderTrig> &trigs, int index) {
    RenderTrig &trig = trigs[index];
    if (trig.drawn) return;
    trig.drawn = true; // setting it before drawing dependencies will cause draw artifacts instead of infinite loops in the case of a dependency cycle

    for(int dep: trig.draw_dependencies)
        _render_recursively(trigs, dep);

    ray::DrawTriangle(
            clip_to_screen_space(trig.vertices[0]),
            clip_to_screen_space(trig.vertices[1]),
            clip_to_screen_space(trig.vertices[2]),
            trig.col);
}

const char* v3_to_text(ray::Vector3 v3) {
    return ray::TextFormat("(%f,%f,%f)", v3.x, v3.y, v3.z);
}

const char* trig_to_text(ray::Vector3 trig[3]) {
    return ray::TextFormat("{(%f,%f,%f),(%f,%f,%f),(%f,%f,%f)}", trig[0].x, trig[0].y, trig[0].z, trig[1].x, trig[1].y, trig[1].z, trig[2].x, trig[2].y, trig[2].z);
}