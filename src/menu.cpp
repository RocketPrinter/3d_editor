#include <utility>
#include "menu.h"

MenuItem* MenuItem::createFromObject(Object* obj){
    MenuItem* menuItem = new MenuItem();
    ray::Rectangle rct = ray::Rectangle{20,10,120,18};
    ray::Rectangle rctVisibleBtn = ray::Rectangle{10,40,12,18};
    menuItem->rect = rct;
    menuItem->rectVisibleBtn = rctVisibleBtn;
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
    //if(this->numElements == 0) {
    //    this->selectMenuItem(*smi);
    //}
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
    MenuAction* actionAdd = new MenuAction {
        .rect = {10,50,130,18},
        .text = "+ Add",
        .onClick = [](World* world, MenuAction *menuAction) {
            world->menu->showSubMenuActionsForAction(*menuAction);
        },
    };

    MenuAction* actionSave = new MenuAction {
        .rect = {10,50,130,18},
        .text = "   Save",
        .onClick = [](World* world, MenuAction *menuAction) {
            world->menu->submenuVisible = false;
            std::ofstream jsonFile;
            jsonFile.open("save.json", std::ofstream::out | std::ofstream::trunc);
            if (jsonFile.is_open()) {
                json s = serialize(*world);
                jsonFile << s.dump(2);
                jsonFile.close();
            }
        },
    };

    MenuAction* actionSelectionMode = new MenuAction {
        .rect = {10,50,130,18},
        .text = "   Sel. mode: object",
        .onClick = [](World* world, MenuAction *menuAction) {
            world->selection.clear();
            world->selection_mode = (SelectionMode) (((int)world->selection_mode + 1) % 3);
            if (world->selection_mode != SelectionMode::Object) world->operation = Operation::Translate;
            switch (world->selection_mode) {
                case SelectionMode::Vertex:   menuAction->text = "   Sel mode: vertex";   break;
                case SelectionMode::Triangle: menuAction->text = "   Sel mode: triangle"; break;
                case SelectionMode::Object:   menuAction->text = "   Sel mode: object";   break;
            }
        },
    };

    MenuAction* actionOperation = new MenuAction {
            .rect = {10,50,130,18},
            .text = "   Op: translate",
            .onClick = [](World* world, MenuAction *menuAction) {
                if (world->selection_mode != SelectionMode::Object) return;
                world->operation = (Operation) (((int)world->operation + 1) % 3);
                switch (world->operation) {
                    case Operation::Translate: menuAction->text = "   Op: translate";   break;
                    case Operation::Rotate:    menuAction->text = "   Op: rotate"; break;
                    case Operation::Scale:     menuAction->text = "   Op: scale";   break;
                }
            },
    };

    MenuAction* actionShowVertices = new MenuAction {
            .rect = {10,50,130,18},
            .text = "   Show vert: false",
            .onClick = [](World* world, MenuAction *menuAction) {
                if (world->show_vertices)
                    world->show_vertices = false, menuAction->text = "   Show vert: false";
                else
                    world->show_vertices = true,  menuAction->text = "   Show vert: true";
            },
    };

    this->menuActions.push_back(actionAdd);
    this->menuActions.push_back(actionSave);
    this->menuActions.push_back(actionSelectionMode);
    this->menuActions.push_back(actionOperation);
    this->menuActions.push_back(actionShowVertices);
}

void Menu::initializeSubMenuActions(){
    const std::pair<std::string, std::function<Object*()> > new_functions[] = {
            {"triangle", Object::new_triangle},
            {"plane",    []{return Object::new_plane();}},
            {"cube"    , Object::new_cube},
            {"cylinder", []{return Object::new_cylinder();}},
            {"cone"    , []{return Object::new_cone();}},
            {"sphere"  , []{return Object::new_sphere(); }}
    };

    float y_offset = 50;
    for (auto f: new_functions) {
        this->subMenuActions.push_back(new SubMenuAction{
            .rect = {130,y_offset,120,18},
            .text = "+ Add new " + f.first,
            .onClick = [=](World* world, MenuItem* mi, Object* obj){
                Object *new_obj = f.second();
                if(mi == nullptr || obj == nullptr){
                    world->addNewObject(new_obj);
                } else {
                    obj->children.push_back(new_obj);
                    world->menu->addToMenuChild(mi, new_obj);
                }
            }
        });
        y_offset += 20;
    }
}

void Menu::showSubMenuActionsForAction(MenuAction &menuAction) {
    submenuVisible = true;
    int i = 0;
    for(SubMenuAction *si : this->subMenuActions){
        si->rect.x = menuAction.rect.x + menuAction.rect.width + 5;
        si->rect.y = (menuAction.rect.y) + ((i) * (18 + (i == 0?0:2)));
        si->menuItem = nullptr;
        i++;
    }
}

void Menu::showSubMenuActions(MenuItem &menuItem) {
    submenuVisible = true;
    int i = 0;
    for(SubMenuAction *si : this->subMenuActions){
        si->rect.x = menuItem.rect.x + menuItem.rect.width + 5;
        si->rect.y = (menuItem.rect.y) + ((i) * (18 + (i == 0?0:2)));
        si->menuItem = &menuItem;
        i++;
    }
}

int Menu::setMenuAlignments(std::vector<MenuItem*> mList, int indexX, int indexY){
    int idx;
    int nrRows = 0;
    int rowsY;
    for (MenuItem *i : mList){
        nrRows++;
        i->rectVisibleBtn.x = 10 + (indexX * 5);
        i->rect.x = i->rectVisibleBtn.x + i->rectVisibleBtn.width + 1;
        i->rect.y = static_cast<float>(10 + (indexY * (18 + (indexY == 0?0:2))))  ;
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
        i->rect.y = static_cast<float>(10 + ((this->numElements + idx) * (18 + (this->numElements == 0?0:2))))  ;
        idx++;
    }
}

bool Menu::isMouseOverMenuItemVisibleBtn(MenuItem &menuItem){
    return ray::CheckCollisionPointRec(ray::GetMousePosition(),menuItem.rectVisibleBtn);
}

bool Menu::isMouseLeftClickMenuItemVisibleBtn(MenuItem &menuItem){
    return isMouseOverMenuItemVisibleBtn(menuItem) && ray::IsMouseButtonPressed(ray::MOUSE_BUTTON_LEFT);
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

void Menu::selectMenuItem(MenuItem &menuItem) {
    world->selection.clear();
    world->selection[menuItem.object];
}

void Menu::showMenuItem(std::vector<MenuItem*> mList){
    for (MenuItem *i : mList){
        if (this->isMouseOverMenuItem(*i) || isMouseOverMenuItemVisibleBtn(*i)){
            i->color = ray::GREEN;
        } else {
            if (world->selection.contains(i->object)) {
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
            showSubMenuActions(*i);
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
        ray::Color color = i->color;
        if (this->isMouseOverMenuAction(*i))
            color = ray::GREEN;

        if (this->isMouseLeftClickMenuAction(*i))
            i->onClick(this->world, i);

        ray::DrawRectangleRec(i->rect, color);
        ray::DrawText( i->text.c_str(), i->rect.x+3, i->rect.y+3, 12, ray::DARKGRAY);
    }
    if(this->submenuVisible) {
        for (SubMenuAction *si: this->subMenuActions) {
            ray::Color color = si->color;
            if (this->isMouseOverSubMenuAction(*si)){
                color = ray::GREEN;
            }

            if (this->isMouseLeftClickSubMenuAction(*si)){
                if (si->menuItem == nullptr)
                    si->onClick(this->world, nullptr, nullptr);
                else
                    si->onClick(this->world, si->menuItem, si->menuItem->object);

                this->submenuVisible = false;

            }
            ray::DrawRectangleRec(si->rect, color);
            ray::DrawText(si->text.c_str(), si->rect.x + 3, si->rect.y + 3, 12, ray::DARKGRAY);
        }

    }
}