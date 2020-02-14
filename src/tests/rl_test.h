//
// Created by samuel on 12/08/19.
//

#ifndef EVOMOTION_RL_TEST_H
#define EVOMOTION_RL_TEST_H

#include <string>
#include "../core/environment.h"
#include "../rl/agent.h"

struct rl_train_info {
    std::string agent_name;
    std::string env_name;

    int nb_episode;
    int max_episode_step;
    int max_consecutive_success;
    float eps;
    float eps_decay;
    float eps_min;

    bool view;

    std::string out_model_folder;
};

struct rl_test_info {
    std::string agent_name;
    std::string env_name;

    int nb_episode;
    bool view;

    std::string in_model_folder;
};

agent* get_agent(std::string agent_name);

void train_reinforcement_learning(rl_train_info);

void test_reinforcement_learning(rl_test_info);

#endif //EVOMOTION_RL_TEST_H
