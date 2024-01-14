#include "menu.h"

void executeSaveAsJSon(World* world, MenuAction *menuAction) {
    if(world->menu->submenuVisible) {
        world->menu->submenuVisible = false;
    }
    std::ofstream jsonFile;
    jsonFile.open("file.out", std::ofstream::out | std::ofstream::trunc);
    if (jsonFile.is_open()) {
        json s = serialize(*world);
        std::string str{s.dump()};
        jsonFile << str;
        jsonFile.close();
    }
}
void addNewObject(World* world, MenuAction *menuAction) {
    world->menu->showHideSubMenuActionsForAction(*menuAction);
}

void addNewCubeSub(World* world, MenuItem* mi, Object* obj) {
    Object* newCube = Object::new_cube(/*world->menu->numElements+1*/);
    if(mi == nullptr){
        world->addNewObject(newCube);
    } else {
        obj->children.push_back(newCube);
        world->menu->addToMenuChild(mi, newCube);
    }
}
void addNewTriangleSub(World* world, MenuItem* mi, Object* obj) {
    Object* newTriangle = Object::new_triangle(/*world->menu->numElements+1*/);
    if(mi == nullptr){
        world->addNewObject(newTriangle);
    } else {
        obj->children.push_back(newTriangle);
        world->menu->addToMenuChild(mi, newTriangle);
    }
}

void addNewCylinderSub(World* world, MenuItem* mi, Object* obj) {
    Object* newCylinder = Object::new_cylinder(world->menu->numElements+1);
    if(mi == nullptr){
        world->addNewObject(newCylinder);
    } else {
        obj->children.push_back(newCylinder);
        world->menu->addToMenuChild(mi, newCylinder);
    }
}


MenuItem* MenuItem::createFromObject(Object* obj){
    int cnt = 0;
    MenuItem* menuItem = new MenuItem();
    ray::Rectangle rct = (ray::Rectangle){20,50,120,18};
    ray::Rectangle rctVisibleBtn = (ray::Rectangle){10,80,12,18};
    menuItem->rect = rct;
    menuItem->rectVisibleBtn = rctVisibleBtn;
    menuItem->color = ray::LIGHTGRAY;
    menuItem->text = obj->name;
    menuItem->visibleBtnText = (obj->is_visible?"v":" ");
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
    if(this->numElements == 0) {
        this->selectMenuItem(*smi);
    }
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
    MenuAction* actionAdd = new MenuAction();
    ray::Rectangle rctNCube = (ray::Rectangle){10,static_cast<float>(50 + (this->numElements * (18 + (this->numElements == 0?0:2)))),120,18};
    actionAdd->rect = rctNCube;
    actionAdd->color = ray::LIGHTGRAY;
    actionAdd->text = "+ Add";
    actionAdd->newFunction = addNewObject;
    MenuAction* actionSave = new MenuAction();
    ray::Rectangle rctSave = (ray::Rectangle){10,static_cast<float>(50 + ((this->numElements + 2) * (18 + (this->numElements == 0?0:2)))),120,18};
    actionSave->rect = rctSave;
    actionSave->color = ray::LIGHTGRAY;
    actionSave->newFunction = executeSaveAsJSon;
    actionSave->text = "    Save";

    this->menuActions.push_back(actionAdd);
    this->menuActions.push_back(actionSave);
}

void Menu::initializeSubMenuActions(){
    SubMenuAction* actionNewCube = new SubMenuAction();
    ray::Rectangle rctNCube = (ray::Rectangle){130,50,120,18};
    actionNewCube->rect = rctNCube;
    actionNewCube->color = ray::LIGHTGRAY;
    actionNewCube->text = "+ Add new cube";
    actionNewCube->newFunction = addNewCubeSub;
    SubMenuAction* actionNewTriangle = new SubMenuAction();
    ray::Rectangle rctNTriangle = (ray::Rectangle){130,70,120,18};
    actionNewTriangle->rect = rctNTriangle;
    actionNewTriangle->color = ray::LIGHTGRAY;
    actionNewTriangle->text = "+ Add new triangle";
    actionNewTriangle->newFunction = addNewTriangleSub;
    SubMenuAction* actionNewCylinder = new SubMenuAction();
    ray::Rectangle rctNCylinder = (ray::Rectangle){130,90,120,18};
    actionNewCylinder->rect = rctNCylinder;
    actionNewCylinder->color = ray::LIGHTGRAY;
    actionNewCylinder->newFunction = addNewCylinderSub;
    actionNewCylinder->text = "+ Add new cylinder";

    this->subMenuActions.push_back(actionNewCube);
    this->subMenuActions.push_back(actionNewTriangle);
    this->subMenuActions.push_back(actionNewCylinder);
}

void Menu::showHideSubMenuActionsForAction(MenuAction &menuAction){
    if(!this->submenuVisible) { this->submenuVisible = !this->submenuVisible; };
    int i = 0;
    for(SubMenuAction *si : this->subMenuActions){
        si->rect.x = menuAction.rect.x + menuAction.rect.width + 5;
        si->rect.y = (menuAction.rect.y) + ((i) * (18 + (i == 0?0:2)));
        si->menuItem = nullptr;
        i++;
    }
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
        i->rectVisibleBtn.x = 10 + (indexX * 5);
        i->rect.x = i->rectVisibleBtn.x + i->rectVisibleBtn.width + 1;
        i->rect.y = static_cast<float>(50 + (indexY * (18 + (indexY == 0?0:2))))  ;
        i->rectVisibleBtn.y = i->rect.y;
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
        i->rect.y = static_cast<float>(50 + ((this->numElements + idx) * (18 + (this->numElements == 0?0:2))))  ;
        idx++;
    }
}

bool Menu::isMouseOverMenuItemVisibleBtn(MenuItem &menuItem){
    return ray::CheckCollisionPointRec(ray::GetMousePosition(),menuItem.rectVisibleBtn);
};

bool Menu::isMouseLeftClickMenuItemVisibleBtn(MenuItem &menuItem){
    return isMouseOverMenuItemVisibleBtn(menuItem) && ray::IsMouseButtonPressed(ray::MOUSE_BUTTON_LEFT);
};

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

void Menu::clearMenuItemSelection(std::list<MenuItem*> mList){
    for (MenuItem *i : mList){
        i->isSelectedObject = false;
        i->color = ray::LIGHTGRAY;
        clearMenuItemSelection(i->submenuList);
    }
}

void Menu::selectMenuItem(MenuItem &menuItem){
    clearMenuItemSelection(this->menuList);
    menuItem.isSelectedObject = true;
    menuItem.color = ray::BLUE;
    this->selectedObject = menuItem.object;
}

void Menu::transformObject(Object *object) {

    if (ray::IsKeyPressed(ray::KEY_SPACE)) {
        editing = (editing + 1) % 3;
    }

    ray::Vector3 input{};
    if (ray::IsKeyDown(ray::KEY_J))input.x -= 0.02;
    if (ray::IsKeyDown(ray::KEY_L))input.x += 0.02;
    if (ray::IsKeyDown(ray::KEY_I))input.y -= 0.02;
    if (ray::IsKeyDown(ray::KEY_K))input.y += 0.02;
    if (ray::IsKeyDown(ray::KEY_U))input.z -= 0.02;
    if (ray::IsKeyDown(ray::KEY_O))input.z += 0.02;

    switch (editing) {
        case 0:
            object->position = ray::Vector3Add(object->position, input);
            break;
        case 1:
            object->rotation = ray::QuaternionMultiply(ray::QuaternionFromEuler(input.x,input.y,input.z), object->rotation);
            break;
        case 2:
            object->scale = ray::Vector3Add(object->scale, input);
            break;
    }

    debug_text(ray::TextFormat("%s position: %s", object->name.c_str(), v3_to_text(object->position)),
               editing == 0 ? ray::GREEN : ray::GRAY);
    debug_text(ray::TextFormat("%s rotation: %s", object->name.c_str(), v3_to_text(ray::QuaternionToEuler(object->rotation))),
               editing == 1 ? ray::GREEN : ray::GRAY);
    debug_text(ray::TextFormat("%s scale: %s", object->name.c_str(), v3_to_text(object->scale)),
               editing == 2 ? ray::GREEN : ray::GRAY);
}

void Menu::showMenuItem(std::list<MenuItem*> mList){
    for (MenuItem *i : mList){
        if (this->isMouseOverMenuItem(*i) || isMouseOverMenuItemVisibleBtn(*i)){
            i->color = ray::GREEN;
        } else {
            if (i->isSelectedObject) {
                i->color = ray::BLUE;
            } else {
                i->color = ray::LIGHTGRAY;
            }
        }
        if (this->isMouseLeftClickMenuItemVisibleBtn(*i)){
            i->object->is_visible = !i->object->is_visible;
            i->visibleBtnText =  (i->object->is_visible?"v":" ");
        }

        if (this->isMouseLeftClickMenuItem(*i)){
            selectMenuItem(*i);
            if(this->submenuVisible) {
                this->submenuVisible = false;
            }
        }

        if (this->isMouseRightClickMenuItem(*i)){
            showHideSubMenuActions(*i);
        }
        ray::DrawRectangleRec(i->rect, i->color);
        ray::DrawText( i->text.c_str(), i->rect.x+3, i->rect.y+3, 12, ray::DARKGRAY);
        ray::DrawRectangleRec(i->rectVisibleBtn, i->color);
        ray::DrawText( i->visibleBtnText.c_str(), i->rectVisibleBtn.x+3, i->rectVisibleBtn.y+3, 12, ray::DARKGRAY);
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
            i->newFunction(this->world, i);
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
                if (si->menuItem == nullptr){
                    si->newFunction(this->world, nullptr, nullptr);
                } else {
                    si->newFunction(this->world, si->menuItem, si->menuItem->object);
                }
                this->submenuVisible = false;

            }
            ray::DrawRectangleRec(si->rect, si->color);
            ray::DrawText(si->text.c_str(), si->rect.x + 3, si->rect.y + 3, 12, ray::DARKGRAY);
        }

    }
    transformObject(this->selectedObject);
}
