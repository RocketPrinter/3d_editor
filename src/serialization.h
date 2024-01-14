#pragma once
#include <nlohmann/json.hpp>
#include "object.h"
#include <iostream>
#include <fstream>
#include <string>

using json = nlohmann::json;

json serializeObj(Object* cube);
json serialize(World &world);
json ReadJsonFromFile(std::string file_name);
Object* deserializeObj(json &jObj);
bool deserialize(World &world);


