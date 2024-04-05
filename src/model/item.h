//
// Created by samuel on 18/12/22.
//

#ifndef EVO_MOTION_ITEM_H
#define EVO_MOTION_ITEM_H

#include <memory>
#include <string>

#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>

#include "shapes.h"

class Item {
public:
    Item(std::string name, const std::shared_ptr<Shape> &shape,
         glm::mat4 model_matrix, glm::vec3 scale, float mass);

    Item(std::string name, const std::shared_ptr<Shape> &shape,
         glm::vec3 position, glm::vec3 scale, float mass);

    Item(std::string name, const std::shared_ptr<Shape> &shape,
         glm::vec3 position, glm::quat rotation, glm::vec3 scale,
         float mass);

    std::shared_ptr<Shape> get_shape();

    std::string get_name();

    glm::mat4 model_matrix();

    glm::mat4 model_matrix_without_scale();

    btRigidBody *get_body();

    void reset();

private:
    std::string name;

    std::shared_ptr<Shape> shape;

    btRigidBody *body;
    btCollisionShape *collision_shape;

    glm::vec3 scale;

    glm::mat4 first_model_matrix;
};

#endif //EVO_MOTION_ITEM_H
