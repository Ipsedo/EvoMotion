//
// Created by samuel on 30/12/23.
//

#ifndef EVO_MOTION_MUSCLE_H
#define EVO_MOTION_MUSCLE_H

#include <btBulletDynamicsCommon.h>

#include <evo_motion_model/item.h>

#include "./skeleton.h"

class Muscle {
public:
    Muscle(
        const std::string &name, float attach_mass, glm::vec3 attach_scale, Item &item_a,
        glm::vec3 pos_in_a, Item &item_b, glm::vec3 pos_in_b, float force, float max_speed);

    void contract(float speed_factor) const;

    void release() const;

    std::vector<Item> get_items();

    std::vector<btTypedConstraint *> get_constraints();

    btSliderConstraint *get_slider_constraint();

    std::tuple<btPoint2PointConstraint *, btPoint2PointConstraint *> get_p2p_constraints();

    ~Muscle();

private:
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

class JsonMuscularSystem final : public AbstractMuscularSystem {
public:
    JsonMuscularSystem(Skeleton skeleton, const std::string &json_path);

    std::vector<Muscle> get_muscles() override;

private:
    std::vector<Muscle> muscles;
};

#endif//EVO_MOTION_MUSCLE_H