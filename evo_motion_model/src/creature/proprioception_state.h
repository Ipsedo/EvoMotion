//
// Created by samuel on 05/04/24.
// You need to cite the author by referencing : https://github.com/Ipsedo
//

#ifndef EVO_MOTION_PROPRIOCEPTION_STATE_H
#define EVO_MOTION_PROPRIOCEPTION_STATE_H

#include <torch/torch.h>

#include <evo_motion_model/item.h>
#include <evo_motion_model/muscle.h>
#include <evo_motion_model/state.h>

class ItemProprioceptionState : public State, public btCollisionWorld::ContactResultCallback {
public:
    ItemProprioceptionState(const Item &item, const Item &floor, btDynamicsWorld *world);

    int get_size() override;

    torch::Tensor get_state(torch::Device device) override;

    btScalar addSingleResult(
        btManifoldPoint &cp, const btCollisionObjectWrapper *colObj0Wrap, int partId0, int index0,
        const btCollisionObjectWrapper *colObj1Wrap, int partId1, int index1) override;

protected:
    Item state_item;
    btVector3 last_ang_vel;
    btVector3 last_lin_vel;

private:
    bool floor_touched;

    torch::Tensor get_point_state(glm::vec3 point) const;
};

class MemberState : public ItemProprioceptionState {
public:
    MemberState(const Item &item, const Item &root_item, const Item &floor, btDynamicsWorld *world);

    int get_size() override;

    torch::Tensor get_state(torch::Device device) override;

private:
    Item root_item;
};

class RootMemberState : public ItemProprioceptionState {
public:
    RootMemberState(const Item &item, const Item &floor, btDynamicsWorld *world);

    int get_size() override;

    torch::Tensor get_state(torch::Device device) override;
};

// Muscle

class MuscleState : public State {
public:
    explicit MuscleState(std::shared_ptr<Muscle> muscle);

    int get_size() override;

    torch::Tensor get_state(torch::Device device) override;

private:
    btSliderConstraint *slider_constraint;
    btPoint2PointConstraint *p2p_a;
    btPoint2PointConstraint *p2p_b;
};

#endif//EVO_MOTION_PROPRIOCEPTION_STATE_H