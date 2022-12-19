//
// Created by samuel on 19/12/22.
//

#ifndef EVO_MOTION_AGENT_H
#define EVO_MOTION_AGENT_H

#include <torch/torch.h>

#include "./model/environment.h"

class Agent {
public:
    virtual torch::Tensor act(step step) = 0;
    virtual void done(step step) = 0;
};

#endif //EVO_MOTION_AGENT_H
