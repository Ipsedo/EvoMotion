//
// Created by samuel on 18/12/22.
//

#include <evo_motion_model/environment.h>
#include <torch/torch.h>

#include "./constants.h"

Environment::Environment()
    : curr_device(torch::kCPU), m_collision_configuration(new btDefaultCollisionConfiguration()),
      m_dispatcher(new btCollisionDispatcher(m_collision_configuration)),
      m_broad_phase(new btDbvtBroadphase()),
      m_constraint_solver(new btSequentialImpulseConstraintSolver()),
      m_world(new btDiscreteDynamicsWorld(
          m_dispatcher, m_broad_phase, m_constraint_solver, m_collision_configuration)) {

    m_world->setGravity(btVector3(0, -9.8f, 0));
}

void Environment::add_item(Item item) { m_world->addRigidBody(item.get_body()); }

step Environment::do_step(const torch::Tensor &action) {
    for (const auto &c: get_controllers()) c->on_input(action);

    m_world->stepSimulation(DELTA_T_MODEL);

    return compute_step();
}

step Environment::reset() {
    reset_engine();
    return compute_step();
}

void Environment::to(torch::DeviceType device) { curr_device = device; }
