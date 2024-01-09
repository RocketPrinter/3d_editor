#pragma once
#include "misc.h"
#include "object.h"
#include <list>
struct MenuItem {
    int nrTotalChildren;
    ray::Rectangle rect;
    ray::Color color;
    std::string text;
    Object* object;
    std::list<MenuItem*> submenuList{};
    static MenuItem* createFromObject(Object* obj);

};
struct MenuAction {
    ray::Rectangle rect;
    ray::Color color;
    std::string text;
    void (*newFunction)(World* world);
    bool clicked = false;
};

struct SubMenuAction {
    MenuItem* menuItem;
    ray::Rectangle rect;
    ray::Color color;
    std::string text;
    void (*newFunction)(World* world, MenuItem* mi, Object* obj);
    bool clicked = false;
};


struct Menu {
    World* world ;
    bool submenuVisible = false;
    std::list<MenuItem*> menuList{};
    std::list<MenuAction*> menuActions{};
    std::list<SubMenuAction*> subMenuActions{};

    int numElements = 0;
    Menu();
    void setMenuActions();
    int setMenuAlignments(std::list<MenuItem*> mList, int indexX = 0, int indexY = 0);
    void addToMenu(Object* obj);
    void addToMenuChild(MenuItem* mi,Object* obj);
    void showMenu();
    void showMenuItem(std::list<MenuItem*> mList);
    bool isMouseOverMenuItem(MenuItem &menuItem);
    bool isMouseOverSubMenuAction(SubMenuAction &subMenuAction);
    bool isMouseLeftClickMenuItem(MenuItem &menuItem);
    bool isMouseRightClickMenuItem(MenuItem &menuItem);
    bool isMouseOverMenuAction(MenuAction &menuAction);
    bool isMouseLeftClickMenuAction(MenuAction &menuAction);
    bool isMouseLeftClickSubMenuAction(SubMenuAction &subMenuAction);
    void initializeMenuActions();
    void showHideSubMenuActions(MenuItem &menuItem);
    void initializeSubMenuActions();
    void clearMenu();
};
