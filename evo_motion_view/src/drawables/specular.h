//
// Created by samuel on 17/12/22.
//

#ifndef EVO_MOTION_SPECULAR_H
#define EVO_MOTION_SPECULAR_H

#include <vector>

#include <glm/glm.hpp>

#include <evo_motion_view/drawable.h>

#include "../program.h"

class OBjSpecular final : public Drawable {
    const int position_size;
    const int normal_size;
    const int stride;

    glm::vec4 ambient_color;
    glm::vec4 diffuse_color;
    glm::vec4 specular_color;
    float shininess;

    int nb_vertices;

    Program program;

    static std::vector<float> to_vbo_data(
        const std::vector<std::tuple<float, float, float>> &vertices,
        const std::vector<std::tuple<float, float, float>> &normals);

public:
    OBjSpecular(
        const std::vector<std::tuple<float, float, float>> &vertices,
        const std::vector<std::tuple<float, float, float>> &normals, glm::vec4 ambient_color,
        glm::vec4 diffuse_color, glm::vec4 specular_color, float shininess);

    void draw(
        glm::mat4 projection_matrix, glm::mat4 view_matrix, glm::mat4 model_matrix,
        glm::vec3 light_pos_from_camera, glm::vec3 camera_pos) override;

    ~OBjSpecular() override;
};

#endif//EVO_MOTION_SPECULAR_H