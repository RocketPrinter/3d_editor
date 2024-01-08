#include <algorithm>
#include "rendering.h"
#include "misc.h"

/// DEBUG
int original = 0, culled = 0, nodes = 0, sliced = 0, discarded = 0, drawn = 0;

BSPNode::BSPNode(Triangle trig, Plane plane, ray::Color col) {
    this->plane = plane;

    trig_vertices = {trig.v0,trig.v1,trig.v2};
    trig_colors = {col};
    nodes++;
}

void BSPNode::add_above(Triangle trig, Plane p, ray::Color col) {
    if (above) above->add(trig, p, col); else above = std::make_unique<BSPNode>(trig,p, col);
}

void BSPNode::add_below(Triangle trig, Plane p, ray::Color col) {
    if (below) below->add(trig, p, col); else below = std::make_unique<BSPNode>(trig, p, col);
}

void BSPNode::add(Triangle trig, Plane p, ray::Color col) {
    if (ray::Vector3Equals(this->plane.normal, p.normal)) {
        // triangles are on parallel planes
        if (this->plane.offset > p.offset)
            // triangle is in front
            add_above(trig, p, col);
        else if (this->plane.offset < p.offset)
            // triangle is behind
            add_below(trig, p, col);
        else {
            // triangle is on the same plane
            trig_vertices.push_back(trig.v0);
            trig_vertices.push_back(trig.v1);
            trig_vertices.push_back(trig.v2);
            trig_colors.push_back(col);
        }
    } else {
        float v0_dist = plane.distance(trig.v0);
        float v1_dist = plane.distance(trig.v1);
        float v2_dist = plane.distance(trig.v2);

        if (v0_dist >= 0 && v1_dist >= 0 && v2_dist >= 0)
            // triangle is v_above plane
            return add_above(trig, p, col);

        if (v0_dist <= 0 && v1_dist <= 0 && v2_dist <= 0)
            // triangle is v_below plane
            return add_below(trig, p, col);

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

        if (v_zero == 1) {
            // handling special case, one point is on the plane while one is above and another is below

            Ray intersection_ray = Ray::from_points(above_arr[0], below_arr[0]);
            float t = plane.ray_intersection(intersection_ray);
            if (!std::isfinite(t)){discarded++; return;}
            auto intersection = intersection_ray.point_at_distance(t);
            add_above(Triangle {
                    .v0 = above_arr[0], .v1 = zero, .v2 = intersection,
            }.ccw(), p, col);
            add_below(Triangle {
                    .v0 = below_arr[0], .v1 = zero, .v2 = intersection,
            }.ccw(), p, col);
        } else if (v_above == 1) {
            //trapezoid is v_below

            Ray intersection_a_ray = Ray::from_points(above_arr[0], below_arr[0]);
            float ta = plane.ray_intersection(intersection_a_ray);
            Ray intersection_b_ray = Ray::from_points(above_arr[0], below_arr[1]);
            float tb = plane.ray_intersection(intersection_b_ray);
            if (!std::isfinite(ta) || !std::isfinite(tb)){discarded++; return;}
            auto intersection_a = intersection_a_ray.point_at_distance(ta);
            auto intersection_b = intersection_b_ray.point_at_distance(tb);

            add_above(Triangle {
                .v0 = above_arr[0], .v1 = intersection_a, .v2 = intersection_b,
            }.ccw(), p, col);
            add_below(Triangle {
                    .v0 = below_arr[0], .v1 = intersection_a, .v2 = intersection_b,
            }.ccw(), p, col);
            add_below(Triangle {
                    .v0 = below_arr[0], .v1 = below_arr[1], .v2 = intersection_b,
            }.ccw(), p, col);
        } else {
            //trapezoid is v_above

            Ray intersection_a_ray = Ray::from_points(above_arr[0], below_arr[0]);
            float ta = plane.ray_intersection(intersection_a_ray);
            Ray intersection_b_ray = Ray::from_points(above_arr[1], below_arr[0]);
            float tb = plane.ray_intersection(intersection_b_ray);
            if (!std::isfinite(ta) || !std::isfinite(tb)){discarded++; return;}
            auto intersection_a = intersection_a_ray.point_at_distance(ta);
            auto intersection_b = intersection_b_ray.point_at_distance(tb);

            add_below(Triangle {
                    .v0 = below_arr[0], .v1 = intersection_a, .v2 = intersection_b,
            }.ccw(), p, col);
            add_above(Triangle {
                    .v0 = above_arr[0], .v1 = intersection_a, .v2 = intersection_b,
            }.ccw(), p, col);
            add_above(Triangle {
                    .v0 = above_arr[0], .v1 = above_arr[1], .v2 = intersection_b,
            }.ccw(), p, col);
        }
        //debug_text(ray::TextFormat("%s %f %s %f",v3_to_text(intersection_a), plane.distance(intersection_a), v3_to_text(intersection_b), plane.distance(intersection_b)));
    }
}

void draw_triangle_if_valid(Triangle trig, ray::Color col) {
    ray::Vector2 v0 = clip_to_screen_space(trig.v0);
    ray::Vector2 v1 = clip_to_screen_space(trig.v1);
    ray::Vector2 v2 = clip_to_screen_space(trig.v2);
    // filters out NaN, -inf, +inf
    if (std::isfinite(v0.x) && std::isfinite(v0.y) &&
        std::isfinite(v1.x) && std::isfinite(v1.y) &&
        std::isfinite(v2.x) && std::isfinite(v2.y) ){
        drawn++;
        ray::DrawTriangle(v0,v1,v2,col);
    }
    else discarded++;
}

void BSPNode::draw() {
    if (below) below->draw();

    int len = trig_colors.size();
    for (int i=0;i<len;i++) {
        draw_triangle_if_valid(Triangle {trig_vertices[i*3  ], trig_vertices[i*3+1], trig_vertices[i*3+2]}, trig_colors[i]);
    }

    if (above) above->draw();
}

void BSPNode::draw_debug(int previous) {
    if (below) below->draw_debug(-1);

    ray::Color col = (previous > 0 ? ray::RED : (previous < 0 ? ray::BLUE : ray::LIME));

    int len = trig_colors.size();
    for (int i=0;i<len;i++) {
        auto rand_col = ray::ColorBrightness(col, ray::GetRandomValue(-25, 25) / 100.);
        draw_triangle_if_valid(Triangle {trig_vertices[i*3  ], trig_vertices[i*3+1], trig_vertices[i*3+2]}, rand_col);
    }

    if (above) above->draw_debug(1);
}

bool Renderer::cull_or_add_to_bsp_tree(Triangle trig, ray::Color col) {
    original++;
    auto aabb_min = ray::Vector3Min(trig.v0, ray::Vector3Min(trig.v1,trig.v2));
    auto aabb_max = ray::Vector3Max(trig.v0, ray::Vector3Max(trig.v1,trig.v2));

    // clip space culling
    if (aabb_max.z > 0 || aabb_max.x < -1 || aabb_min.x > 1 || aabb_max.y < -1 || aabb_min.y > 1) {
        culled++; return false;
    }

    Plane plane = trig.get_plane();

    // CW winding culling
    if (ray::Vector3DotProduct(plane.normal,ray::Vector3{0,0,1}) <= 0) {
        culled++; return false;
    }

    if (root) {
        root->add(trig, plane, col);
    } else {
        root = std::make_unique<BSPNode>(trig, plane, col);
    }
    return true;
}

void Renderer::draw(bool debug) {
    if (!root) return;

    if (debug) {
        ray::DrawFPS(550,20);
        debug_text(ray::TextFormat("renderer: original=%d culled=%d nodes=%d", original, culled, nodes));
        debug_text(ray::TextFormat("sliced=%d discarded=%d drawn=%d", sliced, discarded, drawn));
        original = 0, culled = 0, nodes = 0, sliced = 0, discarded = 0, drawn = 0;
        ray::SetRandomSeed(69); // prevent flickering
        root->draw_debug(0);
    } else
        root->draw();
}