//
// Created by samuel on 06/05/24.
//

#include <evo_motion_view/drawable.h>

#include "./specular.h"

std::shared_ptr<Drawable> Drawable::Builder::make_specular_obj(
    const std::vector<std::tuple<float, float, float>> &vertices,
    const std::vector<std::tuple<float, float, float>> &normals, glm::vec4 ambient_color,
    glm::vec4 diffuse_color, glm::vec4 specular_color, float shininess) {
    return std::make_shared<OBjSpecular>(
        vertices, normals, ambient_color, diffuse_color, specular_color, shininess);
}

Drawable::~Drawable() = default;