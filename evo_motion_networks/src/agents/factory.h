//
// Created by samuel on 19/09/24.
//

#ifndef EVO_MOTION_FACTORY_H
#define EVO_MOTION_FACTORY_H

#include <evo_motion_networks/agent.h>

class RandomAgentFactory : public AgentFactory {
public:
    explicit RandomAgentFactory(const std::map<std::string, std::string> &parameters);

    std::shared_ptr<Agent> create_agent(
        const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space) override;
};

class ConstantAgentFactory : public AgentFactory {
public:
    explicit ConstantAgentFactory(const std::map<std::string, std::string> &parameters);

    std::shared_ptr<Agent> create_agent(
        const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space) override;
};

class ActorCriticFactory : public AgentFactory {
public:
    explicit ActorCriticFactory(const std::map<std::string, std::string> &parameters);

    std::shared_ptr<Agent> create_agent(
        const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space) override;
};

class ActorCriticLiquidFactory : public AgentFactory {
public:
    explicit ActorCriticLiquidFactory(const std::map<std::string, std::string> &parameters);

    std::shared_ptr<Agent> create_agent(
        const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space) override;
};

#endif//EVO_MOTION_FACTORY_H
