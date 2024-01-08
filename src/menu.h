#pragma once
#include "misc.h"
#include "object.h"
#include <list>
struct MenuItem {
    ray::Rectangle rect;
    ray::Color color;
    std::string text;
    Object* object;
    bool clicked = false;
    std::list<MenuItem*> submenuList{};
};

struct Menu {
    std::list<MenuItem*> menuList{};
    int numElements = 0;
    void addToMenu(Object* obj);
    void showMenu();
    bool isMouseOverMenuItem(MenuItem &menuItem);
    bool isMouseLeftClickMenuItem(MenuItem &menuItem);
    void clearMenu();
};
