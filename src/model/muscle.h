//
// Created by samuel on 30/12/23.
//

#ifndef EVO_MOTION_MUSCLE_H
#define EVO_MOTION_MUSCLE_H

#include <btBulletDynamicsCommon.h>

#include "./item.h"

class Muscle {
public:
    Muscle(const std::string &name, float attach_mass, glm::vec3 attach_scale, Item& item_a, glm::vec3 pos_in_a, Item& item_b, glm::vec3 pos_in_b);
    void contract(float force);
    void release();

    std::vector<Item> get_items();
    std::vector<btTypedConstraint *> get_constraints();
private:
    std::shared_ptr<Item> attach_a;
    std::shared_ptr<Item> attach_b;

    btSliderConstraint *muscle_slider_constraint;
    btConeTwistConstraint *attach_a_constraint;
    btConeTwistConstraint *attach_b_constraint;

};

#endif //EVO_MOTION_MUSCLE_H
