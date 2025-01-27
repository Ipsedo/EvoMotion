//
// Created by samuel on 11/12/24.
//

#ifndef EVO_MOTION_FACTORY_H
#define EVO_MOTION_FACTORY_H

#include "./drawable.h"

class ObjSpecularFactory : public DrawableFactory {
private:
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

    std::shared_ptr<Drawable> get_drawable() override;
};

class TileGroundFactory : public DrawableFactory {
private:
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

    std::shared_ptr<Drawable> get_drawable() override;
};

#endif//EVO_MOTION_FACTORY_H
