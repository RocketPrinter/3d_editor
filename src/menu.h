#pragma once
#include "misc.h"
#include "object.h"
#include <list>
struct MenuItem {
    ray::Rectangle rect;
    ray::Color color;
    std::string text;
    Object* object;
    std::list<MenuItem*> submenuList{};
};
struct MenuAction {
    ray::Rectangle rect;
    ray::Color color;
    std::string text;
    void (*newFunction)(World* world);
    bool clicked = false;
};


struct Menu {
    World* world ;
    std::list<MenuItem*> menuList{};
    std::list<MenuAction*> menuActions{};
    int numElements = 0;
    Menu();
    void setMenuActions();
    void addToMenu(Object* obj);
    void showMenu();
    bool isMouseOverMenuItem(MenuItem &menuItem);
    bool isMouseLeftClickMenuItem(MenuItem &menuItem);
    bool isMouseOverMenuAction(MenuAction &menuAction);
    bool isMouseLeftClickMenuAction(MenuAction &menuAction);
    void initializeMenuActions();
    void clearMenu();
};
