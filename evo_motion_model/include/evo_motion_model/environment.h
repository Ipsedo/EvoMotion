//
// Created by samuel on 18/12/22.
//

#ifndef EVO_MOTION_ENVIRONMENT_H
#define EVO_MOTION_ENVIRONMENT_H

#include <vector>

#include <btBulletDynamicsCommon.h>
#include <torch/torch.h>

#include "./controller.h"
#include "./item.h"

struct step {
    torch::Tensor state;
    float reward;
    bool done;
};

class Environment {
protected:
    torch::DeviceType curr_device;

    btDefaultCollisionConfiguration *m_collision_configuration;
    btCollisionDispatcher *m_dispatcher;
    btBroadphaseInterface *m_broad_phase;
    btSequentialImpulseConstraintSolver *m_constraint_solver;
    btDynamicsWorld *m_world;

    virtual step compute_step() = 0;

    virtual void reset_engine() = 0;

    void add_item(const Item &item) const;

public:
    Environment();

    virtual std::vector<Item> get_items() = 0;

    virtual std::vector<std::shared_ptr<Controller>> get_controllers() = 0;

    step do_step(const torch::Tensor &action);

    step reset();

    virtual std::vector<int64_t> get_state_space() = 0;

    virtual std::vector<int64_t> get_action_space() = 0;

    [[nodiscard]] virtual bool is_continuous() const = 0;

    void to(torch::DeviceType device);

    virtual ~Environment();
};

#endif//EVO_MOTION_ENVIRONMENT_H