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

    bool will_draw;

    std::function<void(torch::Tensor, std::vector<item>)> m_apply_action;
    std::function<env_step(std::vector<item>)> m_get_state;

    environment(renderer renderer, std::vector<item> items, bool will_draw,
            std::function<void(torch::Tensor, std::vector<item>)> apply_action,
            std::function<env_step(std::vector<item>)> process_env);

    env_step step(float delta, torch::Tensor action);
};

#endif //EVOMOTION_ENVIRONMENT_H
