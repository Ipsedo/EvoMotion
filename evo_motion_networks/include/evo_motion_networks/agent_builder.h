//
// Created by samuel on 14/04/24.
//

#ifndef EVO_MOTION_AGENT_BUILDER_H
#define EVO_MOTION_AGENT_BUILDER_H

#include <functional>
#include <map>
#include <memory>
#include <vector>

#include "./agent.h"

class AgentBuilder {
public:
    AgentBuilder(
        std::string name, int seed, const std::vector<int64_t> &state_space,
        const std::vector<int64_t> &action_space, int hidden_size, float lr);

    std::shared_ptr<Agent> get();

private:
    std::string name;
    int seed;
    std::vector<int64_t> state_space;
    std::vector<int64_t> action_space;
    int hidden_size;
    float lr;

    std::map<
        std::string, std::function<std::shared_ptr<Agent>(
            int, std::vector<int64_t>, std::vector<int64_t>, int, float)> >
    agent_constructors;
};

#endif//EVO_MOTION_AGENT_BUILDER_H