#include "misc.h"

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
    const int pos_x=550, pos_y=20;
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