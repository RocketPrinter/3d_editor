#include "serialization.h"

json serializeObj(Object* obj){
    json objJSON;
    objJSON["name"] = obj->name;
    objJSON["is_visible"] = obj->is_visible;
    objJSON["position"]["x"] = obj->position.x;
    objJSON["position"]["y"] = obj->position.y;
    objJSON["position"]["z"] = obj->position.z;
    objJSON["scale"]["x"] = obj->scale.x;
    objJSON["scale"]["y"] = obj->scale.y;
    objJSON["scale"]["z"] = obj->scale.z;
    objJSON["rotation"]["x"] = obj->rotation.x;
    objJSON["rotation"]["y"] = obj->rotation.y;
    objJSON["rotation"]["z"] = obj->rotation.z;
    objJSON["rotation"]["w"] = obj->rotation.w;
    objJSON["vertices"] = json::array();
    for (ray::Vector3 &vert: obj->vertices) {
        json vertices;
        vertices["x"] = vert.x;
        vertices["y"] = vert.y;
        vertices["z"] = vert.z;
        objJSON["vertices"].push_back(vertices);
    }
    objJSON["triangle_indexes"] = json::array();
    for (int i: obj->triangle_indexes) {
        objJSON["triangle_indexes"]. push_back(i);
    }
    objJSON["triangle_colors"] = json::array();
    for (ray::Color &clr: obj->triangle_colors) {
        json colors;
        colors["r"] = clr.r;
        colors["g"] = clr.g;
        colors["b"] = clr.b;
        colors["a"] = clr.a;
        objJSON["triangle_colors"].push_back(colors);
    }
    objJSON["children"]=json::array();
    for (Object cub :obj->children) {
        json temp = serializeObj(&cub);
        objJSON["children"].push_back(temp);
    }
    return objJSON;
}
json serialize(World &world){
    json j;
    j["objects"] = json::array();
    for (Object* obj: world.objects) {
        json objJSON;
        objJSON = serializeObj(obj);
        j["objects"].push_back(objJSON);
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
    obj->is_visible = jObj["is_visible"];
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
