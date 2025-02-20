//
// Created by samuel on 05/04/24.
// You need to cite the author by referencing : https://github.com/Ipsedo
//

#ifndef EVO_MOTION_PROPRIOCEPTION_STATE_H
#define EVO_MOTION_PROPRIOCEPTION_STATE_H

#include <torch/torch.h>

#include <evo_motion_model/item.h>
#include <evo_motion_model/robot/muscle.h>
#include <evo_motion_model/state.h>

class ItemProprioceptionState : public State, public btCollisionWorld::ContactResultCallback {
public:
    ItemProprioceptionState(
        const std::shared_ptr<RigidBodyItem> &item, const std::shared_ptr<RigidBodyItem> &floor,
        btDynamicsWorld *world);

    int get_size() override;

    torch::Tensor get_state(const torch::Device &device) override;

    btScalar addSingleResult(
        btManifoldPoint &cp, const btCollisionObjectWrapper *colObj0Wrap, int partId0, int index0,
        const btCollisionObjectWrapper *colObj1Wrap, int partId1, int index1) override;

protected:
    std::shared_ptr<RigidBodyItem> state_item;
    btVector3 last_ang_vel;
    btVector3 last_lin_vel;

private:
    bool floor_touched;

    torch::Tensor get_point_state(glm::vec3 point) const;
};

class MemberState final : public ItemProprioceptionState {
public:
    MemberState(
        const std::shared_ptr<RigidBodyItem> &item, const std::shared_ptr<RigidBodyItem> &root_item,
        const std::shared_ptr<RigidBodyItem> &floor, btDynamicsWorld *world);

    int get_size() override;

    torch::Tensor get_state(const torch::Device &device) override;

private:
    std::shared_ptr<RigidBodyItem> root_item;
};

class RootMemberState final : public ItemProprioceptionState {
public:
    RootMemberState(
        const std::shared_ptr<RigidBodyItem> &item, const std::shared_ptr<RigidBodyItem> &floor,
        btDynamicsWorld *world);

    int get_size() override;

    torch::Tensor get_state(const torch::Device &device) override;
};

// Muscle

class MuscleState final : public State {
public:
    explicit MuscleState(const std::shared_ptr<Muscle> &muscle);

    int get_size() override;

    torch::Tensor get_state(const torch::Device &device) override;

private:
    btSliderConstraint *slider_constraint;
    btPoint2PointConstraint *p2p_a;
    btPoint2PointConstraint *p2p_b;
};

#endif//EVO_MOTION_PROPRIOCEPTION_STATE_H