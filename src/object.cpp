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
    return ray::Vector3{v4.x/v4.w, v4.y/v4.w, v4.z/v4.w};
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
                    ray::Vector3{0,0,2./3.},ray::Vector3{0,-d,-1./3.},ray::Vector3{0,d,-1./3.},
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
                ray::Vector3{-1,-1, 1},ray::Vector3{1,-1, 1},ray::Vector3{-1, 1, 1},ray::Vector3{ 1, 1, 1},
                ray::Vector3{-1,-1,-1},ray::Vector3{1,-1,-1},ray::Vector3{-1, 1,-1},ray::Vector3{ 1, 1,-1},
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

ray::Matrix World::CameraSettings::get_view_projection_matrix() {
    ray::Matrix matrix = ray::MatrixLookAt(position, target, up);
    float aspect_ratio = screenWidth / (float) screenHeight;
    matrix = ray::MatrixMultiply(
            // view matrix
            ray::MatrixLookAt(position, target, up),
            // perspective projection matrix
            ray::MatrixPerspective(fov, aspect_ratio, 1, 200)
            );

    // projection matrix flips the handedness for some reason so we unflip it and substract one on the z axis so it's in the range (-inf, 0]
    matrix = ray::MatrixMultiply(matrix, ray::MatrixScale(1,-1,1));
    matrix = ray::MatrixMultiply(matrix, ray::MatrixTranslate(0,0,-1));
    return matrix;
}

void World::render(bool debug_render) {
    Renderer renderer{};

    ray::Matrix vp_matrix = camera.get_view_projection_matrix();

    for(Object &obj : objects) {
        obj.add_to_render(renderer, vp_matrix);
    }
    
    renderer.draw(debug_render);
}