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
    : rd_uni(0.f, 1.f), vertices(vertices), normals(normals), ambient_color(ambient_color),
      diffuse_color(diffuse_color), specular_color(specular_color), shininess(shininess) {}

ObjSpecularFactory::ObjSpecularFactory(
    const std::vector<std::tuple<float, float, float>> &vertices,
    const std::vector<std::tuple<float, float, float>> &normals, std::mt19937 &rng,
    const float shininess)
    : rd_uni(0.f, 1.f), vertices(vertices), normals(normals),
      ambient_color(glm::vec4(rd_uni(rng), rd_uni(rng), rd_uni(rng), 1.f)),
      diffuse_color(glm::vec4(rd_uni(rng), rd_uni(rng), rd_uni(rng), 1.f)),
      specular_color(glm::vec4(rd_uni(rng), rd_uni(rng), rd_uni(rng), 1.f)), shininess(shininess) {}

std::shared_ptr<Drawable> ObjSpecularFactory::create_drawable() {
    return std::make_shared<OBjSpecular>(
        vertices, normals, ambient_color, diffuse_color, specular_color, shininess);
}

// TileGround

TileGroundFactory::TileGroundFactory(
    const std::vector<std::tuple<float, float, float>> &vertices,
    const std::vector<std::tuple<float, float, float>> &normals, glm::vec4 ambient_color_a,
    glm::vec4 diffuse_color_a, glm::vec4 specular_color_a, glm::vec4 ambient_color_b,
    glm::vec4 diffuse_color_b, glm::vec4 specular_color_b, const float shininess,
    const float tile_size)
    : rd_uni(0.f, 1.f), vertices(vertices), normals(normals), ambient_color_a(ambient_color_a),
      diffuse_color_a(diffuse_color_a), specular_color_a(specular_color_a),
      ambient_color_b(ambient_color_b), diffuse_color_b(diffuse_color_b),
      specular_color_b(specular_color_b), shininess(shininess), tile_size(tile_size) {}

TileGroundFactory::TileGroundFactory(
    const std::vector<std::tuple<float, float, float>> &vertices,
    const std::vector<std::tuple<float, float, float>> &normals, std::mt19937 &rng,
    const float shininess, const float tile_size)
    : rd_uni(0.f, 1.f), vertices(vertices), normals(normals),
      ambient_color_a(glm::vec4(rd_uni(rng), rd_uni(rng), rd_uni(rng), 1.f)),
      diffuse_color_a(glm::vec4(rd_uni(rng), rd_uni(rng), rd_uni(rng), 1.f)),
      specular_color_a(glm::vec4(rd_uni(rng), rd_uni(rng), rd_uni(rng), 1.f)),
      ambient_color_b(glm::vec4(rd_uni(rng), rd_uni(rng), rd_uni(rng), 1.f)),
      diffuse_color_b(glm::vec4(rd_uni(rng), rd_uni(rng), rd_uni(rng), 1.f)),
      specular_color_b(glm::vec4(rd_uni(rng), rd_uni(rng), rd_uni(rng), 1.f)), shininess(shininess),
      tile_size(tile_size) {}

std::shared_ptr<Drawable> TileGroundFactory::create_drawable() {
    return std::make_shared<TileGround>(
        vertices, normals, ambient_color_a, diffuse_color_a, specular_color_a, ambient_color_b,
        diffuse_color_b, specular_color_b, shininess, tile_size);
}

// Edge Specular

BuilderObjSpecularFactory::BuilderObjSpecularFactory(
    const std::vector<std::tuple<float, float, float>> &vertices,
    const std::vector<std::tuple<float, float, float>> &normals, const glm::vec4 ambient_color,
    const glm::vec4 diffuse_color, const glm::vec4 specular_color, const float shininess,
    const std::optional<std::function<bool()>> &is_focus_function,
    const std::optional<std::function<bool()>> &is_hidden_function)
    : rd_uni(0.f, 1.f), vertices(vertices), normals(normals), ambient_color(ambient_color),
      diffuse_color(diffuse_color), specular_color(specular_color), shininess(shininess),
      is_focus_function(is_focus_function), is_hidden_function(is_hidden_function) {}

BuilderObjSpecularFactory::BuilderObjSpecularFactory(
    const std::vector<std::tuple<float, float, float>> &vertices,
    const std::vector<std::tuple<float, float, float>> &normals, std::mt19937 &rng,
    const float shininess, const std::optional<std::function<bool()>> &is_focus_function,
    const std::optional<std::function<bool()>> &is_hidden_function)
    : rd_uni(0.f, 1.f), vertices(vertices), normals(normals),
      ambient_color(glm::vec4(rd_uni(rng), rd_uni(rng), rd_uni(rng), 1.f)),
      diffuse_color(glm::vec4(rd_uni(rng), rd_uni(rng), rd_uni(rng), 1.f)),
      specular_color(glm::vec4(rd_uni(rng), rd_uni(rng), rd_uni(rng), 1.f)), shininess(shininess),
      is_focus_function(is_focus_function), is_hidden_function(is_hidden_function) {}

std::shared_ptr<Drawable> BuilderObjSpecularFactory::create_drawable() {
    return std::make_shared<BuilderObjSpecular>(
        vertices, normals, ambient_color, diffuse_color, specular_color, shininess,
        is_focus_function, is_hidden_function);
}
