//
// Created by samuel on 14/02/20.
//

#include "hunt.h"

HuntingEnv::HuntingEnv(int seed)
: Environment(renderer(1920, 1080), get_items()) {

}

c10::IntArrayRef HuntingEnv::action_space() {
    return torch::IntArrayRef({1 /* direction angle */ +
                               1 /* vision angle */ +
                               1 /* velocity */
                              });
}

c10::IntArrayRef HuntingEnv::state_space() {
    return torch::IntArrayRef({3 /* pos */ +
                               3 /* vel */ +
                               3 /* acc */ +
                               1 /* is prey visible */ +
                               1 /* current wheel angle */ +
                               1 /* current vision angle */ +
                               1 /* angle diff vision */
                              });
}

bool HuntingEnv::is_action_discrete() {
    return false;
}

void HuntingEnv::act(torch::Tensor action) {

}

env_step HuntingEnv::compute_new_state() {
    return env_step{
        torch::zeros(state_space()),
        0.f,
        false
    };
}

env_step HuntingEnv::reset_engine() {
    return compute_new_state();
}

std::vector<item> HuntingEnv::get_items() {
    return std::vector<item>();
}
