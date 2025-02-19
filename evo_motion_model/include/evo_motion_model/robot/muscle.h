//
// Created by samuel on 30/12/23.
//

#ifndef EVO_MOTION_MUSCLE_H
#define EVO_MOTION_MUSCLE_H

#include <functional>

#include <btBulletDynamicsCommon.h>

#include <evo_motion_model/item.h>

#include "../serializer.h"
#include "./member.h"

class Muscle {
public:
    Muscle(
        const std::string &name, const float attach_mass, const glm::vec3 &attach_scale,
        const std::shared_ptr<RigidBodyItem> &item_a, const glm::vec3 &pos_in_a,
        const std::shared_ptr<RigidBodyItem> &item_b, const glm::vec3 &pos_in_b, const float force,
        const float max_speed);

    Muscle(
        const std::shared_ptr<AbstractDeserializer> &deserializer,
        const std::function<std::shared_ptr<Member>(std::string)> &get_member_function);

    void contract(float speed_factor) const;

    void release() const;

    std::string get_name();

    std::vector<std::shared_ptr<ShapeItem>> get_items();

    std::vector<btRigidBody *> get_bodies() const;
    std::vector<btTypedConstraint *> get_constraints();

    btSliderConstraint *get_slider_constraint() const;
    std::tuple<btPoint2PointConstraint *, btPoint2PointConstraint *> get_p2p_constraints();

    virtual std::shared_ptr<AbstractSerializer>
    serialize(const std::shared_ptr<AbstractSerializer> &serializer);

    virtual ~Muscle();

private:
    std::string name;

    std::string item_a_name;
    std::string item_b_name;

    float max_speed;

    std::shared_ptr<RigidBodyItem> attach_a;
    std::shared_ptr<RigidBodyItem> attach_b;

    btSliderConstraint *muscle_slider_constraint;
    btPoint2PointConstraint *attach_a_constraint;
    btPoint2PointConstraint *attach_b_constraint;
};

#endif//EVO_MOTION_MUSCLE_H