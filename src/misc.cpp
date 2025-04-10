#include <iostream>
#include "misc.h"

#pragma region DEBUG

bool Vector3Finite(ray::Vector3 v) {
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}

const char *v2_to_text(ray::Vector2 v) {
    return ray::TextFormat("(%f,%f)", v.x, v.y);
}

const char *v3_to_text(ray::Vector3 v) {
    return ray::TextFormat("(%f,%f,%f)", v.x, v.y, v.z);
}

const char *v4_to_text(ray::Vector4 v) {
    return ray::TextFormat("(%f,%f,%f,%f)", v.x, v.y, v.z, v.w);
}

const char *color_to_text(ray::Color col) {
    return ray::TextFormat("(%d,%d,%d,%d)", col.r, col.g, col.b, col.a);
}

const char *trig_to_text(ray::Vector3 v0, ray::Vector3 v1, ray::Vector3 v2) {
    return ray::TextFormat("{(%f,%f,%f),(%f,%f,%f),(%f,%f,%f)}", v0.x, v0.y, v0.z, v1.x, v1.y, v1.z, v2.x, v2.y, v2.z);
}

int __draw_debug_offset=0;
void debug_text(const char *text, ray::Color color) {
    const int pos_x=700, pos_y=40;
    int overflow_width = ray::MeasureText(text, 10) - (screenWidth - pos_x);
    if (overflow_width < 0)
        ray::DrawText(text, pos_x, pos_y + __draw_debug_offset * 10, 10, color);
    else
        ray::DrawText(text, pos_x - overflow_width, pos_y + __draw_debug_offset * 10, 10, color);

    __draw_debug_offset++;
}

void reset_draw_debug() {
    __draw_debug_offset = 0;
}

#pragma endregion

#pragma region geometry_constructs

Ray Ray::from_points(ray::Vector3 a, ray::Vector3 b) {
    return Ray {.origin = a, .direction = ray::Vector3Normalize(ray::Vector3Subtract(b,a))};
}

ray::Vector3 Ray::point_at_distance(float t) {
    return ray::Vector3Add(origin, ray::Vector3Scale(direction,t));
}

float Plane::distance(ray::Vector3 point) {
    return -(ray::Vector3DotProduct(point,normal) + offset);
}

// returns the distance to the point of intersection relative to the origin, so the point would be origin + direction * t
float Plane::ray_intersection(Ray r) {
    return -(offset + ray::Vector3DotProduct(r.origin, normal)) / (ray::Vector3DotProduct(r.direction, normal));
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

std::optional<float> Sphere::ray_intersection(Ray r) {
    ray::Vector3 offset = ray::Vector3Subtract(r.origin, center);
    // getting coeficients
    float a = ray::Vector3DotProduct(r.direction,r.direction);
    float b = 2 * ray::Vector3DotProduct(r.direction, offset);
    float c = ray::Vector3DotProduct(offset, offset) - radius * radius;

    // solving quadratic
    float delta = b * b - 4 * a * c;
    if (delta < 0) return {};
    if (delta == 0) return {-b/2*a};
    float sqrt_delta = std::sqrt(delta);
    return {fmax(0, (-b-sqrt_delta/2*a))};
}


bool Triangle::is_point_on_triangle(ray::Vector3 p) {
    // v0 is A, v1 is B, v2 is C
    auto AB = ray::Vector3Subtract(v1,v0);
    auto AC = ray::Vector3Subtract(v2,v0);
    auto AP = ray::Vector3Subtract(p, v0);
    // versors of the plane determined by the triangle
    auto i = ray::Vector3Normalize(AB);
    auto j = ray::Vector3Normalize(AC);
    // project coordinates
    ray::Vector2 Pp = {ray::Vector3DotProduct(i,AP),ray::Vector3DotProduct(j,AP)};
    ray::Vector2 Bp = {ray::Vector3DotProduct(i,AB),ray::Vector3DotProduct(j,AB)};
    ray::Vector2 Cp = {ray::Vector3DotProduct(i,AC),ray::Vector3DotProduct(j,AC)};
    // AP = i*l1+j*l2
    float l1 = (Pp.y*Cp.x - Pp.x*Cp.y)/(Bp.y*Cp.x - Bp.x*Cp.y);
    float l2 = (Pp.y - l1 * Bp.y)/Cp.y;
    // p is inside the triangle if
    return l1 >= 0 & l2 >= 0 && l1 + l2 <= 1;
}

Triangle Triangle::ccw() {
    if((v1.x - v0.x) * (v2.y - v0.y) - (v2.x - v0.x) * (v1.y - v0.y) < 0)
        return *this;
    return Triangle{.v0=v0, .v1=v2, .v2=v1};
}

ray::Vector3 apply_transformation(ray::Vector3 v, ray::Matrix matrix) {
    // homogeneous coordinates
    ray::Vector4 v4 = ray::Vector4Transform(ray::Vector4FromVector3(v, 1.), matrix);
    // perspective divide/normalization
    return {v4.x/v4.w, v4.y/v4.w, v4.z/v4.w};
}

ray::Vector2 clip_to_screen_space(ray::Vector3 v3) {
    return ray::Vector2 {
            (v3.x + 1) * screenWidth / (float) 2,
            (v3.y + 1) * screenHeight / (float) 2,
    };
}

ray::Color lerp_color(ray::Color a, ray::Color b, float t) {
    return {
        .r= (unsigned char) ((1-t) * (float)a.r + t * (float)b.r),
        .g= (unsigned char) ((1-t) * (float)a.g + t * (float)b.g),
        .b= (unsigned char) ((1-t) * (float)a.b + t * (float)b.b),
        .a= (unsigned char) ((1-t) * (float)a.a + t * (float)b.a),
    };
}
#pragma endregion