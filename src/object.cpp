#include "object.h"

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
            if (!result.has_value() || new_result.distance < result->distance)
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

Object Object::new_plane(int divisions_x, int divisions_z) {
    std::vector<ray::Vector3> vertices{};
    vertices.reserve(divisions_x * divisions_z);

    for(int i=0;i<divisions_x;i++)
    {
        float x = (i/(divisions_x-1.))*2.-1.;
        for (int j=0;j<divisions_z;j++)
            vertices.push_back({x,0,(j/(divisions_z-1.0f))*2.0f-1.0f});
    }

    std::vector<int> triangle_indexes{};
    std::vector<ray::Color> triangle_colors{};
    for (int i=0;i<divisions_x-1;i++)
        for (int j=0;j<divisions_z-1;j++) {
            int p = i * divisions_z+j;
            triangle_indexes.insert(triangle_indexes.end(),{p,p+1,p+divisions_z});
            triangle_colors.push_back(ray::LIME);
            triangle_indexes.insert(triangle_indexes.end(),{p+1,p+divisions_z+1,p+divisions_z});
            triangle_colors.push_back(ray::GREEN);
        }

    return {.name="Plane", .vertices = vertices, .triangle_indexes = triangle_indexes, .triangle_colors = triangle_colors};
}

Object Object::new_cube() {
    std::vector<ray::Color> colors = {ray::RED, ray::MAROON, ray::LIME, ray::GREEN, ray::BLUE, ray::DARKBLUE};
    for (int i=6;i<12;i++){
        colors.push_back(ray::ColorBrightness(colors[i-6],-0.7));
    }

    return Object{
        .name =  "Cube",
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

Object Object::new_cone(int nr_vertices) {
    std::vector<ray::Vector3> vertices{};
    vertices.resize(nr_vertices+1);
    for (int i=0;i<nr_vertices;i++) {
        float angle = 2. * PI * i / nr_vertices;
        float x=cos(angle),z=sin(angle);

        vertices[i] = {x,-1./3.,z};
    }
    vertices[nr_vertices] = {0,2./3.,0};

    std::vector<int> triangle_indexes{};
    std::vector<ray::Color> triangle_colors{};

    // conical face
    for (int i=0;i<nr_vertices-1;i++) {
        triangle_indexes.insert(triangle_indexes.end(), { i+1, i, nr_vertices});
        triangle_colors.push_back(ray::ColorBrightness(ray::LIME,-0.8 + (i / (float)nr_vertices)));
    }
    triangle_indexes.insert(triangle_indexes.end(), {0,nr_vertices-1,nr_vertices});
    triangle_colors.push_back(ray::ColorBrightness(ray::LIME,0.1));

    // bottom face
    for (int i=0;i<nr_vertices-1;i++) {
        triangle_indexes.insert(triangle_indexes.end(), { 0, i, i+1});
        triangle_colors.push_back(ray::ColorBrightness(ray::BLUE,-1.1 + (i / (float) nr_vertices)));
    }

    return {.name="Cone", .vertices = vertices, .triangle_indexes = triangle_indexes, .triangle_colors = triangle_colors};
}

Object Object::new_sphere(int meridians, int parallels) {
    std::vector<ray::Vector3> vertices{};
    vertices.push_back({0,1,0}); // north pole, parallel 0
    for (int i=1;i<parallels;i++) {
        float theta = (i/(float)parallels) * PI;
        float sin_theta = std::sin(theta);
        float cos_theta = std::cos(theta);
        for(int j=0;j<meridians;j++) {
            float phi = 2. * PI * j / meridians;
            vertices.push_back({std::cos(phi) * sin_theta, cos_theta, std::sin(phi) * sin_theta});
        }
    }
    vertices.push_back({0,-1,0}); // south pole, parallel paralells

    std::vector<int> triangle_indexes{};

    // top triangles
    for(int i=1;i<meridians;i++) {
        triangle_indexes.insert(triangle_indexes.end(), {0,i+1,i});
    }
    triangle_indexes.insert(triangle_indexes.end(), {0, 1, meridians});// tf?

    // quads between parallels
    for (int i=0;i<parallels-2;i++) {
        int line_start = 1 + i * meridians;
        for (int j=0;j<meridians-1;j++) {
            int k = line_start + j;
            triangle_indexes.insert(triangle_indexes.end(), {k,k+1,k+meridians});
            triangle_indexes.insert(triangle_indexes.end(), {k+1,k+meridians+1,k+meridians});
        }
        triangle_indexes.insert(triangle_indexes.end(), {line_start+meridians-1, line_start, line_start+2*meridians-1});
        triangle_indexes.insert(triangle_indexes.end(), {line_start,line_start+meridians, line_start+2*meridians-1});
    }

    // bottom triangles
    int last = vertices.size()-1;
    for(int i=last-meridians;i<last-1;i++) {
        triangle_indexes.insert(triangle_indexes.end(), {last,i,i+1});
    }
    triangle_indexes.insert(triangle_indexes.end(), {last,last-1,last-meridians});

    // colors
    std::vector<ray::Color> triangle_colors{};
    int len = triangle_indexes.size()/3;
    triangle_colors.reserve(len);
    for (int i=0;i<len;i++) {
        auto center = ray::Vector3Add(vertices[triangle_indexes[i*3  ]],ray::Vector3Add(vertices[triangle_indexes[i*3+1]],vertices[triangle_indexes[i*3+2]]));
        center = ray::Vector3Scale(center, (1./3.)*255);
        triangle_colors.push_back({.r=(unsigned char)std::abs(center.x),.g=(unsigned char)std::abs(center.y),.b=(unsigned char)std::abs(center.z),.a=255});
    }

    return {.name="Sphere", .vertices = vertices, .triangle_indexes = triangle_indexes, .triangle_colors = triangle_colors};
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
            .direction = ray::Vector3Normalize(ray::Vector3Subtract(origin, world_space_point)),
    };
    debug_text(ray::TextFormat("origin:%s direction:%s wsp:%s", v3_to_text(r.origin), v3_to_text(r.direction),v3_to_text(world_space_point)));
    return r;
}

void World::raycast_and_modify_selection(int x, int y, bool remove_from_selection) {
    Ray r = camera.ray_from_mouse_position(x, y);
    ray::Matrix identity = ray::MatrixIdentity();

    std::optional<RaycastResult> result{};
    for (Object &obj : objects) {
        auto obj_result = obj.raycast(r, selection_mode, identity);
        if (obj_result.has_value() && (!result.has_value() || result->distance > obj_result->distance))
            result.emplace(*obj_result);
    }
    if (!result.has_value()) {
        if (true || debug_render) debug_text("raycast no hit");
        return;
    }
    if (true || debug_render) {
        debug_text(ray::TextFormat("raycast hit: name=%s pos=%s dist=%f",result->obj->name.c_str(), v3_to_text(result->hit_pos), result->distance));
        point_queue.push_back(RenderPoint{.pos=result->hit_pos,.col=ray::YELLOW});
    }

    if (!remove_from_selection)
        selection[result->obj].insert(result->index);
    else
        selection[result->obj].erase(result->index);
}

void World::render() {
    Renderer renderer{};

    ray::Matrix vp_matrix = camera.get_view_projection_matrix();

    float selection_color_factor = sin(SELECTION_FREQUENCY * ray::GetTime());

    for(Object &obj : objects) {
        obj.add_to_render(renderer, vp_matrix, selection_mode, selection, selection_color_factor);
    }

    for (auto point: point_queue) {
        renderer.add_point(vp_matrix, point);
    }
    point_queue.clear();

    renderer.draw(debug_render);
}