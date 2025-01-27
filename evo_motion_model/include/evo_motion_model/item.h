//
// Created by samuel on 18/12/22.
//

#ifndef EVO_MOTION_ITEM_H
#define EVO_MOTION_ITEM_H

#include <memory>
#include <string>

#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>

#include "./shapes.h"

enum DrawableKind { SPECULAR, TILE_SPECULAR };

class Item {
public:
    Item(
        std::string name, const std::shared_ptr<Shape> &shape, glm::mat4 model_matrix,
        glm::vec3 scale, float mass, DrawableKind drawable_kind);

    Item(
        std::string name, const std::shared_ptr<Shape> &shape, glm::vec3 position, glm::vec3 scale,
        float mass, DrawableKind drawable_kind);

    Item(
        std::string name, const std::shared_ptr<Shape> &shape, glm::vec3 position,
        glm::quat rotation, glm::vec3 scale, float mass, DrawableKind drawable_kind);

    [[nodiscard]] std::shared_ptr<Shape> get_shape() const;

    [[nodiscard]] std::string get_name() const;

    [[nodiscard]] glm::mat4 model_matrix() const;

    [[nodiscard]] glm::mat4 model_matrix_without_scale() const;

    [[nodiscard]] btRigidBody *get_body() const;

    void reset(const glm::mat4 &main_model_matrix) const;

    DrawableKind get_drawable_kind() const;

private:
    std::string name;

    std::shared_ptr<Shape> shape;

    btRigidBody *body;
    btCollisionShape *collision_shape;

    glm::vec3 scale;

    glm::mat4 first_model_matrix;

    DrawableKind kind;
};

#endif//EVO_MOTION_ITEM_H