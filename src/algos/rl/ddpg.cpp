//
// Created by samuel on 19/08/19.
//

#include "ddpg.h"

ddpg::ddpg(torch::IntArrayRef state_space, torch::IntArrayRef action_space) :
    agent(state_space,
          action_space,
          20000) {

}

void ddpg::step(torch::Tensor state, torch::Tensor action, float reward, torch::Tensor next_state, bool done) {

}

at::Tensor ddpg::act(torch::Tensor state, float eps) {
    return at::Tensor();
}
