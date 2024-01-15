#pragma once
#include <functional>
#include "object.h"
#include "misc.h"

void show_tools(World& world);

struct Tool {
    std::string name = "";
    //std::function<void(World& world, float offset, float &width)> draw=nullptr;
    std::function<void(World& world)> click=nullptr;

    // returns in the width parameter the width of the resulting element
    void show(World &world, float x_offset, float &width);
};