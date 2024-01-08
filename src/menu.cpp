#include "menu.h"

void addNewCube(World* world) {
    Object* newCube = Object::new_cube(world->menu->numElements+1);
    world->addNewObject(newCube);
}
void addNewTriangle(World* world) {
    Object* newTriangle = Object::new_triangle(world->menu->numElements+1);
    world->addNewObject(newTriangle);
}

void addNewCylinder(World* world) {
    Object* newCylinder = Object::new_cylinder(world->menu->numElements+1);
    world->addNewObject(newCylinder);
}

Menu::Menu(){
    this->initializeMenuActions();
}
void Menu::addToMenu(Object* obj){
    MenuItem* menuItem = new MenuItem();
    ray::Rectangle rct = (ray::Rectangle){10,static_cast<float>(80 + (this->numElements * (18 + (this->numElements == 0?0:2)))),120,18};
    menuItem->rect = rct;
    menuItem->color = ray::LIGHTGRAY;
    menuItem->text = "v "+obj->name;
    menuItem->object = obj;
    this->menuList.push_back(menuItem);
    this->numElements += 1;
    setMenuActions();
}

void Menu::initializeMenuActions(){
    MenuAction* actionNewCube = new MenuAction();
    ray::Rectangle rctNCube = (ray::Rectangle){10,static_cast<float>(80 + (this->numElements * (18 + (this->numElements == 0?0:2)))),120,18};
    actionNewCube->rect = rctNCube;
    actionNewCube->color = ray::LIGHTGRAY;
    actionNewCube->text = "+ Add new cube";
    actionNewCube->newFunction = addNewCube;
    MenuAction* actionNewTriangle = new MenuAction();
    ray::Rectangle rctNTriangle = (ray::Rectangle){10,static_cast<float>(80 + ((this->numElements + 1) * (18 + (this->numElements == 0?0:2)))),120,18};
    actionNewTriangle->rect = rctNTriangle;
    actionNewTriangle->color = ray::LIGHTGRAY;
    actionNewTriangle->text = "+ Add new triangle";
    actionNewTriangle->newFunction = addNewTriangle;
    MenuAction* actionNewCylinder = new MenuAction();
    ray::Rectangle rctNCylinder = (ray::Rectangle){10,static_cast<float>(80 + ((this->numElements + 2) * (18 + (this->numElements == 0?0:2)))),120,18};
    actionNewCylinder->rect = rctNCylinder;
    actionNewCylinder->color = ray::LIGHTGRAY;
    actionNewCylinder->newFunction = addNewCylinder;
    actionNewCylinder->text = "+ Add new cylinder";

    this->menuActions.push_back(actionNewCube);
    this->menuActions.push_back(actionNewTriangle);
    this->menuActions.push_back(actionNewCylinder);
}

void Menu::setMenuActions(){
    int idx = 0;
    for (MenuAction *i : this->menuActions){
        i->rect.y = static_cast<float>(80 + ((this->numElements + idx) * (18 + (this->numElements == 0?0:2))))  ;
        idx++;
    }
}

void Menu::clearMenu() {
    this->menuList.clear();
}
bool Menu::isMouseOverMenuItem(MenuItem &menuItem){
    return ray::CheckCollisionPointRec(ray::GetMousePosition(),menuItem.rect);
}

bool Menu::isMouseOverMenuAction(MenuAction &menuAction){
    return ray::CheckCollisionPointRec(ray::GetMousePosition(),menuAction.rect);
}

bool Menu::isMouseLeftClickMenuItem(MenuItem &menuItem){
    return isMouseOverMenuItem(menuItem) && ray::IsMouseButtonPressed(ray::MOUSE_BUTTON_LEFT);
}

bool Menu::isMouseLeftClickMenuAction(MenuAction &menuAction){
    return isMouseOverMenuAction(menuAction) && ray::IsMouseButtonPressed(ray::MOUSE_BUTTON_LEFT);
}
void Menu::showMenu() {
    for (MenuItem *i : this->menuList){
        if (this->isMouseOverMenuItem(*i)){
            i->color = ray::GREEN;
        } else {
            i->color = ray::LIGHTGRAY;
        }
        if (this->isMouseLeftClickMenuItem(*i)){
            i->clicked = !i->clicked;
            i->object->is_visible = !i->clicked;
            i->text =  (i->object->is_visible?"v ":"  ") + i->object->name;
        }
        ray::DrawRectangleRec(i->rect, i->color);
        ray::DrawText( i->text.c_str(), i->rect.x+3, i->rect.y+3, 12, ray::DARKGRAY);
    }
    for (MenuAction *i : this->menuActions){
        if (this->isMouseOverMenuAction(*i)){
            i->color = ray::GREEN;
        } else {
            i->color = ray::LIGHTGRAY;
        }
        if (this->isMouseLeftClickMenuAction(*i)){
            i->newFunction(this->world);
        }
        ray::DrawRectangleRec(i->rect, i->color);
        ray::DrawText( i->text.c_str(), i->rect.x+3, i->rect.y+3, 12, ray::DARKGRAY);
    }
}
