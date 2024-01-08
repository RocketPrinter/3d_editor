#include "menu.h"

void Menu::addToMenu(Object &obj){
    MenuItem* menuItem = new MenuItem();
    ray::Rectangle rct = (ray::Rectangle){10,static_cast<float>(80 + (this->numElements * (18 + (this->numElements == 0?0:2)))),100,18};
    menuItem->rect = rct;
    menuItem->color = ray::LIGHTGRAY;
    menuItem->text = "v "+obj.name;
    menuItem->object = &obj;
    this->menuList.push_back(menuItem);
    this->numElements += 1;
}
void Menu::clearMenu() {
    this->menuList.clear();
}
bool Menu::isMouseOverMenuItem(MenuItem &menuItem){
    return ray::CheckCollisionPointRec(ray::GetMousePosition(),menuItem.rect);
}
bool Menu::isMouseLeftClickMenuItem(MenuItem &menuItem){
    return isMouseOverMenuItem(menuItem) && ray::IsMouseButtonPressed(ray::MOUSE_BUTTON_LEFT);
}
void Menu::showMenu() {
    for (auto i : this->menuList){
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
}
