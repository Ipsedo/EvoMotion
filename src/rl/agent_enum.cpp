//
// Created by samuel on 16/02/20.
//

#include "agent_enum.h"
#include "deep_q_learning.h"
#include "ddpg.h"

// Agent Enum

AgentEnum::AgentEnum(std::string name,
                     std::function<agent *(int, torch::IntArrayRef, torch::IntArrayRef)> get_env_fun)
        : name(std::move(name)), get_env_fun(std::move(get_env_fun)) {}

std::string AgentEnum::get_name() const {
    return name;
}

agent *AgentEnum::get_agent(int hidden_size, torch::IntArrayRef state_space, torch::IntArrayRef action_space) const {
    return get_env_fun(hidden_size, state_space, action_space);
}

AgentEnum AgentEnum::from_str(std::string agent_value) {
    std::vector<AgentEnum> agent_list = AgentEnum::get_values();

    for (auto a: agent_list)
        if (a.get_name() == agent_value)
            return a;

    return AgentEnum::DQN;
}

std::vector<AgentEnum> AgentEnum::get_values() {
    return {AgentEnum::DDPG, AgentEnum::DQN, AgentEnum::RANDOM};
}

std::vector<std::string> AgentEnum::get_names() {
    auto agents = AgentEnum::get_values();
    std::vector<std::string> agent_names(agents.size());

    std::transform(agents.begin(), agents.end(), agent_names.begin(), [](AgentEnum a) { return a.get_name(); });

    return agent_names;
}

const AgentEnum AgentEnum::DQN =
        AgentEnum("DQN", [](int hidden_size, torch::IntArrayRef state_space, torch::IntArrayRef action_space) {
            return new dqn_agent(static_cast<int>(time(nullptr)), state_space, action_space, hidden_size);
        });


const AgentEnum AgentEnum::DDPG =
        AgentEnum("DDPG", [](int hidden_size, torch::IntArrayRef state_space, torch::IntArrayRef action_space) {
            return new ddpg(static_cast<int>(time(nullptr)), state_space, action_space, hidden_size);
        });

const AgentEnum AgentEnum::RANDOM =
        AgentEnum("RANDOM", [](int hidden_size, torch::IntArrayRef state_space, torch::IntArrayRef action_space) {
            return new random_agent(state_space, action_space);
        });
