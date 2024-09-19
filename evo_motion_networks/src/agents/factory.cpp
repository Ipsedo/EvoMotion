//
// Created by samuel on 19/09/24.
//

#include "./factory.h"

#include <memory>
#include <utility>

#include <evo_motion_networks/agent.h>

#include "./actor_critic.h"
#include "./actor_critic_liquid.h"
#include "./debug_agents.h"

// Abstract

AgentFactory::AgentFactory(std::map<std::string, std::string> parameters)
    : parameters(std::move(parameters)) {}

std::string AgentFactory::get_value(const std::string &key) {
    if (parameters.find(key) == parameters.end()) throw std::invalid_argument(key);

    return parameters[key];
}

// Agents

std::shared_ptr<Agent> RandomAgentFactory::create_agent(
    const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space) {
    return std::make_shared<RandomAgent>(action_space);
}

RandomAgentFactory::RandomAgentFactory(const std::map<std::string, std::string> &parameters)
    : AgentFactory(parameters) {}

std::shared_ptr<Agent> ConstantAgentFactory::create_agent(
    const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space) {
    return std::make_shared<ConstantAgent>(action_space);
}

ConstantAgentFactory::ConstantAgentFactory(const std::map<std::string, std::string> &parameters)
    : AgentFactory(parameters) {}

std::shared_ptr<Agent> ActorCriticFactory::create_agent(
    const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space) {
    return std::make_shared<ActorCritic>(
        std::stoi(get_value("seed")), state_space, action_space,
        std::stoi(get_value("hidden_size")), std::stoi(get_value("batch_size")),
        std::stof(get_value("learning_rate")));
}

ActorCriticFactory::ActorCriticFactory(const std::map<std::string, std::string> &parameters)
    : AgentFactory(parameters) {}

std::shared_ptr<Agent> ActorCriticLiquidFactory::create_agent(
    const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space) {
    return std::make_shared<ActorCriticLiquid>(
        std::stoi(get_value("seed")), state_space, action_space,
        std::stoi(get_value("hidden_size")), std::stoi(get_value("batch_size")),
        std::stof(get_value("learning_rate")), std::stoi(get_value("unfolding_steps")));
}

ActorCriticLiquidFactory::ActorCriticLiquidFactory(
    const std::map<std::string, std::string> &parameters)
    : AgentFactory(parameters) {}

// Build factory

std::map<
    std::string, std::function<std::shared_ptr<AgentFactory>(std::map<std::string, std::string>)>>
    FACTORY_CONSTRUCTORS = {
        {"random", std::make_shared<RandomAgentFactory, std::map<std::string, std::string>>},
        {"constant", std::make_shared<ConstantAgentFactory, std::map<std::string, std::string>>},
        {"actor_critic", std::make_shared<ActorCriticFactory, std::map<std::string, std::string>>},
        {"actor_critic_liquid",
         std::make_shared<ActorCriticLiquidFactory, std::map<std::string, std::string>>}};

std::shared_ptr<AgentFactory>
get_factory(const std::string &agent_name, std::map<std::string, std::string> parameters) {
    return FACTORY_CONSTRUCTORS[agent_name](std::move(parameters));
}
