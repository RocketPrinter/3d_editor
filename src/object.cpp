#include "object.h"
#include "menu.h"

ray::Matrix Object::get_model_matrix() {
    ray::Matrix model = ray::MatrixScale(scale.x, scale.y, scale.z);
    model = ray::MatrixMultiply(model, ray::QuaternionToMatrix(rotation));
    model = ray::MatrixMultiply(model, ray::MatrixTranslate(position.x, position.y, position.z));
    return model;
}

std::optional<RaycastResult> Object::raycast(Ray r, SelectionMode mode, ray::Matrix &parent_transform) {
    // multiplying parent transform by the model transform of this object
    ray::Matrix world_space_matrix = ray::MatrixMultiply(get_model_matrix(), parent_transform);

    // transforming vertices
    std::vector<ray::Vector3> transformed_vertices{};
    for (auto &v: vertices) {
        transformed_vertices.push_back(apply_transformation(v, world_space_matrix));
    }

    std::optional<RaycastResult> result{};

    if (mode == SelectionMode::Vertex) {
        // testing a sphere around vertices
        // todo:
    } else {
        // testing triangles
        int len = triangle_colors.size();
        for (int i=0; i < len; i++) {
            Triangle trig = {
                    .v0 = transformed_vertices[triangle_indexes[i*3+0]],
                    .v1 = transformed_vertices[triangle_indexes[i*3+1]],
                    .v2 = transformed_vertices[triangle_indexes[i*3+2]],
            };
            Plane p = trig.get_plane();
            float t = p.ray_intersection(r);
            auto hit_pos = r.point_at_distance(t);
            if (!std::isfinite(t) || t < 0 || !trig.is_point_on_triangle(hit_pos)) continue;
            RaycastResult new_result = {.distance = t,.obj = this,.hit_pos = hit_pos,.index = i,};
            if (result.has_value() && (new_result.distance < result->distance))
                result.emplace(new_result);
        }
    }

    // raycasting children
    for(Object &child : children) {
        auto child_result = child.raycast(r, mode, world_space_matrix);
        if (child_result.has_value() && (!result.has_value() || result->distance > child_result->distance))
            result.emplace(*child_result);
    }

    return result;
}


void Object::add_to_render(Renderer &renderer, ray::Matrix &parent_transform, SelectionMode selection_mode, Selection &selection, float selection_color_factor) {
    // multiplying parent transform by the model transform of this object
    ray::Matrix mvp_matrix = ray::MatrixMultiply(get_model_matrix(), parent_transform);

    // transforming vertices
    std::vector<ray::Vector3> transformed_vertices{};
    for (auto &v: vertices) {
        transformed_vertices.push_back(apply_transformation(v, mvp_matrix));
    }

    auto kvp = selection.find(this);
    if (kvp == selection.end()) {
        // object is not part of selection, render normally

        // rendering triangles
        int len = triangle_colors.size();
        for (int i=0; i<len;i++) {
            renderer.cull_or_add_to_bsp_tree(
                    Triangle {
                            .v0 = transformed_vertices[triangle_indexes[i*3  ]],
                            .v1 = transformed_vertices[triangle_indexes[i*3+1]],
                            .v2 = transformed_vertices[triangle_indexes[i*3+2]],
                    }, triangle_colors[i]);
        }
    } else {
        if (selection_mode == SelectionMode::Vertex) {
            // todo: render selected vertices
        }

        // rendering triangles
        int len = triangle_colors.size();
        for (int i=0; i<len;i++) {
            ray::Color col = triangle_colors[i];
            if (selection_mode == SelectionMode::Object || kvp->second.contains(i))
                // todo: mix color
                col = ray::ColorBrightness(SELECTION_COLOR, selection_color_factor);

            renderer.cull_or_add_to_bsp_tree(
                    Triangle {
                            .v0 = transformed_vertices[triangle_indexes[i*3  ]],
                            .v1 = transformed_vertices[triangle_indexes[i*3+1]],
                            .v2 = transformed_vertices[triangle_indexes[i*3+2]],
                    }, col);
        }
    }

    // rendering children
    for(Object &child : children) {
        child.add_to_render(renderer, mvp_matrix, selection_mode, selection, selection_color_factor);
    }
}

#pragma region new_objects

Object Object::new_triangle() {
    float d = 2 * std::sqrt(2) /3.;
    return Object {
            .name = "Triangle",
            .vertices = {
                    {0,0,2./3.},{0,-d,-1./3.},{0,d,-1./3.},
            },
            .triangle_indexes = {0,1,2},
            .triangle_colors = {ray::RED},
    };
}

Object* Object::new_cube(int index) {
    std::vector<ray::Color> colors = {ray::RED, ray::MAROON, ray::LIME, ray::GREEN, ray::BLUE, ray::DARKBLUE};
    for (int i=6;i<12;i++){
        colors.push_back(ray::ColorBrightness(colors[i-6],-0.7));
    }

    return new Object{
        .name =  "Cube "+  std::to_string(index),
        .vertices = {
                {-1,-1, 1},{1,-1, 1},{-1, 1, 1},{ 1, 1, 1},
                {-1,-1,-1},{1,-1,-1},{-1, 1,-1},{ 1, 1,-1},
        },
        .triangle_indexes = {
            0,1,2,1,3,2,
            1,5,3,5,7,3,
            2,3,6,3,7,6,
            4,7,5,4,6,7,
            0,6,4,0,2,6,
            0,4,1,1,4,5,
        },
        .triangle_colors = colors
    };
}

Object Object::new_cylinder(int nr_vertices) {
    std::vector<ray::Vector3> vertices{};
    vertices.resize(nr_vertices*2);
    for (int i=0;i<nr_vertices;i++) {
        float angle = 2. * PI * i / nr_vertices;
        float x=cos(angle),z=sin(angle);

        vertices[i] = {x,1.,z};
        vertices[i+nr_vertices] = {x,-1.,z};
    }

    std::vector<int> triangle_indexes{};
    std::vector<ray::Color> triangle_colors{};

    // top face
    for (int i=2;i<nr_vertices;i++) {
        triangle_indexes.insert(triangle_indexes.end(), { 0, i, i-1});
        triangle_colors.push_back(ray::ColorBrightness(ray::RED,-1.1 + (i / (float)nr_vertices)));
    }

    // bottom face
    for (int i=nr_vertices+2;i<nr_vertices*2;i++) {
        triangle_indexes.insert(triangle_indexes.end(), { nr_vertices, i-1, i});
        triangle_colors.push_back(ray::ColorBrightness(ray::BLUE,-1.1 + (i / (2.*nr_vertices))));
    }

    // sides
    for (int i=0;i<nr_vertices - 1; i++) {
        float brightness = -0.8 + (i / (float)nr_vertices);
        triangle_indexes.insert(triangle_indexes.end(), { i, i+1, i+nr_vertices});
        triangle_colors.push_back(ray::ColorBrightness(ray::LIME, brightness));
        triangle_indexes.insert(triangle_indexes.end(), { i+1, i+nr_vertices+1, i+nr_vertices});
        triangle_colors.push_back(ray::ColorBrightness(ray::LIME, brightness));
    }
    triangle_indexes.insert(triangle_indexes.end(), { nr_vertices-1,0, 2*nr_vertices-1});
    triangle_colors.push_back(ray::LIME);
    triangle_indexes.insert(triangle_indexes.end(), { 0,nr_vertices, 2*nr_vertices-1});
    triangle_colors.push_back(ray::LIME);

    return {.name="Cylinder", .vertices = vertices, .triangle_indexes = triangle_indexes, .triangle_colors = triangle_colors};
}

#pragma endregion

void CameraSettings::input_movement() {
    const float wasd_sensitivity = 2.3;
    const float mouse_wheel_sensitivity = 13;
    const float mouse_sensitivity = 0.7;

    /// WASDRF MOVEMENT
    ray::Vector3 ws_dir = {std::sin(yaw), 0, std::cos(yaw)};
    ray::Vector3 ad_dir = {ws_dir.z, 0, -ws_dir.x};
    ws_dir = ray::Vector3Scale(ws_dir, (ray::IsKeyDown(ray::KEY_W) * -1 + ray::IsKeyDown(ray::KEY_S) * 1) * wasd_sensitivity);
    ad_dir = ray::Vector3Scale(ad_dir, (ray::IsKeyDown(ray::KEY_A) * -1 + ray::IsKeyDown(ray::KEY_D) * 1) * wasd_sensitivity);
    auto rf_dir = ray::Vector3Scale({0,1,0}, (ray::IsKeyDown(ray::KEY_R) * 1 + ray::IsKeyDown(ray::KEY_F) * -1) * wasd_sensitivity);
    auto sum = ray::Vector3Add(ws_dir, ray::Vector3Add(ad_dir, rf_dir));
    target = ray::Vector3Add(target, ray::Vector3Scale(sum, wasd_sensitivity * ray::GetFrameTime()));

    // orbiting distance
    distance += ray::GetMouseWheelMove() * -mouse_wheel_sensitivity * ray::GetFrameTime();

    // orbiting
    if ((ray::IsMouseButtonDown(ray::MOUSE_BUTTON_LEFT) && ray::IsKeyDown(ray::KEY_LEFT_SHIFT)) || ray::IsMouseButtonDown(ray::MOUSE_BUTTON_MIDDLE)) {
        auto mouse_delta = ray::Vector2Scale(ray::GetMouseDelta(),-mouse_sensitivity * ray::GetFrameTime() );
        yaw += mouse_sensitivity * mouse_delta.x;
        pitch += mouse_sensitivity * mouse_delta.y;
        pitch = std::min(std::max(pitch, -PI/2 + 0.01f), PI/2 - 0.01f);
    }
}

ray::Matrix CameraSettings::get_view_projection_matrix() {
    auto position = ray::Vector3RotateByQuaternion({0,0,distance}, ray::QuaternionFromEuler(pitch, yaw, 0));
    position = ray::Vector3Add(position, target);
    float aspect_ratio = screenWidth / (float) screenHeight;
    ray::Matrix matrix = ray::MatrixMultiply(
            // view matrix
            ray::MatrixLookAt(position, target, {0, 1, 0}),
            // perspective projection matrix
            ray::MatrixPerspective(fov, aspect_ratio, 0.001, 200)
            );

    // projection matrix flips the handedness for some reason so we unflip it and substract one on the z axis so it's in the range (-inf, 0]
    matrix = ray::MatrixMultiply(matrix, ray::MatrixScale(1,-1,1));
    matrix = ray::MatrixMultiply(matrix, ray::MatrixTranslate(0,0,-1));
    return matrix;
}

Ray CameraSettings::ray_from_mouse_position(int x, int y) {
    ray::Vector3 clip_space_point = {(x/(float)screenWidth-0.5f)*2,(y/(float)screenHeight-0.5f)*2,-1};
    ray::Matrix inverse_vp = ray::MatrixInvert(get_view_projection_matrix());
    auto world_space_point = apply_transformation(clip_space_point, inverse_vp);
    auto origin = apply_transformation({0,0,-0.001}, inverse_vp);
    Ray r = {
            .origin = origin,
            .direction = ray::Vector3Normalize(ray::Vector3Subtract(world_space_point, origin)),
    };
    debug_text(ray::TextFormat("origin:%s direction:%s wsp:%s", v3_to_text(r.origin), v3_to_text(r.direction),v3_to_text(world_space_point)));
    return r;
}

void World::raycast_and_add_to_selection(int x, int y) {
    Ray r = camera.ray_from_mouse_position(x, y);
    ray::Matrix identity = ray::MatrixIdentity();

    std::optional<RaycastResult> result{};
    for (Object* obj : objects) {
        auto obj_result = obj->raycast(r, selection_mode, identity);
        if (obj_result.has_value() && (!result.has_value() || result->distance > obj_result->distance))
            result.emplace(*obj_result);
    }
    if (!result.has_value()) return;

    selection[result->obj].insert(result->index);
}

void World::raycast_and_remove_from_selection(int x, int y) {
    Ray r = camera.ray_from_mouse_position(x, y);
    ray::Matrix identity = ray::MatrixIdentity();

    std::optional<RaycastResult> result{};
    for (Object* obj : objects) {
        auto obj_result = obj->raycast(r, selection_mode, identity);
        if (obj_result.has_value() && (!result.has_value() || result->distance > obj_result->distance))
            result.emplace(*obj_result);
    }
    if (!result.has_value()) return;

    selection[result->obj].erase(result->index);
}

void World::addNewObject(Object* object){
    this->objects.push_back(object);
    this->menu->addToMenu(object);
}

void World::render() {
    Renderer renderer{};

    ray::Matrix vp_matrix = camera.get_view_projection_matrix();

    float selection_color_factor = sin(SELECTION_FREQUENCY * ray::GetTime());

    for(Object* obj : objects) {
        if(obj->is_visible) {
            obj->add_to_render(renderer, vp_matrix, selection_mode, selection, selection_color_factor);
        }
    }
    
    renderer.draw(debug_render);
}