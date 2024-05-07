//
// Created by samuel on 14/04/24.
//

#include <evo_motion_networks/agent_builder.h>

#include <utility>

#include "./actor_critic.h"
#include "./actor_critic_liquid.h"
#include "./debug_agents.h"

AgentBuilder::AgentBuilder(
    std::string name, const int seed, const std::vector<int64_t> &state_space,
    const std::vector<int64_t> &action_space, const int hidden_size, const float lr)
    : name(std::move(name)), seed(seed), state_space(state_space), action_space(action_space),
      hidden_size(hidden_size), lr(lr),
      agent_constructors(
          {{"actor_critic",
            std::make_shared<
                ActorCritic, int, std::vector<int64_t>, std::vector<int64_t>, int, float>},
           {"actor_critic_liquid",
            std::make_shared<
                ActorCriticLiquid, int, std::vector<int64_t>, std::vector<int64_t>, int, float>},
           {"random",
            std::make_shared<
                RandomAgent, int, std::vector<int64_t>, std::vector<int64_t>, int, float>},
           {"constant",
            std::make_shared<
                ConstantAgent, int, std::vector<int64_t>, std::vector<int64_t>, int, float>}

          }) {
    if (agent_constructors.find(this->name) == agent_constructors.end()) {
        std::vector<std::string> agent_keys;
        for (const auto &[k, v]: agent_constructors) agent_keys.push_back(k);

        std::cerr << "Unrecognized agent \"" << this->name << "\"" << std::endl;
        std::cerr << "Supported agents are : " << agent_keys << std::endl;
        std::exit(1);
    }
}

std::shared_ptr<Agent> AgentBuilder::get() {
    return agent_constructors[name](seed, state_space, action_space, hidden_size, lr);
}
