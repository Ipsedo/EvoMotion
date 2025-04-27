//
// Created by samuel on 19/09/24.
//

#ifndef EVO_MOTION_AGENT_FACTORY_H
#define EVO_MOTION_AGENT_FACTORY_H

#include <evo_motion_networks/agent.h>

// Agent abstract factory

class AgentFactory {
public:
    virtual ~AgentFactory() = default;

    explicit AgentFactory(std::map<std::string, std::string> parameters);
    virtual std::shared_ptr<Agent> create_agent(
        const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space) = 0;

protected:
    template<typename Value>
    Value get_value(const std::string &key);

    template<typename Value>
    Value generic_get_value(
        const std::function<Value(const std::string &)> &converter, const std::string &key);

private:
    std::map<std::string, std::string> parameters;
};

std::shared_ptr<AgentFactory>
get_agent_factory(const std::string &agent_name, std::map<std::string, std::string> parameters);

// Factories

class RandomAgentFactory final : public AgentFactory {
public:
    explicit RandomAgentFactory(const std::map<std::string, std::string> &parameters);

    std::shared_ptr<Agent> create_agent(
        const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space) override;
};

class ConstantAgentFactory final : public AgentFactory {
public:
    explicit ConstantAgentFactory(const std::map<std::string, std::string> &parameters);

    std::shared_ptr<Agent> create_agent(
        const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space) override;
};

class ActorCriticFactory final : public AgentFactory {
public:
    explicit ActorCriticFactory(const std::map<std::string, std::string> &parameters);

    std::shared_ptr<Agent> create_agent(
        const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space) override;
};

class ActorCriticLiquidFactory final : public AgentFactory {
public:
    explicit ActorCriticLiquidFactory(const std::map<std::string, std::string> &parameters);

    std::shared_ptr<Agent> create_agent(
        const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space) override;
};

class LinearSoftActorCriticFactory final : public AgentFactory {
public:
    explicit LinearSoftActorCriticFactory(const std::map<std::string, std::string> &parameters);
    std::shared_ptr<Agent> create_agent(
        const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space) override;
};

class KanSoftActorCriticFactory final : public AgentFactory {
public:
    explicit KanSoftActorCriticFactory(const std::map<std::string, std::string> &parameters);
    std::shared_ptr<Agent> create_agent(
        const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space) override;
};

class SoftActorCriticLiquidFactory final : public AgentFactory {
public:
    explicit SoftActorCriticLiquidFactory(const std::map<std::string, std::string> &parameters);
    std::shared_ptr<Agent> create_agent(
        const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space) override;
};

class PpoGaeFactory final : public AgentFactory {
public:
    explicit PpoGaeFactory(const std::map<std::string, std::string> &parameters);
    std::shared_ptr<Agent> create_agent(
        const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space) override;
};

class PpoVanillaFactory final : public AgentFactory {
public:
    explicit PpoVanillaFactory(const std::map<std::string, std::string> &parameters);
    std::shared_ptr<Agent> create_agent(
        const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space) override;
};

class PpoGaeLiquidFactory final : public AgentFactory {
public:
    explicit PpoGaeLiquidFactory(const std::map<std::string, std::string> &parameters);
    std::shared_ptr<Agent> create_agent(
        const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space) override;
};

class CrossQFactory final : public AgentFactory {
public:
    explicit CrossQFactory(const std::map<std::string, std::string> &parameters);
    std::shared_ptr<Agent> create_agent(
        const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space) override;
};

#endif//EVO_MOTION_AGENT_FACTORY_H
