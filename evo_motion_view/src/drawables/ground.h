//
// Created by samuel on 11/12/24.
//

#ifndef EVO_MOTION_GROUND_H
#define EVO_MOTION_GROUND_H

#include <evo_motion_view/drawable.h>

#include "../program.h"

class TileGround final : public Drawable {
    const int position_size;
    const int normal_size;
    const int stride;

    glm::vec4 ambient_color_a;
    glm::vec4 diffuse_color_a;
    glm::vec4 specular_color_a;

    glm::vec4 ambient_color_b;
    glm::vec4 diffuse_color_b;
    glm::vec4 specular_color_b;

    float shininess;

    int nb_vertices;

    float tile_size;

    Program program;

    static std::vector<float> to_vbo_data(
        const std::vector<std::tuple<float, float, float>> &vertices,
        const std::vector<std::tuple<float, float, float>> &normals);

public:
    TileGround(
        const std::vector<std::tuple<float, float, float>> &vertices,
        const std::vector<std::tuple<float, float, float>> &normals, glm::vec4 ambient_color_a,
        glm::vec4 diffuse_color_a, glm::vec4 specular_color_a, glm::vec4 ambient_color_b,
        glm::vec4 diffuse_color_b, glm::vec4 specular_color_b, float shininess, float tile_size);

    void draw(
        glm::mat4 projection_matrix, glm::mat4 view_matrix, glm::mat4 model_matrix,
        glm::vec3 light_pos_from_camera, glm::vec3 camera_pos) override;

    ~TileGround() override;
};

#endif//EVO_MOTION_GROUND_H
