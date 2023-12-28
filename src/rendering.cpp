#include <algorithm>
#include <iostream>
#include "rendering.h"
#include "misc.h"

RenderTrig::RenderTrig(const ray::Vector3 v0, const ray::Vector3 v1, const ray::Vector3 v2, ray::Color c) {
    vertices[0] = v0;
    vertices[1] = v1;
    vertices[2] = v2;

    aabb_min = ray::Vector3Min(v0, ray::Vector3Min(v1,v2));
    aabb_max = ray::Vector3Max(v0, ray::Vector3Max(v1,v2));

    cross = ray::Vector3CrossProduct(
            ray::Vector3Subtract(v2, v0),
            ray::Vector3Subtract(v1, v0)
    );

    col = c;
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
    //ray::DrawText(ray::TextFormat("intersections: %u", intersections), 20, 80, 10, ray::RED);
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