//
// Created by samuel on 05/04/24.
// You need to cite the author by referencing : https://github.com/Ipsedo
//

#ifndef EVO_MOTION_STATE_H
#define EVO_MOTION_STATE_H

#include "./item.h"

#include <torch/torch.h>

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

#endif //EVO_MOTION_STATE_H
