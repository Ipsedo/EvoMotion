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

class Environment {
private:
    int m_step;

protected:
    // Model
    engine m_engine;

    // View
    renderer m_renderer;

    // Items
    std::vector<item> m_items;

    /**
     * Virtual fonction applying action
     * Need to be overloaded !
     * @param action
     */
    virtual void act(torch::Tensor action) = 0;

    /**
     * Virtual function to compute next state
     * @return
     */
    virtual env_step compute_new_state() = 0;

    virtual env_step reset_engine() = 0;

public:
    Environment(renderer renderer, std::vector<item> items);
    virtual torch::IntArrayRef action_space() = 0;
    virtual torch::IntArrayRef state_space() = 0;
    const env_step reset();
    const env_step step(float delta, torch::Tensor action, bool will_draw);

    bool is_renderer_on();

    virtual ~Environment();

};

#endif //EVOMOTION_ENVIRONMENT_H
