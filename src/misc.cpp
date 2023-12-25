#include "misc.h"

const char *v3_to_text(ray::Vector3 v3) {
    return ray::TextFormat("(%f,%f,%f)", v3.x, v3.y, v3.z);
}

const char *trig_to_text(ray::Vector3 *trig) {
    return ray::TextFormat("{(%f,%f,%f),(%f,%f,%f),(%f,%f,%f)}", trig[0].x, trig[0].y, trig[0].z, trig[1].x, trig[1].y, trig[1].z, trig[2].x, trig[2].y, trig[2].z);
}