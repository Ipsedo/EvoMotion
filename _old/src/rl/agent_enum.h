//
// Created by samuel on 16/02/20.
//

#ifndef EVOMOTION_AGENT_ENUM_H
#define EVOMOTION_AGENT_ENUM_H

#include <string>
#include "agent.h"

class AgentEnum {
public:
    static const AgentEnum DQN;
    static const AgentEnum DDPG;
    static const AgentEnum RANDOM;


private:
    const std::string name;
    const std::function<agent *(int, torch::IntArrayRef, torch::IntArrayRef)> get_env_fun;
    AgentEnum(std::string name, std::function<agent *(int, torch::IntArrayRef, torch::IntArrayRef)> get_env_fun);

public:

    std::string get_name() const ;
    agent *get_agent(int hidden_size, torch::IntArrayRef state_space, torch::IntArrayRef action_space) const ;

    static AgentEnum from_str(std::string agent_value);

    static std::vector<AgentEnum> get_values();
    static std::vector<std::string> get_names();

};

#endif //EVOMOTION_AGENT_ENUM_H
