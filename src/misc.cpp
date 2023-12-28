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
