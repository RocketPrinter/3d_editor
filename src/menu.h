#pragma once
#include "misc.h"
#include "object.h"
#include <list>
#include <nlohmann/json.hpp>
#include <fstream>
#include "serialization.h"
using json = nlohmann::json;
struct MenuItem {
    int nrTotalChildren;
    ray::Rectangle rect;
    ray::Rectangle rectVisibleBtn;
    ray::Color color;
    std::string text;
    std::string visibleBtnText;
    Object* object;
    bool isSelectedObject = false;
    std::list<MenuItem*> submenuList{};
    static MenuItem* createFromObject(Object* obj);

};
struct MenuAction {
    ray::Rectangle rect;
    ray::Color color;
    std::string text;
    void (*newFunction)(World* world, MenuAction* menuAction);
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
    int editing=0;
    bool submenuVisible = false;
    std::list<MenuItem*> menuList{};
    std::list<MenuAction*> menuActions{};
    std::list<SubMenuAction*> subMenuActions{};
    Object* selectedObject;
    int numElements = 0;
    Menu();
    void setMenuActions();
    int setMenuAlignments(std::list<MenuItem*> mList, int indexX = 0, int indexY = 0);
    void addToMenu(Object* obj);
    void addToMenuChild(MenuItem* mi,Object* obj);
    void showMenu();
    void showMenuItem(std::list<MenuItem*> mList);
    bool isMouseOverMenuItem(MenuItem &menuItem);
    bool isMouseOverMenuItemVisibleBtn(MenuItem &menuItem);
    bool isMouseLeftClickMenuItemVisibleBtn(MenuItem &menuItem);
    bool isMouseOverSubMenuAction(SubMenuAction &subMenuAction);
    bool isMouseLeftClickMenuItem(MenuItem &menuItem);
    bool isMouseRightClickMenuItem(MenuItem &menuItem);
    bool isMouseOverMenuAction(MenuAction &menuAction);
    bool isMouseLeftClickMenuAction(MenuAction &menuAction);
    bool isMouseLeftClickSubMenuAction(SubMenuAction &subMenuAction);
    void initializeMenuActions();
    void showHideSubMenuActions(MenuItem &menuItem);
    void showHideSubMenuActionsForAction(MenuAction &menuAction);
    void initializeSubMenuActions();
    void selectMenuItem(MenuItem &menuItem);
    void clearMenuItemSelection(std::list<MenuItem*> mList);
    void transformObject(Object *object);
    void clearMenu();
};
