//
// Created by samuel on 05/04/24.
// You need to cite the author by referencing : https://github.com/Ipsedo
//

#ifndef EVO_MOTION_STATE_H
#define EVO_MOTION_STATE_H

#include <torch/torch.h>

#include <evo_motion_model/item.h>

class State {
public:
    virtual int get_size() = 0;

    virtual torch::Tensor get_state() = 0;

    virtual ~State();
};

class ItemState : public State, public btCollisionWorld::ContactResultCallback {
public:
    ItemState(Item item, const Item &floor, btDynamicsWorld *world);

    int get_size() override;

    torch::Tensor get_state() override;

    btScalar addSingleResult(
        btManifoldPoint &cp, const btCollisionObjectWrapper *colObj0Wrap, int partId0, int index0,
        const btCollisionObjectWrapper *colObj1Wrap, int partId1, int index1) override;

    ~ItemState() override;

private:
    Item state_item;
    btVector3 last_lin_velocity;
    btVector3 last_ang_velocity;
    bool floor_touched;
};

#endif//EVO_MOTION_STATE_H
