//
// Created by samuel on 14/02/20.
//

#ifndef EVOMOTION_HUNTINGENV_H
#define EVOMOTION_HUNTINGENV_H


#include <random>
#include "environment.h"

class HuntingEnv : public Environment {
public:
    c10::IntArrayRef action_space() override;

    c10::IntArrayRef state_space() override;

    bool is_action_discrete() override;

    explicit HuntingEnv(long seed);

protected:
    void act(torch::Tensor action) override;

    env_step compute_new_state() override;

    env_step reset_engine() override;

private:

    /*
	 * Random stuff
	 */

    std::mt19937 rd_gen;
    std::uniform_real_distribution<float> rd_uni;

    /*
     * Env init stuff
     */

    std::vector<item> create_items();
};


#endif //EVOMOTION_HUNTINGENV_H
