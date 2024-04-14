//
// Created by samuel on 14/04/24.
//

#include <evo_motion_networks/agent_builder.h>

#include "./actor_critic.h"
#include "./actor_critic_liquid.h"
#include "./debug_agents.h"

AgentBuilder::AgentBuilder(
    const std::string &name, int seed, const std::vector<int64_t> &state_space,
    const std::vector<int64_t> &action_space, int hidden_size, float lr)
    : name(name), seed(seed), state_space(state_space), action_space(action_space),
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

          }) {}

std::shared_ptr<Agent> AgentBuilder::get() {
    return agent_constructors[name](seed, state_space, action_space, hidden_size, lr);
}
