#include "object.h"

ray::Matrix Object::get_model_matrix() {
    ray::Matrix model = ray::MatrixScale(scale.x, scale.y, scale.z);
    model = ray::MatrixMultiply(model, ray::QuaternionToMatrix(rotation));
    model = ray::MatrixMultiply(model, ray::MatrixTranslate(position.x, position.y, position.z));
    return model;
}

ray::Vector3 apply_transformation(ray::Vector3 v, ray::Matrix mvp_matrix) {
    // homogeneous coordinates
    ray::Vector4 v4 = ray::Vector4Transform(ray::Vector4FromVector3(v, 1.), mvp_matrix);
    // perspective divide/normalization
    return {v4.x/v4.w, v4.y/v4.w, v4.z/v4.w};
}

void Object::add_to_render(Renderer &renderer, ray::Matrix &parent_transform) {
    // multiplying parent transform by the model transform of this object
    ray::Matrix mvp_matrix = ray::MatrixMultiply(get_model_matrix(), parent_transform);

    // transforming vertices (todo: this allocation can be optimised)
    std::vector<ray::Vector3> transformed_vertices{};
    for (auto &v: vertices) {
        transformed_vertices.push_back(apply_transformation(v, mvp_matrix));
    }

    // adding triangles
    int len = triangle_colors.size();
    for (int i=0; i<len;i++) {
        renderer.cull_or_add_to_bsp_tree(
                Triangle {
                    .v0 = transformed_vertices[triangle_indexes[i*3  ]],
                    .v1 = transformed_vertices[triangle_indexes[i*3+1]],
                    .v2 = transformed_vertices[triangle_indexes[i*3+2]],
                    .col = triangle_colors[i],
                });
    }

    // rendering children
    for(Object &child : children) {
        child.add_to_render(renderer, mvp_matrix);
    }
}

#pragma region new_objects

Object Object::new_triangle() {
    float d = 2 * std::sqrt(2) /3.;
    return Object {
            .name = "New triangle",
            .vertices = {
                    {0,0,2./3.},{0,-d,-1./3.},{0,d,-1./3.},
            },
            .triangle_indexes = {0,1,2},
            .triangle_colors = {ray::RED},
    };
}

Object Object::new_cube() {
    std::vector<ray::Color> colors = {ray::RED, ray::MAROON, ray::LIME, ray::GREEN, ray::BLUE, ray::DARKBLUE};
    for (int i=6;i<12;i++){
        colors.push_back(ray::ColorBrightness(colors[i-6],-0.7));
    }

    return Object{
        .name =  "New cube",
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
            ray::MatrixPerspective(fov, aspect_ratio, 1, 200)
            );

    // projection matrix flips the handedness for some reason so we unflip it and substract one on the z axis so it's in the range (-inf, 0]
    matrix = ray::MatrixMultiply(matrix, ray::MatrixScale(1,-1,1));
    matrix = ray::MatrixMultiply(matrix, ray::MatrixTranslate(0,0,-1));
    return matrix;
}

Ray CameraSettings::ray_from_mouse_position(int x, int y) {
    // create Ray in clip space
    Ray r = {
            .origin = {(x/(float)screenWidth-0.5f)*2,(y/(float)screenHeight-0.5f)*2,0},
            .direction = {0,0,-1},
    };
    debug_text(ray::TextFormat("origin:%s direction:%s", v3_to_text(r.origin), v3_to_text(r.direction)));

    // we transform the Ray coordinates from clip space to world space
    ray::Matrix inverse_VP = ray::MatrixInvert(get_view_projection_matrix());
    r.origin = apply_transformation(r.origin, inverse_VP);
    r.direction = apply_transformation(r.direction, inverse_VP);

    debug_text(ray::TextFormat("origin:%s direction:%s", v3_to_text(r.origin), v3_to_text(r.direction)));
    return r;
}

void World::raycast_and_add_to_selection(int x, int y) {
    Ray r = camera.ray_from_mouse_position(x, y);

    std::optional<RaycastResult> result{};
    /*for (Object &obj : objects) {
        auto obj_result = obj.raycast(r, selection_mode);
        if (obj_result.has_value() && (!result.has_value() || result->distance > obj_result->distance))
            result.emplace(*obj_result);
    }*/
    if (!result.has_value()) return;

    selection[result->obj].insert(result->index);
}

void World::raycast_and_remove_from_selection(int x, int y) {
    Ray r = camera.ray_from_mouse_position(x, y);

    std::optional<RaycastResult> result{};
    /*for (Object &obj : objects) {
        auto obj_result = obj.raycast(r, selection_mode);
        if (obj_result.has_value() && (!result.has_value() || result->distance > obj_result->distance))
            result.emplace(*obj_result);
    }*/
    if (!result.has_value()) return;

    selection[result->obj].erase(result->index);
}


void World::render() {
    Renderer renderer{};

    ray::Matrix vp_matrix = camera.get_view_projection_matrix();

    for(Object &obj : objects) {
        obj.add_to_render(renderer, vp_matrix);
    }
    
    renderer.draw(debug_render);
}