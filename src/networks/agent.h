//
// Created by samuel on 19/12/22.
//

#ifndef EVO_MOTION_AGENT_H
#define EVO_MOTION_AGENT_H

#include <map>

#include <torch/torch.h>

#include "../model/environment.h"

class Agent {
public:
    virtual torch::Tensor act(step step) = 0;

    virtual void done(step step) = 0;

    virtual void save(const std::string &output_folder_path) = 0;

    virtual void load(const std::string &input_folder_path) = 0;

    virtual std::map<std::string, float> get_metrics() = 0;

    virtual void to(torch::DeviceType device) = 0;

    virtual void set_eval(bool eval) = 0;
};

#endif//EVO_MOTION_AGENT_H
