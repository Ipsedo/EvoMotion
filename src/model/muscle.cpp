//
// Created by samuel on 30/12/23.
//

#include <memory>

#include "./muscle.h"





Muscle::Muscle(
        const std::string &name,
        float attach_mass,
        glm::vec3 attach_scale,
        Item& item_a,
        glm::vec3 pos_in_a,
        Item& item_b,
        glm::vec3 pos_in_b
        ) : item_a_pos(item_a){

    attach_a = Item(name + "_attach_a", std::make_shared<ObjShape>("./resources/obj/cube.obj"), pos_in_a, attach_scale, attach_mass);
    attach_b = Item(name + "_attach_b", std::make_shared<ObjShape>("./resources/obj/cube.obj"), pos_in_b, attach_scale, attach_mass);

    btTransform frame_in_a;
    frame_in_a.setIdentity();
    frame_in_a.setOrigin(btVector3(attach_scale.x, 0, 0));

    btTransform frame_in_b;
    frame_in_b.setIdentity();
    frame_in_b.setOrigin(btVector3(-attach_scale.x, 0, 0));

    muscle_slider_constraint = new btSliderConstraint(*attach_a.get_body(), *attach_b.get_body(), frame_in_a, frame_in_b,
                                                      true);

    attach_a_constraint = new btPoint2PointConstraint(
            *item_a.get_body(),
            *attach_a.get_body(),
            btVector3(pos_in_a.x, pos_in_a.y, pos_in_a.z),
            btVector3(-attach_scale.x, 0, 0)
            );

    attach_b_constraint = new btPoint2PointConstraint(
            *item_b.get_body(),
            *attach_b.get_body(),
            btVector3(pos_in_b.x, pos_in_b.y, pos_in_b.z),
            btVector3(attach_scale.x, 0, 0)
    );

}

void Muscle::contract(float force) {

}

void Muscle::release() {

}

std::vector<Item> Muscle::get_items() {
    return {attach_a, attach_b};
}

std::vector<btTypedConstraint *> Muscle::get_constraints() {
    return {muscle_slider_constraint, attach_a_constraint, attach_b_constraint};
}
