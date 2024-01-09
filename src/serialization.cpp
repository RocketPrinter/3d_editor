#include "serialization.h"

json serializeObj(Object* cube){
    json obj;
    obj["name"] = cube->name;
    obj["position"]["x"] = cube->position.x;
    obj["position"]["y"] = cube->position.y;
    obj["position"]["z"] = cube->position.z;
    obj["scale"]["x"] = cube->scale.x;
    obj["scale"]["y"] = cube->scale.y;
    obj["scale"]["z"] = cube->scale.z;
    obj["rotation"]["x"] = cube->rotation.x;
    obj["rotation"]["y"] = cube->rotation.y;
    obj["rotation"]["z"] = cube->rotation.z;
    obj["rotation"]["w"] = cube->rotation.w;
    obj["vertices"] = json::array();
    for (ray::Vector3 &vert: cube->vertices) {
        json vertices;
        vertices["x"] = vert.x;
        vertices["y"] = vert.y;
        vertices["z"] = vert.z;
        obj["vertices"].push_back(vertices);
    }
    obj["triangle_indexes"] = json::array();
    for (int i: cube->triangle_indexes) {
        obj["triangle_indexes"]. push_back(i);
    }
    obj["triangle_colors"] = json::array();
    for (ray::Color &clr: cube->triangle_colors) {
        json colors;
        colors["r"] = clr.r;
        colors["g"] = clr.g;
        colors["b"] = clr.b;
        colors["a"] = clr.a;
        obj["triangle_colors"].push_back(colors);
    }
    obj["children"]=json::array();
    for (Object cub :cube->children) {
        json temp = serializeObj(&cub);
        obj["children"].push_back(temp);
    }
    return obj;
}
json serialize(World &world){
    json j;
    j["objects"] = json::array();
    for (Object* cube: world.objects) {
        json obj;
        obj = serializeObj(cube);
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
Object* deserializeObj(json &jObj){
    Object* obj = new Object();
    obj->name = jObj["name"] ;
    obj->position.x = jObj["position"]["x"];
    obj->position.y = jObj["position"]["y"];
    obj->position.z = jObj["position"]["z"];
    obj->scale.x = jObj["scale"]["x"];
    obj->scale.y = jObj["scale"]["y"];
    obj->scale.z = jObj["scale"]["z"];
    obj->rotation.x = jObj["rotation"]["x"];
    obj->rotation.y = jObj["rotation"]["y"];
    obj->rotation.z = jObj["rotation"]["z"];
    obj->rotation.w = jObj["rotation"]["w"];
    for(json& vrt : jObj["vertices"]) {
        obj->vertices.push_back({vrt["x"], vrt["y"], vrt["z"]});
    }
    for (int ti : jObj["triangle_indexes"]) {
        obj->triangle_indexes.push_back(ti);
    }
    for (json& clrs : jObj["triangle_colors"]) {
        ray::Color color{};
        color.a = clrs["a"];
        color.r = clrs["r"];
        color.b = clrs["b"];
        color.g = clrs["g"];
        obj->triangle_colors.push_back(color);
    }
//    for (json& child:jObj["children"]){
//        obj->children.push_back(&(deserializeObj(child)));
//    }
    return obj;
}

bool deserialize(World &world){
    json jInput = ReadJsonFromFile("file.out");
    if (jInput.empty()) {
        return false;
    }

    for(json& o : jInput["objects"]) {
        world.objects.push_back(deserializeObj(o)) ;
    }

    return true;
}
