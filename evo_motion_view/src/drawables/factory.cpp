//
// Created by samuel on 06/05/24.
//

#include <evo_motion_view/factory.h>

#include "./ground.h"
#include "./specular.h"

// ObjSpecular

ObjSpecularFactory::ObjSpecularFactory(
    const std::vector<std::tuple<float, float, float>> &vertices,
    const std::vector<std::tuple<float, float, float>> &normals, glm::vec4 ambient_color,
    glm::vec4 diffuse_color, glm::vec4 specular_color, float shininess)
    : vertices(vertices), normals(normals), ambient_color(ambient_color),
      diffuse_color(diffuse_color), specular_color(specular_color), shininess(shininess) {}

std::shared_ptr<Drawable> ObjSpecularFactory::get_drawable() {
    return std::make_shared<OBjSpecular>(
        vertices, normals, ambient_color, diffuse_color, specular_color, shininess);
}

// TileGround

TileGroundFactory::TileGroundFactory(
    const std::vector<std::tuple<float, float, float>> &vertices,
    const std::vector<std::tuple<float, float, float>> &normals, glm::vec4 ambient_color_a,
    glm::vec4 diffuse_color_a, glm::vec4 specular_color_a, glm::vec4 ambient_color_b,
    glm::vec4 diffuse_color_b, glm::vec4 specular_color_b, float shininess, float tile_size)
    : vertices(vertices), normals(normals), ambient_color_a(ambient_color_a),
      diffuse_color_a(diffuse_color_a), specular_color_a(specular_color_a),
      ambient_color_b(ambient_color_b), diffuse_color_b(diffuse_color_b),
      specular_color_b(specular_color_b), shininess(shininess), tile_size(tile_size) {}

std::shared_ptr<Drawable> TileGroundFactory::get_drawable() {
    return std::make_shared<TileGround>(
        vertices, normals, ambient_color_a, diffuse_color_a, specular_color_a, ambient_color_b,
        diffuse_color_b, specular_color_b, shininess, tile_size);
}

// Edge Specular

EdgeObjSpecularFactory::EdgeObjSpecularFactory(
    const std::vector<std::tuple<float, float, float>> &vertices,
    const std::vector<std::tuple<float, float, float>> &normals, glm::vec4 ambient_color,
    glm::vec4 diffuse_color, glm::vec4 specular_color, float shininess,
    const std::optional<std::function<bool()>> &is_focus_function)
    : vertices(vertices), normals(normals), ambient_color(ambient_color),
      diffuse_color(diffuse_color), specular_color(specular_color), shininess(shininess),
      is_focus_function(is_focus_function) {}

std::shared_ptr<Drawable> EdgeObjSpecularFactory::get_drawable() {
    return std::make_shared<EdgeObjSpecular>(
        vertices, normals, ambient_color, diffuse_color, specular_color, shininess,
        is_focus_function);
}
