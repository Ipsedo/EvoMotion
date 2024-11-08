//
// Created by samuel on 19/12/22.
//

#ifndef EVO_MOTION_AGENT_H
#define EVO_MOTION_AGENT_H

#include <map>

#include <torch/torch.h>

#include "./metrics.h"

// Agent abstract class

class Agent {
public:
    virtual torch::Tensor act(torch::Tensor state, float reward) = 0;

    virtual void done(float reward) = 0;

    virtual void save(const std::string &output_folder_path) = 0;

    virtual void load(const std::string &input_folder_path) = 0;

    virtual std::vector<LossMeter> get_metrics() = 0;

    virtual void to(torch::DeviceType device) = 0;

    virtual void set_eval(bool eval) = 0;

    virtual int count_parameters() = 0;

    virtual ~Agent() = default;
};

// Agent abstract factory

class AgentFactory {
public:
    explicit AgentFactory(std::map<std::string, std::string> parameters);
    virtual std::shared_ptr<Agent> create_agent(
        const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space) = 0;

protected:
    template<typename Value>
    Value get_value(const std::string &key);

    template<typename Value>
    Value
    generic_get_value(std::function<Value(const std::string &)> converter, const std::string &key);

private:
    std::map<std::string, std::string> parameters;
};

std::shared_ptr<AgentFactory>
get_factory(const std::string &agent_name, std::map<std::string, std::string> parameters);

#endif//EVO_MOTION_AGENT_H