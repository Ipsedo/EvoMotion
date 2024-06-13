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

class ItemState final : public State, public btCollisionWorld::ContactResultCallback {
public:
    ItemState(Item item, const std::optional<Item> &root_item, const Item &floor, btDynamicsWorld *world);

    ItemState(const Item &item, const Item &floor, btDynamicsWorld *world);

    int get_size() override;

    torch::Tensor get_state() override;

    btScalar addSingleResult(
        btManifoldPoint &cp, const btCollisionObjectWrapper *colObj0Wrap, int partId0, int index0,
        const btCollisionObjectWrapper *colObj1Wrap, int partId1, int index1) override;

    ~ItemState() override;

private:
    std::optional<Item> root_item;
    Item state_item;
    bool floor_touched;
protected:
    torch::Tensor get_point_state(glm::vec3 point) const;
};

#endif//EVO_MOTION_STATE_H
