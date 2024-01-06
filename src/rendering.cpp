#include <algorithm>
#include <cassert>
#include "rendering.h"
#include "misc.h"

/// DEBUG
int original = 0, culled = 0, nodes = 0, sliced = 0, discarded = 0, drawn = 0;

float Plane::distance(ray::Vector3 point) {
    return -(ray::Vector3DotProduct(point,normal) + offset);
}

ray::Vector3 Plane::line_intersection(ray::Vector3 a, ray::Vector3 b) {
    ray::Vector3 slope = ray::Vector3Normalize(ray::Vector3Subtract(b,a));
    float t = -(offset + ray::Vector3DotProduct(a, normal)) / (ray::Vector3DotProduct(slope, normal));
    return ray::Vector3Add(a,ray::Vector3Scale(slope,t));
}

Plane Triangle::get_plane() {
    auto cross = ray::Vector3CrossProduct(
            ray::Vector3Subtract(v2, v0), // C - A
            ray::Vector3Subtract(v1, v0) // B - A
    );

    auto normal = ray::Vector3Normalize(cross);

    return Plane {
        .normal = normal,
        // Ax + By + Cz + offset = 0 <=> offset = -(Ax + By + Cz)
        .offset = -(normal.x * v0.x + normal.y * v0.y + normal.z * v0.z)
    };
}

Triangle Triangle::ccw() {
    if((v1.x - v0.x) * (v2.y - v0.y) - (v2.x - v0.x) * (v1.y - v0.y) < 0)
        return *this;
    return Triangle{.v0=v0, .v1=v2, .v2=v1, .col = col};
}

BSPNode::BSPNode(Triangle trig, Plane plane) {
    this->plane = plane;

    trig_vertices = {trig.v0,trig.v1,trig.v2};
    trig_colors = {trig.col};
    nodes++;
}

void BSPNode::add_above(Triangle trig, Plane p) {
    if (above) above->add(trig,p); else above = std::make_unique<BSPNode>(trig,p);
}

void BSPNode::add_below(Triangle trig, Plane p) {
    if (below) below->add(trig,p); else below = std::make_unique<BSPNode>(trig,p);
}

void BSPNode::add(Triangle trig, Plane p) {
    if (ray::Vector3Equals(this->plane.normal, p.normal)) {
        // triangles are on parallel planes
        if (this->plane.offset > p.offset)
            // triangle is in front
            add_above(trig, p);
        else if (this->plane.offset < p.offset)
            // triangle is behind
            add_below(trig, p);
        else {
            // triangle is on the same plane
            trig_vertices.push_back(trig.v0);
            trig_vertices.push_back(trig.v1);
            trig_vertices.push_back(trig.v2);
            trig_colors.push_back(trig.col);
        }
    } else {
        float v0_dist = plane.distance(trig.v0);
        float v1_dist = plane.distance(trig.v1);
        float v2_dist = plane.distance(trig.v2);

        if (v0_dist >= 0 && v1_dist >= 0 && v2_dist >= 0)
            // triangle is v_above plane
            return add_above(trig, p);

        if (v0_dist <= 0 && v1_dist <= 0 && v2_dist <= 0)
            // triangle is v_below plane
            return add_below(trig, p);

        sliced++;

        int v_above=0,v_below=0, v_zero=0;
        ray::Vector3 above_arr[3], below_arr[3], zero;
        if (v0_dist > 0) above_arr[v_above++]=trig.v0; else if (v0_dist < 0) below_arr[v_below++]=trig.v0; else zero=trig.v0,v_zero++;
        if (v1_dist > 0) above_arr[v_above++]=trig.v1; else if (v1_dist < 0) below_arr[v_below++]=trig.v1; else zero=trig.v1,v_zero++;
        if (v2_dist > 0) above_arr[v_above++]=trig.v2; else if (v2_dist < 0) below_arr[v_below++]=trig.v2; else zero=trig.v2,v_zero++;

        if(v_above == 3 || v_below == 3 || v_above + v_below + v_zero != 3 || v_zero > 1) {
            // something is wrong, discarding triangle
            debug_text(ray::TextFormat("error dist:%s v_nr:%d %d %d", v3_to_text(ray::Vector3 {v0_dist, v1_dist, v2_dist}), v_above, v_below, v_zero), ray::RED);
            discarded++;
            return;
        }

        ray::Vector3 intersection_a, intersection_b;
        if (v_zero == 1) {
            // handling special case, one point is on the plane while one is above and another is below
            intersection_a = plane.line_intersection(above_arr[0], below_arr[0]);
            if (!ray::Vector3Finite(intersection_a)) {
                discarded++;return;
            }
            add_above(Triangle {
                    .v0 = above_arr[0], .v1 = zero, .v2 = intersection_a, .col = trig.col,
            }.ccw(), p);
            add_below(Triangle {
                    .v0 = below_arr[0], .v1 = zero, .v2 = intersection_a, .col = trig.col,
            }.ccw(), p);
        } else if (v_above == 1) {
            //trapezoid is v_below
            intersection_a = plane.line_intersection(above_arr[0], below_arr[0]);
            intersection_b = plane.line_intersection(above_arr[0], below_arr[1]);
            if (!ray::Vector3Finite(intersection_a) || !ray::Vector3Finite(intersection_b)) {
                discarded++;return;
            }
            add_above(Triangle {
                .v0 = above_arr[0], .v1 = intersection_a, .v2 = intersection_b, .col = trig.col,
            }.ccw(), p);
            add_below(Triangle {
                    .v0 = below_arr[0], .v1 = intersection_a, .v2 = intersection_b, .col = trig.col,
            }.ccw(), p);
            add_below(Triangle {
                    .v0 = below_arr[0], .v1 = below_arr[1], .v2 = intersection_b, .col = trig.col,
            }.ccw(), p);
        } else {
            //trapezoid is v_above
            intersection_a = plane.line_intersection(above_arr[0], below_arr[0]);
            intersection_b = plane.line_intersection(above_arr[1], below_arr[0]);
            if (!ray::Vector3Finite(intersection_a) || !ray::Vector3Finite(intersection_b)) {
                discarded++;return;
            }
            add_below(Triangle {
                    .v0 = below_arr[0], .v1 = intersection_a, .v2 = intersection_b, .col = trig.col,
            }.ccw(), p);
            add_above(Triangle {
                    .v0 = above_arr[0], .v1 = intersection_a, .v2 = intersection_b, .col = trig.col,
            }.ccw(), p);
            add_above(Triangle {
                    .v0 = above_arr[0], .v1 = above_arr[1], .v2 = intersection_b, .col = trig.col,
            }.ccw(), p);
        }
        //debug_text(ray::TextFormat("%s %f %s %f",v3_to_text(intersection_a), plane.distance(intersection_a), v3_to_text(intersection_b), plane.distance(intersection_b)));
    }
}

void BSPNode::draw() {
    if (below) below->draw();

    int len = trig_colors.size();
    drawn += len;
    for (int i=0;i<len;i++) {
        ray::DrawTriangle(
                clip_to_screen_space(trig_vertices[i*3  ]),
                clip_to_screen_space(trig_vertices[i*3+1]),
                clip_to_screen_space(trig_vertices[i*3+2]),
                trig_colors[i]);
    }

    if (above) above->draw();
}

void BSPNode::draw_debug(int previous) {
    if (below) below->draw_debug(-1);

    ray::Color col = (previous > 0 ? ray::RED : (previous < 0 ? ray::BLUE : ray::LIME));

    int len = trig_colors.size();
    drawn += len;
    for (int i=0;i<len;i++) {
        auto rand_col = ray::ColorBrightness(col, ray::GetRandomValue(-25, 25) / 100.);
        ray::DrawTriangle(
                clip_to_screen_space(trig_vertices[i*3  ]),
                clip_to_screen_space(trig_vertices[i*3+1]),
                clip_to_screen_space(trig_vertices[i*3+2]),
                rand_col);
    }

    if (above) above->draw_debug(1);
}

bool Renderer::cull_or_add_to_bsp_tree(Triangle trig) {
    original++;
    auto aabb_min = ray::Vector3Min(trig.v0, ray::Vector3Min(trig.v1,trig.v2));
    auto aabb_max = ray::Vector3Max(trig.v0, ray::Vector3Max(trig.v1,trig.v2));

    // clip space culling
    if (aabb_min.z > 0 || aabb_max.x < -1 || aabb_min.x > 1 || aabb_max.y < -1 || aabb_min.y > 1) {
        culled++; return false;
    }

    Plane plane = trig.get_plane();

    // CW winding culling
    if (ray::Vector3DotProduct(plane.normal,ray::Vector3{0,0,1}) <= 0) {
        culled++; return false;
    }

    if (root) {
        root->add(trig, plane);
    } else {
        root = std::make_unique<BSPNode>(trig, plane);
    }
    return true;
}

void Renderer::draw(bool debug) {
    if (!root) return;

    if (debug) {
        debug_text(ray::TextFormat("renderer: original=%d culled=%d nodes=%d", original, culled, nodes));
        debug_text(ray::TextFormat("sliced=%d discarded=%d drawn=%d", sliced, discarded, drawn));
        original = 0, culled = 0, nodes = 0, sliced = 0, discarded = 0, drawn = 0;
        ray::SetRandomSeed(69); // prevent flickering
        root->draw_debug(0);
    } else
        root->draw();
}

ray::Vector2 clip_to_screen_space(ray::Vector3 v3) {
    return ray::Vector2 {
            (v3.x + 1) * screenWidth / (float) 2,
            (v3.y + 1) * screenHeight / (float) 2,
    };
}