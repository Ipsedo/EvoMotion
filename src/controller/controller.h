//
// Created by samuel on 18/12/22.
//

#ifndef EVO_MOTION_CONTROLLER_H
#define EVO_MOTION_CONTROLLER_H

#include <torch/torch.h>

class Controller {
public:
    virtual void on_input(torch::Tensor action) = 0;
};

#endif//EVO_MOTION_CONTROLLER_H
