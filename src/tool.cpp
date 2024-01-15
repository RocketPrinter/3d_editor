#include <set>
#include <utility>
#include "tool.h"

const Tool vertex_tools[] = {
    Tool {
        .name = "Create vertex",
        .click = [](World& world) {
            for (auto kvp: world.selection) {
                // creates a new vertex at the barycenter of the selected points in the object
                ray::Vector3 center{};
                for (int i: kvp.second)
                    center = ray::Vector3Add(kvp.first->vertices[i], center);
                center = ray::Vector3Scale(center, 1/(float)kvp.second.size());

                kvp.second.clear();
                kvp.second.insert(kvp.first->vertices.size());

                kvp.first->vertices.push_back(center);
            }
        },
    },

    Tool {
            .name = "Delete",
            .click = [](World& world) {
                // todo: not quite working FOR SOME REASON
                std::set<int> triangles_to_remove{};

                for (auto kvp: world.selection) {
                    auto &triangle_indexes = kvp.first->triangle_indexes;
                    auto &triangle_colors = kvp.first->triangle_colors;

                    for (int i=0; i < triangle_indexes.size();i++)
                        if (kvp.second.contains(triangle_indexes[i]))
                            triangles_to_remove.insert(i/3);

                    int count = 0;
                    std::remove_if(triangle_indexes.begin(), triangle_indexes.end(),[&](int&){ return triangles_to_remove.contains((count++)/3); });
                    count = 0;
                    std::remove_if(triangle_colors.begin(), triangle_colors.end(),[&](ray::Color&){ return triangles_to_remove.contains(count++); });

                    triangles_to_remove.clear();
                }
                world.selection.clear();
            },
    },

    Tool {
            .name = "Create triangle",
            .click = [](World& world) {
                for (auto kvp: world.selection) {
                    // for each object if 3 vertices are selected we use it to build a triangle and we make sure the visible part is towards the camera
                    if (kvp.second.size() != 3) continue;

                    int v0,v1,v2;
                    auto iter = kvp.second.begin();
                    v0 = *iter; iter++;
                    v1 = *iter; iter++;
                    v2 = *iter;
                    auto v0p = kvp.first->vertices[v0];

                    auto cross = ray::Vector3CrossProduct(
                            ray::Vector3Subtract(kvp.first->vertices[v1], v0p),
                            ray::Vector3Subtract(kvp.first->vertices[v2], v0p)
                            );
                    bool winding = ray::Vector3DotProduct(cross, ray::Vector3Subtract(v0p, world.camera.get_position())) > 0;

                    kvp.first->triangle_indexes.push_back(v0);
                    kvp.first->triangle_indexes.push_back(winding?v1:v2);
                    kvp.first->triangle_indexes.push_back(winding?v2:v1);
                    kvp.first->triangle_colors.push_back(ray::RED);
                }
            },
    },
};

const Tool triangle_tools[] = {
    Tool {
            .name = "Flip normals",
            .click = [](World& world) {
                for (auto kvp: world.selection) {
                    for (int i: kvp.second) {
                        std::swap(kvp.first->triangle_indexes[i*3+1], kvp.first->triangle_indexes[i*3+2]);
                    }
                }
            },
    },

    Tool {
            .name = "Delete",
            .click = [](World& world) {
                for (auto kvp: world.selection) {
                    auto &triangle_indexes = kvp.first->triangle_indexes;
                    auto &triangle_colors = kvp.first->triangle_colors;

                    int count = 0;
                    std::remove_if(triangle_indexes.begin(), triangle_indexes.end(),[&](int&){ return kvp.second.contains((count++)/3); });
                    count = 0;
                    std::remove_if(triangle_colors.begin(), triangle_colors.end(),[&](ray::Color&){ return kvp.second.contains(count++); });
                }
                world.selection.clear();
            },
    },

    Tool {
            .name = "Alpha +",
            .click = [](World& world) {
                for (auto kvp: world.selection) {
                    for (int i: kvp.second) {
                        ray::Color &col = kvp.first->triangle_colors[i];
                        if (col.a >= 224) col.a = 255;
                        else col.a += 32;
                    }
                }
            },
    },

    Tool {
            .name = "Alpha -",
            .click = [](World& world) {
                for (auto kvp: world.selection) {
                    for (int i: kvp.second) {
                        ray::Color &col = kvp.first->triangle_colors[i];
                        col.a = std::max(63, col.a - 32);
                    }
                }
            },
    },
};

const float text_size=12, rect_height=20, bottom_padding = 3, horizontal_padding = 6, horizontal_text_padding=6, vertical_text_padding=4;

void show_tools(World& world) {
    float x_offset=horizontal_padding, width;
    if (world.selection_mode == SelectionMode::Vertex) {
        for (auto tool: vertex_tools) {
            tool.show(world, x_offset, width);
            x_offset += width;
        }
    } else if (world.selection_mode == SelectionMode::Triangle) {
        for (auto tool: triangle_tools) {
            tool.show(world, x_offset, width);
            x_offset += width;
        }
    }
}

void Tool::show(World &world, float x_offset, float &width) {
    float text_width = ray::MeasureText(this->name.c_str(), text_size);
    float rect_width = text_width + horizontal_text_padding * 2;
    width = rect_width + horizontal_padding;

    ray::Rectangle rect = {x_offset, screenHeight - (bottom_padding + rect_height), rect_width,rect_height};
    bool selected = ray::CheckCollisionPointRec(ray::GetMousePosition(), rect);

    ray::DrawRectangleRec(rect, selected? ray::GREEN : ray::LIGHTGRAY);
    ray::DrawText(this->name.c_str(), x_offset + horizontal_text_padding, screenHeight - (bottom_padding + rect_height - vertical_text_padding), text_size, ray::DARKGRAY);

    if (selected && ray::IsMouseButtonPressed(ray::MOUSE_BUTTON_LEFT))
        click(world);
}
