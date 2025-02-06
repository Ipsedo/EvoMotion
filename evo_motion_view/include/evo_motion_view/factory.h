//
// Created by samuel on 11/12/24.
//

#ifndef EVO_MOTION_FACTORY_H
#define EVO_MOTION_FACTORY_H

#include <functional>
#include <optional>

#include "./drawable.h"

class ObjSpecularFactory : public DrawableFactory {
private:
    std::uniform_real_distribution<float> rd_uni;

    std::vector<std::tuple<float, float, float>> vertices;
    std::vector<std::tuple<float, float, float>> normals;
    glm::vec4 ambient_color;
    glm::vec4 diffuse_color;
    glm::vec4 specular_color;
    float shininess;

public:
    ObjSpecularFactory(
        const std::vector<std::tuple<float, float, float>> &vertices,
        const std::vector<std::tuple<float, float, float>> &normals, glm::vec4 ambient_color,
        glm::vec4 diffuse_color, glm::vec4 specular_color, float shininess);

    ObjSpecularFactory(
        const std::vector<std::tuple<float, float, float>> &vertices,
        const std::vector<std::tuple<float, float, float>> &normals, std::mt19937 &rng,
        float shininess);

    std::shared_ptr<Drawable> get_drawable() override;
};

class EdgeObjSpecularFactory : public DrawableFactory {
private:
    std::uniform_real_distribution<float> rd_uni;

    std::vector<std::tuple<float, float, float>> vertices;
    std::vector<std::tuple<float, float, float>> normals;
    glm::vec4 ambient_color;
    glm::vec4 diffuse_color;
    glm::vec4 specular_color;
    float shininess;

    std::optional<std::function<bool()>> is_focus_function;

public:
    EdgeObjSpecularFactory(
        const std::vector<std::tuple<float, float, float>> &vertices,
        const std::vector<std::tuple<float, float, float>> &normals, glm::vec4 ambient_color,
        glm::vec4 diffuse_color, glm::vec4 specular_color, float shininess,
        const std::optional<std::function<bool()>> &is_focus_function);

    EdgeObjSpecularFactory(
        const std::vector<std::tuple<float, float, float>> &vertices,
        const std::vector<std::tuple<float, float, float>> &normals, std::mt19937 &rng,
        float shininess, const std::optional<std::function<bool()>> &is_focus_function);

    std::shared_ptr<Drawable> get_drawable() override;
};

class TileGroundFactory : public DrawableFactory {
private:
    std::uniform_real_distribution<float> rd_uni;

    std::vector<std::tuple<float, float, float>> vertices;
    std::vector<std::tuple<float, float, float>> normals;

    glm::vec4 ambient_color_a;
    glm::vec4 diffuse_color_a;
    glm::vec4 specular_color_a;

    glm::vec4 ambient_color_b;
    glm::vec4 diffuse_color_b;
    glm::vec4 specular_color_b;

    float shininess;

    float tile_size;

public:
    TileGroundFactory(
        const std::vector<std::tuple<float, float, float>> &vertices,
        const std::vector<std::tuple<float, float, float>> &normals, glm::vec4 ambient_color_a,
        glm::vec4 diffuse_color_a, glm::vec4 specular_color_a, glm::vec4 ambient_color_b,
        glm::vec4 diffuse_color_b, glm::vec4 specular_color_b, float shininess, float tile_size);

    TileGroundFactory(
        const std::vector<std::tuple<float, float, float>> &vertices,
        const std::vector<std::tuple<float, float, float>> &normals, std::mt19937 &rng,
        float shininess, float tile_size);

    std::shared_ptr<Drawable> get_drawable() override;
};

#endif//EVO_MOTION_FACTORY_H
