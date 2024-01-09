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

void addNewCubeSub(World* world, MenuItem* mi, Object* obj) {
    Object* newCube = Object::new_cube(world->menu->numElements+1);
    obj->children.push_back(newCube);
    world->menu->addToMenuChild(mi,newCube);
}
void addNewTriangleSub(World* world, MenuItem* mi, Object* obj) {
    Object* newTriangle = Object::new_triangle(world->menu->numElements+1);
    obj->children.push_back(newTriangle);
    world->menu->addToMenuChild(mi,newTriangle);
}

void addNewCylinderSub(World* world, MenuItem* mi, Object* obj) {
    Object* newCylinder = Object::new_cylinder(world->menu->numElements+1);
    obj->children.push_back(newCylinder);
    world->menu->addToMenuChild(mi,newCylinder);
}


MenuItem* MenuItem::createFromObject(Object* obj){
    int cnt = 0;
    MenuItem* menuItem = new MenuItem();
    ray::Rectangle rct = (ray::Rectangle){10,80,120,18};
    menuItem->rect = rct;
    menuItem->color = ray::LIGHTGRAY;
    menuItem->text = (obj->is_visible?"v ":"  ")+obj->name;
    menuItem->object = obj;
    menuItem->nrTotalChildren = 0;
    for(Object *objChild : obj->children){
        MenuItem* smi = createFromObject(objChild);
        menuItem->nrTotalChildren += (smi->nrTotalChildren == 0)?1:smi->nrTotalChildren+1;
        menuItem->submenuList.push_back(createFromObject(objChild));
    }
    return menuItem;
}

Menu::Menu(){
    this->initializeMenuActions();
    this->initializeSubMenuActions();
}
void Menu::addToMenu(Object* obj){
    MenuItem* smi = MenuItem::createFromObject(obj);
    this->menuList.push_back(smi);
    this->numElements += 1 + smi->nrTotalChildren;
    setMenuAlignments(this->menuList);
    setMenuActions();
}
void Menu::addToMenuChild(MenuItem* mi,Object* obj){
    mi->submenuList.push_back(MenuItem::createFromObject(obj));
    this->numElements += 1;
    setMenuAlignments(this->menuList);
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

void Menu::initializeSubMenuActions(){
    SubMenuAction* actionNewCube = new SubMenuAction();
    ray::Rectangle rctNCube = (ray::Rectangle){130,80,120,18};
    actionNewCube->rect = rctNCube;
    actionNewCube->color = ray::LIGHTGRAY;
    actionNewCube->text = "+ Add new cube";
    actionNewCube->newFunction = addNewCubeSub;
    SubMenuAction* actionNewTriangle = new SubMenuAction();
    ray::Rectangle rctNTriangle = (ray::Rectangle){130,100,120,18};
    actionNewTriangle->rect = rctNTriangle;
    actionNewTriangle->color = ray::LIGHTGRAY;
    actionNewTriangle->text = "+ Add new triangle";
    actionNewTriangle->newFunction = addNewTriangleSub;
    SubMenuAction* actionNewCylinder = new SubMenuAction();
    ray::Rectangle rctNCylinder = (ray::Rectangle){130,120,120,18};
    actionNewCylinder->rect = rctNCylinder;
    actionNewCylinder->color = ray::LIGHTGRAY;
    actionNewCylinder->newFunction = addNewCylinderSub;
    actionNewCylinder->text = "+ Add new cylinder";

    this->subMenuActions.push_back(actionNewCube);
    this->subMenuActions.push_back(actionNewTriangle);
    this->subMenuActions.push_back(actionNewCylinder);
}

void Menu::showHideSubMenuActions(MenuItem &menuItem){
    if(!this->submenuVisible) { this->submenuVisible = !this->submenuVisible; };
    int i = 0;
    for(SubMenuAction *si : this->subMenuActions){
        si->rect.x = menuItem.rect.x + menuItem.rect.width + 5;
        si->rect.y = (menuItem.rect.y) + ((i) * (18 + (i == 0?0:2)));
        si->menuItem = &menuItem;
        i++;
    }
}

int Menu::setMenuAlignments(std::list<MenuItem*> mList, int indexX, int indexY){
    int idx;
    int nrRows = 0;
    int rowsY;
    for (MenuItem *i : mList){
        nrRows++;
        i->rect.x = 10 + (indexX * 5);
        i->rect.y = static_cast<float>(80 + (indexY * (18 + (indexY == 0?0:2))))  ;
        indexY++;
        idx=indexX;
        idx ++;
        rowsY = setMenuAlignments(i->submenuList, idx, indexY);
        nrRows += rowsY;
        indexY += rowsY;
    }
    return nrRows;
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

bool Menu::isMouseOverSubMenuAction(SubMenuAction &subMenuAction){
    return ray::CheckCollisionPointRec(ray::GetMousePosition(),subMenuAction.rect);
}

bool Menu::isMouseOverMenuAction(MenuAction &menuAction){
    return ray::CheckCollisionPointRec(ray::GetMousePosition(),menuAction.rect);
}

bool Menu::isMouseLeftClickMenuItem(MenuItem &menuItem){
    return isMouseOverMenuItem(menuItem) && ray::IsMouseButtonPressed(ray::MOUSE_BUTTON_LEFT);
}
bool Menu::isMouseRightClickMenuItem(MenuItem &menuItem){
    return isMouseOverMenuItem(menuItem) && ray::IsMouseButtonPressed(ray::MOUSE_BUTTON_RIGHT);
}

bool Menu::isMouseLeftClickMenuAction(MenuAction &menuAction){
    return isMouseOverMenuAction(menuAction) && ray::IsMouseButtonPressed(ray::MOUSE_BUTTON_LEFT);
}

bool Menu::isMouseLeftClickSubMenuAction(SubMenuAction &subMenuAction){
    return isMouseOverSubMenuAction(subMenuAction) && ray::IsMouseButtonPressed(ray::MOUSE_BUTTON_LEFT);
}

void Menu::showMenuItem(std::list<MenuItem*> mList){
    for (MenuItem *i : mList){
        if (this->isMouseOverMenuItem(*i)){
            i->color = ray::GREEN;
        } else {
            i->color = ray::LIGHTGRAY;
        }
        if (this->isMouseLeftClickMenuItem(*i)){
            i->object->is_visible = !i->object->is_visible;
            i->text =  (i->object->is_visible?"v ":"  ") + i->object->name;
        }
        if (this->isMouseRightClickMenuItem(*i)){
            showHideSubMenuActions(*i);
        }
        ray::DrawRectangleRec(i->rect, i->color);
        ray::DrawText( i->text.c_str(), i->rect.x+3, i->rect.y+3, 12, ray::DARKGRAY);
        showMenuItem(i->submenuList);
    }
}
void Menu::showMenu() {
    showMenuItem(this->menuList);
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
    if(this->submenuVisible) {
        for (SubMenuAction *si: this->subMenuActions) {
            if (this->isMouseOverSubMenuAction(*si)){
                si->color = ray::GREEN;
            } else {
                si->color = ray::LIGHTGRAY;
            }

            if (this->isMouseLeftClickSubMenuAction(*si)){
                si->newFunction(this->world, si->menuItem, si->menuItem->object);
                this->submenuVisible = false;
            }
            ray::DrawRectangleRec(si->rect, si->color);
            ray::DrawText(si->text.c_str(), si->rect.x + 3, si->rect.y + 3, 12, ray::DARKGRAY);
        }
    }
}
