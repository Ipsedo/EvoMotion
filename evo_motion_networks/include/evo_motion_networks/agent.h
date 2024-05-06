//
// Created by samuel on 19/12/22.
//

#ifndef EVO_MOTION_AGENT_H
#define EVO_MOTION_AGENT_H

#include <map>

#include <torch/torch.h>

class Agent {
public:
    virtual torch::Tensor act(torch::Tensor state, float reward) = 0;

    virtual void done(float reward) = 0;

    virtual void save(const std::string &output_folder_path) = 0;

    virtual void load(const std::string &input_folder_path) = 0;

    virtual std::map<std::string, float> get_metrics() = 0;

    virtual void to(torch::DeviceType device) = 0;

    virtual void set_eval(bool eval) = 0;

    virtual int count_parameters() = 0;
    virtual float grad_norm_mean() = 0;

    virtual ~Agent();
};

#endif//EVO_MOTION_AGENT_H
