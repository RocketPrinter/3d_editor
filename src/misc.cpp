#include "misc.h"

const char *v3_to_text(ray::Vector3 v) {
    return ray::TextFormat("(%f,%f,%f)", v.x, v.y, v.z);
}

const char *v4_to_text(ray::Vector4 v) {
    return ray::TextFormat("(%f,%f,%f,%f)", v.x, v.y, v.z, v.w);
}

const char *trig_to_text(ray::Vector3 *trig) {
    return ray::TextFormat("{(%f,%f,%f),(%f,%f,%f),(%f,%f,%f)}", trig[0].x, trig[0].y, trig[0].z, trig[1].x, trig[1].y, trig[1].z, trig[2].x, trig[2].y, trig[2].z);
}

int __draw_debug_offset=0;
void draw_debug(const char *text, ray::Color color) {
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