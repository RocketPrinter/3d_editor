#pragma once
#include <fstream>
#include <nlohmann/json.hpp>
#include "misc.h"
#include "object.h"
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
    std::vector<MenuItem*> submenuList{};
    static MenuItem* createFromObject(Object* obj);

};
struct MenuAction {
    ray::Rectangle rect;
    ray::Color color = ray::LIGHTGRAY;
    std::string text;
    std::function<void(World* world, MenuAction* menuAction)> onClick;
};

struct SubMenuAction {
    MenuItem* menuItem;
    ray::Rectangle rect;
    ray::Color color = ray::LIGHTGRAY;
    std::string text;
    std::function<void(World* world, MenuItem* mi, Object* obj)> onClick;
};


struct Menu {
    World* world ;
    int editing=0;
    bool submenuVisible = false;
    std::vector<MenuItem*> menuList{};
    std::vector<MenuAction*> menuActions{};
    std::vector<SubMenuAction*> subMenuActions{};
    int numElements = 0;
    Menu();
    void setMenuActions();
    int setMenuAlignments(std::vector<MenuItem*> mList, int indexX = 0, int indexY = 0);
    void addToMenu(Object* obj);
    void addToMenuChild(MenuItem* mi,Object* obj);
    void showMenu();
    void showMenuItem(std::vector<MenuItem*> mList);
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
    void showSubMenuActions(MenuItem &menuItem);
    void showSubMenuActionsForAction(MenuAction &menuAction);
    void initializeSubMenuActions();
    void selectMenuItem(MenuItem &menuItem);
};
