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
};

class ItemState : public State {
public:
    explicit ItemState(const Item &item);

    int get_size() override;

    torch::Tensor get_state() override;

private:
    Item item;
    glm::vec3 scale;
    glm::vec3 last_lin_velocity;
    glm::vec3 last_ang_velocity;
};

class SkeletonItemState : public State {
public:
    SkeletonItemState(const Item &root, const Item &item);
    int get_size() override;
    torch::Tensor get_state() override;

private:
    Item root;
    Item item;
    glm::vec3 scale;
    glm::vec3 last_lin_velocity;
    glm::vec3 last_ang_velocity;
};

#endif//EVO_MOTION_STATE_H
