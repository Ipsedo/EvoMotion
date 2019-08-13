//
// Created by samuel on 11/08/19.
//

#ifndef EVOMOTION_ENVIRONMENT_H
#define EVOMOTION_ENVIRONMENT_H

#include <btBulletDynamicsCommon.h>
#include <torch/torch.h>
#include "../view/obj_mtl_vbo.h"
#include "../view/renderer.h"
#include "../model/engine.h"

struct env_step {
    torch::Tensor state;
    float reward;
    bool done;
};

struct environment {

    engine m_engine;
    renderer m_renderer;
    std::vector<item> m_items;

    int m_step;

    std::function<void(torch::Tensor, std::vector<item>)> m_act_fun;
    std::function<env_step(std::vector<item>)> m_step_fun;
    std::function<void(std::vector<item>)> m_reset_fun;

    torch::IntArrayRef m_actions_sizes;
    torch::IntArrayRef m_state_sizes;

    environment(renderer renderer, std::vector<item> items,
            torch::IntArrayRef action_sizes, torch::IntArrayRef state_sizes,
            std::function<void(torch::Tensor, std::vector<item>)> act_fun,
            std::function<env_step(std::vector<item>)> step_fun,
            std::function<void(std::vector<item>)> reset_fun);

    env_step step(float delta, torch::Tensor action, bool will_draw);
    void reset();

    // TODO quit : delete ObjMtlVBO pointers
};

#endif //EVOMOTION_ENVIRONMENT_H
