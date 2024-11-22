//
// Created by samuel on 19/09/24.
//

#include <memory>
#include <utility>

#include <evo_motion_networks/agent.h>
#include <evo_motion_networks/agent_factory.h>
#include <evo_motion_networks/agents/actor_critic.h>
#include <evo_motion_networks/agents/actor_critic_liquid.h>
#include <evo_motion_networks/agents/debug_agents.h>
#include <evo_motion_networks/agents/ppo_gae.h>
#include <evo_motion_networks/agents/ppo_vanilla.h>
#include <evo_motion_networks/agents/soft_actor_critic.h>
#include <evo_motion_networks/agents/soft_actor_critic_liquid.h>

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
    return std::make_shared<ActorCriticAgent>(
        get_value<int>("seed"), state_space, action_space, get_value<int>("hidden_size"),
        get_value<int>("batch_size"), get_value<float>("lr"), get_value<float>("gamma"),
        get_value<float>("entropy_start_factor"), get_value<float>("entropy_end_factor"),
        get_value<long>("entropy_steps"), get_value<int>("replay_buffer_size"),
        get_value<int>("train_every"));
}

ActorCriticFactory::ActorCriticFactory(const std::map<std::string, std::string> &parameters)
    : AgentFactory(parameters) {}

std::shared_ptr<Agent> ActorCriticLiquidFactory::create_agent(
    const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space) {
    return std::make_shared<ActorCriticLiquidAgent>(
        get_value<int>("seed"), state_space, action_space, get_value<int>("neuron_number"),
        get_value<int>("batch_size"), get_value<float>("lr"), get_value<float>("gamma"),
        get_value<float>("entropy_start_factor"), get_value<float>("entropy_end_factor"),
        get_value<long>("entropy_steps"), get_value<int>("unfolding_steps"),
        get_value<int>("replay_buffer_size"), get_value<int>("train_every"));
}

ActorCriticLiquidFactory::ActorCriticLiquidFactory(
    const std::map<std::string, std::string> &parameters)
    : AgentFactory(parameters) {}

SofActorCriticFactory::SofActorCriticFactory(const std::map<std::string, std::string> &parameters)
    : AgentFactory(parameters) {}

std::shared_ptr<Agent> SofActorCriticFactory::create_agent(
    const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space) {
    return std::make_shared<SoftActorCriticAgent>(
        get_value<int>("seed"), state_space, action_space, get_value<int>("hidden_size"),
        get_value<int>("batch_size"), get_value<float>("learning_rate"), get_value<float>("gamma"),
        get_value<float>("tau"), get_value<int>("replay_buffer_size"),
        get_value<int>("train_every"));
}

SofActorCriticLiquidFactory::SofActorCriticLiquidFactory(
    const std::map<std::string, std::string> &parameters)
    : AgentFactory(parameters) {}

std::shared_ptr<Agent> SofActorCriticLiquidFactory::create_agent(
    const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space) {
    return std::make_shared<SoftActorCriticLiquidAgent>(
        get_value<int>("seed"), state_space, action_space, get_value<int>("neuron_number"),
        get_value<int>("batch_size"), get_value<float>("learning_rate"), get_value<float>("gamma"),
        get_value<float>("tau"), get_value<int>("unfolding_steps"),
        get_value<int>("replay_buffer_size"), get_value<int>("train_every"));
}

PpoGaeFactory::PpoGaeFactory(const std::map<std::string, std::string> &parameters)
    : AgentFactory(parameters) {}

std::shared_ptr<Agent> PpoGaeFactory::create_agent(
    const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space) {
    return std::make_shared<PpoGaeAgent>(
        get_value<int>("seed"), state_space, action_space, get_value<int>("hidden_size"),
        get_value<float>("gamma"), get_value<float>("lambda"), get_value<float>("epsilon"),
        get_value<float>("entropy_factor"), get_value<float>("critic_loss_factor"),
        get_value<int>("epoch"), get_value<int>("batch_size"), get_value<float>("learning_rate"));
}

PpoVanillaFactory::PpoVanillaFactory(const std::map<std::string, std::string> &parameters)
    : AgentFactory(parameters) {}

std::shared_ptr<Agent> PpoVanillaFactory::create_agent(
    const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space) {
    return std::make_shared<PpoVanillaAgent>(
        get_value<int>("seed"), state_space, action_space, get_value<int>("hidden_size"),
        get_value<float>("gamma"), get_value<float>("epsilon"), get_value<float>("entropy_factor"),
        get_value<float>("critic_loss_factor"), get_value<int>("epoch"),
        get_value<int>("batch_size"), get_value<float>("learning_rate"),
        get_value<int>("replay_buffer_size"), get_value<int>("train_every"));
}

// Build factory

std::map<
    std::string, std::function<std::shared_ptr<AgentFactory>(std::map<std::string, std::string>)>>
    AGENT_FACTORY_CONSTRUCTORS = {
        {"random", std::make_shared<RandomAgentFactory, std::map<std::string, std::string>>},
        {"constant", std::make_shared<ConstantAgentFactory, std::map<std::string, std::string>>},
        {"actor_critic", std::make_shared<ActorCriticFactory, std::map<std::string, std::string>>},
        {"actor_critic_liquid",
         std::make_shared<ActorCriticLiquidFactory, std::map<std::string, std::string>>},
        {"soft_actor_critic",
         std::make_shared<SofActorCriticFactory, std::map<std::string, std::string>>},
        {"soft_actor_critic_liquid",
         std::make_shared<SofActorCriticLiquidFactory, std::map<std::string, std::string>>},
        {"ppo", std::make_shared<PpoGaeFactory, std::map<std::string, std::string>>},
        {"ppo_vanilla", std::make_shared<PpoVanillaFactory, std::map<std::string, std::string>>}};

std::shared_ptr<AgentFactory>
get_agent_factory(const std::string &agent_name, std::map<std::string, std::string> parameters) {
    if (AGENT_FACTORY_CONSTRUCTORS.find(agent_name) == AGENT_FACTORY_CONSTRUCTORS.end())
        throw std::invalid_argument(agent_name);
    return AGENT_FACTORY_CONSTRUCTORS[agent_name](std::move(parameters));
}
