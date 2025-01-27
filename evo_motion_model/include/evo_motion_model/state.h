//
// Created by samuel on 20/01/25.
//

#ifndef EVO_MOTION_STATE_H
#define EVO_MOTION_STATE_H

#include <torch/torch.h>

class State {
public:
    virtual int get_size() = 0;

    virtual torch::Tensor get_state(torch::Device device) = 0;

    virtual ~State() = default;
};

#endif//EVO_MOTION_STATE_H
