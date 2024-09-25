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

template<typename Value>
Value AgentFactory::generic_get_value(
    std::function<Value(const std::string &)> converter, const std::string &key) {
    if (parameters.find(key) == parameters.end()) throw std::invalid_argument(key);
    return converter(parameters[key]);
}

template<>
int AgentFactory::get_value(const std::string &key) {
    return generic_get_value<int>([](const std::string &s) { return std::stoi(s); }, key);
}

template<>
long AgentFactory::get_value(const std::string &key) {
    return generic_get_value<long>([](const std::string &s) { return std::stol(s); }, key);
}

template<>
float AgentFactory::get_value(const std::string &key) {
    return generic_get_value<float>([](const std::string &s) { return std::stof(s); }, key);
}

template<>
bool AgentFactory::get_value(const std::string &key) {
    return generic_get_value<bool>(
        [](const std::string &v) {
            if (v == "true") return true;
            else if (v == "false") return false;

            throw std::invalid_argument(v);
        },
        key);
}

template<>
std::string AgentFactory::get_value(const std::string &key) {
    return generic_get_value<std::string>([](const std::string &s) { return s; }, key);
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
        get_value<int>("seed"), state_space, action_space, get_value<int>("hidden_size"),
        get_value<int>("batch_size"), get_value<float>("learning_rate"), get_value<float>("gamma"),
        get_value<float>("first_entropy_factor"), get_value<float>("wanted_entropy_factor"),
        get_value<long>("entropy_factor_steps"));
}

ActorCriticFactory::ActorCriticFactory(const std::map<std::string, std::string> &parameters)
    : AgentFactory(parameters) {}

std::shared_ptr<Agent> ActorCriticLiquidFactory::create_agent(
    const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space) {
    return std::make_shared<ActorCriticLiquid>(
        get_value<int>("seed"), state_space, action_space, get_value<int>("hidden_size"),
        get_value<int>("batch_size"), get_value<float>("learning_rate"), get_value<float>("gamma"),
        get_value<float>("first_entropy_factor"), get_value<float>("wanted_entropy_factor"),
        get_value<long>("entropy_factor_steps"), get_value<int>("unfolding_steps"));
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
