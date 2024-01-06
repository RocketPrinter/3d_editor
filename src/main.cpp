#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include "misc.h"
#include "object.h"
#define RAYGUI_IMPLEMENTATION
#include "lib/raygui.h"
#include "serialization.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#define ARRAY_SIZE(A) (sizeof(A)/sizeof(A[0]))

enum {
    MENU_NEW = 0,
    MENU_OPEN,
    MENU_SAVE,
    MENU_SAVEAS,
    MENU_EXIT,
};

enum {
    SUBMENU_NEW_PROJECT=0,
    SUBMENU_NEW_DATABASE,
    SUBMENU_NEW_WORKSPACE,
};

enum {
    SUBMENU_OPEN_FILE = 0,
    SUBMENU_OPEN_PROJECT,
};

enum {
    SUBMENU_SAVEAS_IMAGE = 0,
    SUBMENU_SAVEAS_DATABASE,
    SUBMENU_SAVEAS_CPP,
    SUBMENU_SAVEAS_JSON
};

enum {
    APP_STATE_NORMAL = 0,
    APP_STATE_SHOW_MENU,
    APP_STATE_SHOW_SUBMENU,
} ;

//new menu
void executeNewProject();
void executeNewDatabase();
void executeNewWorkspace();
//open menu
void executeOpenFile();
void executeOpenProject();
//saveas menu
void executeSaveAsImage();
void executeSaveAsDatabase();
void executeSaveAsCPP();
void executeSaveAsJSon();

char* mesajMenu = "";
#include "misc.h"

void test_config(World &world);
static World world{};

static json serializeObj(Object &cube){
    json obj;
    obj["name"] = cube.name;
    obj["position"]["x"] = cube.position.x;
    obj["position"]["y"] = cube.position.y;
    obj["position"]["z"] = cube.position.z;
    obj["scale"]["x"] = cube.scale.x;
    obj["scale"]["y"] = cube.scale.y;
    obj["scale"]["z"] = cube.scale.z;
    obj["rotation"]["x"] = cube.rotation.x;
    obj["rotation"]["y"] = cube.rotation.y;
    obj["rotation"]["z"] = cube.rotation.z;
    obj["rotation"]["w"] = cube.rotation.w;
    obj["vertices"] = json::array();
    for (ray::Vector3 &vert: cube.vertices) {
        json vertices;
        vertices["x"] = vert.x;
        vertices["y"] = vert.y;
        vertices["z"] = vert.z;
        obj["vertices"].push_back(vertices);
    }
    obj["triangle_indexes"] = json::array();
    for (int i: cube.triangle_indexes) {
        obj["triangle_indexes"]. push_back(i);
    }
    obj["triangle_colors"] = json::array();
    for (ray::Color &clr: cube.triangle_colors) {
        json colors;
        colors["r"] = clr.r;
        colors["g"] = clr.g;
        colors["b"] = clr.b;
        colors["a"] = clr.a;
        obj["triangle_colors"].push_back(colors);
    }
    obj["children"]=json::array();
    for (Object &cub :cube.children) {
        json temp = serializeObj(cub);
        obj["children"].push_back(temp);
    }
    return obj;
}
static json serialize(World &world){
    json j;
    j["objects"] = json::array();
    for (Object &cube: world.objects) {
        json obj;
        obj= serializeObj(cube);
        j["objects"].push_back(obj);
    }
    return j;
}

json ReadJsonFromFile(std::string file_name) {
    try {
        return json::parse(std::ifstream{file_name, std::ios::in});
    } catch (json::parse_error& e) {
        std::cerr << "JSON parse exception : " << e.what() << std::endl;
    } catch (std::ifstream::failure& e) {
        std::cerr << "Stream exception : " << e.what() << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Exception : " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unk error" << std::endl;
    }
    return {};
}
static Object deserializeObj(json &jObj){
    Object obj{};
    obj.name = jObj["name"] ;
    obj.position.x = jObj["position"]["x"];
    obj.position.y = jObj["position"]["y"];
    obj.position.z = jObj["position"]["z"];
    obj.scale.x = jObj["scale"]["x"];
    obj.scale.y = jObj["scale"]["y"];
    obj.scale.z = jObj["scale"]["z"];
    obj.rotation.x = jObj["rotation"]["x"];
    obj.rotation.y = jObj["rotation"]["y"];
    obj.rotation.z = jObj["rotation"]["z"];
    obj.rotation.w = jObj["rotation"]["w"];
    for(json& vrt : jObj["vertices"]) {
        obj.vertices.push_back({vrt["x"], vrt["y"], vrt["z"]});
    }
    for (json& child:jObj["children"]){
        obj.children.push_back(deserializeObj(child));
    }
    return obj;
}
bool deserialize(){
    json jInput = ReadJsonFromFile("file.out");
    if (jInput.empty()) {
        return false;
    }

    for(json& o : jInput["objects"]) {
        world.objects.push_back(deserializeObj(o)) ;
    }

    return true;
}
void HandleMenu(int *state, int *mainActive, int *mainFocused, int *subActive, int *scrollIndex, ray::Rectangle* menuRec){

    const char* menuItems[] = { "New", "Open", "Save", "Save As", "Exit" };
    const char* submenuNew[] = {"Project", "Database", "Workspace"};
    const char* submenuOpen[] = {"File", "Project"};
    const char* submenuSaveAs[] = {"Image", "Database", "C++ Source", "JSON"};

    ray::Vector2 mouse = ray::GetMousePosition();

    if(*state == APP_STATE_NORMAL)
    {
        if(IsMouseButtonPressed(ray::MOUSE_RIGHT_BUTTON)) {
            *state = APP_STATE_SHOW_MENU;
            menuRec->x = mouse.x;
            menuRec->y = mouse.y;
        }
    }
    // DRAW
    GuiGrid((ray::Rectangle){0, 0, screenWidth, screenHeight},"",20.0f, 2, &mouse);


    char** submenuText = NULL;
    void (*funcSubmenu[4])();
    int sz = 0;
    const int itemHeight = (GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT));
    if(*state == APP_STATE_SHOW_MENU || *state == APP_STATE_SHOW_SUBMENU )
    {
        menuRec->height = itemHeight * ARRAY_SIZE(menuItems) + 10;
        int focused = *mainFocused;
        GuiListViewEx(*menuRec, menuItems, ARRAY_SIZE(menuItems), scrollIndex, mainActive, &focused);
        if(focused != -1)  {
            *mainFocused = *mainActive = focused;
            if(IsMouseButtonPressed(ray::MOUSE_LEFT_BUTTON)) {
                TraceLog(ray::LOG_INFO, ray::TextFormat("CLICKED >>> %s", menuItems[focused]));
                *state = APP_STATE_NORMAL;
                *mainFocused = -1;
                *mainActive = -1;
                *subActive = -1;
            }
        }
        if(*mainFocused == MENU_NEW) {
            submenuText = (char**)submenuNew;
            funcSubmenu[0] = &executeNewProject;
            funcSubmenu[1] = &executeNewDatabase;
            funcSubmenu[2] = &executeNewWorkspace;
            *state = APP_STATE_SHOW_SUBMENU;
            sz = ARRAY_SIZE(submenuNew);
        } else if(*mainFocused == MENU_OPEN) {
            funcSubmenu[0] = &executeOpenFile;
            funcSubmenu[1] = &executeOpenProject;
            submenuText = (char**)submenuOpen;
            *state = APP_STATE_SHOW_SUBMENU;
            sz = ARRAY_SIZE(submenuOpen);
        } else if(*mainFocused == MENU_SAVEAS) {
            funcSubmenu[0] = &executeSaveAsImage;
            funcSubmenu[1] = &executeSaveAsDatabase;
            funcSubmenu[2] = &executeSaveAsCPP;
            funcSubmenu[3] = &executeSaveAsJSon;
            submenuText = (char**)submenuSaveAs;
            *state = APP_STATE_SHOW_SUBMENU;
            sz = ARRAY_SIZE(submenuSaveAs);
        }
    }

    if(*state == APP_STATE_SHOW_SUBMENU && submenuText != NULL)
    {
        ray::Rectangle bounds = {menuRec->x + menuRec->width + 2, menuRec->y + (*mainFocused)*itemHeight, 100, static_cast<float>(sz*itemHeight + 10)};
        int focused = -1;
        GuiListViewEx(bounds, (const char**)submenuText, sz, NULL, subActive, &focused);
        if(IsMouseButtonPressed(ray::MOUSE_LEFT_BUTTON)) {
            (*funcSubmenu[focused])();
            TraceLog(ray::LOG_INFO, ray::TextFormat("CLICKED >>> %s > %s", menuItems[*mainActive], submenuText[focused]));
            *state = APP_STATE_NORMAL;
            *mainFocused = *mainActive = *subActive = -1;
        }
    }

}

int main()
{

    int state = APP_STATE_NORMAL;
    int mainActive = -1;
    int mainFocused = -1;
    int subActive = -1;
    int scrollIndex = -1;
    ray::Rectangle menuRec = {0, 0, 100, 180};

    ray::InitWindow(screenWidth, screenHeight, "Editor 3D");
    ray::SetTargetFPS(60);

    if (not deserialize()) {
        world.objects.push_back(Object::new_cube());
    }

    // Main game loop
    while (!ray::WindowShouldClose())
    {
        ray::BeginDrawing();
        ClearBackground(ray::RAYWHITE);

        if (ray::IsKeyPressed(ray::KEY_SLASH)) world.debug_render = !world.debug_render;
        if (ray::IsMouseButtonDown(ray::MOUSE_BUTTON_LEFT) && !ray::IsKeyDown(ray::KEY_LEFT_SHIFT)) world.raycast_and_add_to_selection(ray::GetMouseX(), ray::GetMouseY());
        if (ray::IsMouseButtonDown(ray::MOUSE_BUTTON_RIGHT)) world.raycast_and_add_to_selection(ray::GetMouseX(), ray::GetMouseY());
        world.camera.input_movement();

        world.render();
        test_config(world);

        reset_draw_debug();
        ray::DrawFPS(20,20);

        HandleMenu(&state, &mainActive, &mainFocused, &subActive, &scrollIndex, &menuRec);

        ray::DrawText( mesajMenu, 10, 90, 20, ray::DARKGRAY);
        ray::EndDrawing();
    }

    ray::CloseWindow();

    return 0;
}

int editing=0, cube_index=0;
void test_config(World &world) {
    auto &cam = world.camera;
    Object &cube = world.objects[cube_index];

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

    for (int i=0; i <= world.objects.size() && i < 10; i++) {
        if (ray::IsKeyPressed(ray::KEY_ZERO + i)) {
            if (i == world.objects.size()) {
                world.objects.push_back(Object::new_cube());
            }
            cube_index = i;
        }
    }

    switch (editing) {
        case 0:
            cube.position = ray::Vector3Add(cube.position, input);
            break;
        case 1:
            cube.rotation = ray::QuaternionMultiply(ray::QuaternionFromEuler(input.x,input.y,input.z), cube.rotation);
            break;
        case 2:
            cube.scale = ray::Vector3Add(cube.scale, input);
            break;
    }

    debug_text(ray::TextFormat("cube #%d position: %s", cube_index, v3_to_text(cube.position)),
               editing == 0 ? ray::GREEN : ray::GRAY);
    debug_text(ray::TextFormat("cube #%d rotation: %s", cube_index, v3_to_text(ray::QuaternionToEuler(cube.rotation))),
               editing == 1 ? ray::GREEN : ray::GRAY);
    debug_text(ray::TextFormat("cube #%d scale: %s", cube_index, v3_to_text(cube.scale)),
               editing == 2 ? ray::GREEN : ray::GRAY);
}

void executeNewProject() {
    mesajMenu = "New Project";
};
void executeNewDatabase(){
    mesajMenu = "New Database";
};
void executeNewWorkspace(){
    mesajMenu = "NewWorkspace";
};

void executeOpenProject() {
    mesajMenu = "Open Project";
};
void executeOpenFile(){
    mesajMenu = "Open File";
};

void executeSaveAsImage(){
    mesajMenu = "SaveAs Image";
};
void executeSaveAsDatabase(){
    mesajMenu = "SaveAs Database";
};
void executeSaveAsCPP(){
    mesajMenu = "SaveAs CPP";
};
void executeSaveAsJSon(){
    std::ofstream jsonFile;
    jsonFile.open("file.out", std::ofstream::out | std::ofstream::trunc);
    if (jsonFile.is_open()) {
        json s = serialize(world);
        std::string str{s.dump()};
        jsonFile<<str;
        jsonFile.close();
    }
};
