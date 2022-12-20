//
// Created by samuel on 18/12/22.
//

#ifndef EVO_MOTION_ITEM_H
#define EVO_MOTION_ITEM_H

#include <memory>
#include <string>

#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>

#include "./model/shapes.h"

class Item {
public:
    Item(std::string name, const std::shared_ptr<Shape> &shape, glm::vec3 position, glm::vec3 scale, float mass);

    std::shared_ptr<Shape> get_shape();

    std::string get_name();

    glm::mat4 model_matrix();

    btRigidBody *get_body();

private:
    std::string name;

    std::shared_ptr<Shape> shape;

    btRigidBody *body;
    btCollisionShape *collision_shape;

    glm::vec3 scale;
};

#endif //EVO_MOTION_ITEM_H
