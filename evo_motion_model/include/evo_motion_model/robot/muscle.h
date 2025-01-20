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
        const std::string &name, float attach_mass, glm::vec3 attach_scale, const Item& item_a,
        glm::vec3 pos_in_a, const Item& item_b, glm::vec3 pos_in_b, float force, float max_speed);

    Muscle(
        const std::shared_ptr<AbstractDeserializer> &deserializer,
        const std::function<std::shared_ptr<Member>(std::string)>& get_member_function);

    void contract(float speed_factor) const;

    void release() const;

    std::vector<Item> get_items();

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

    Item attach_a;
    Item attach_b;

    btSliderConstraint *muscle_slider_constraint;
    btPoint2PointConstraint *attach_a_constraint;
    btPoint2PointConstraint *attach_b_constraint;
};

class AbstractMuscularSystem {
public:
    virtual std::vector<Muscle> get_muscles() = 0;

    virtual ~AbstractMuscularSystem();
};

#endif//EVO_MOTION_MUSCLE_H