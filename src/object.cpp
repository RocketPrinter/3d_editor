#include "object.h"

ray::Matrix Object::get_model_matrix() {
    ray::Matrix model = ray::MatrixScale(scale.x, scale.y, scale.z);
    model = ray::MatrixMultiply(model, ray::QuaternionToMatrix(rotation));
    model = ray::MatrixMultiply(model, ray::MatrixTranslate(position.x, position.y, position.z));
    return model;
}

void Object::render(RenderContext &ctx, ray::Matrix &parent_transform) {
    ray::Matrix matrix = get_model_matrix();

    // multiplying parent transform by the model transform of this object
    matrix = ray::MatrixMultiply(matrix, parent_transform);

    // adding triangles
    // todo;

    // rendering children
    for(Object &child : children) {
        child.render(ctx, matrix);
    }
}
#pragma region new_objects

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
    matrix = ray::MatrixMultiply(
            // view matrix
            ray::MatrixLookAt(position, target, up),
            is_perspective ?
                // perspective projection matrix
                ray::MatrixPerspective(fov, screenWidth / (float) screenHeight, 1, 200) :
                // todo: orthographic projection matrix
                ray::Matrix{}
            );
    // projection matrix flips the handedness for some reason so we unflip it and substract one on the z axis so it's in the range (-inf, 0]
    matrix = ray::MatrixMultiply(matrix, ray::MatrixScale(1,-1,1));
    matrix = ray::MatrixMultiply(matrix, ray::MatrixTranslate(0,0,-1));
    return matrix;
}

void World::render() {
    RenderContext ctx{};

    ray::Matrix matrix = camera.get_view_projection_matrix();

    for(Object &obj : objects) {
        obj.render(ctx, matrix);
    }
}