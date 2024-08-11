//
// Created by samuel on 18/12/22.
//

#include <torch/torch.h>

#include <evo_motion_model/environment.h>

#include "./constants.h"

Environment::Environment()
    : curr_device(torch::kCPU), m_collision_configuration(new btDefaultCollisionConfiguration()),
      m_dispatcher(new btCollisionDispatcher(m_collision_configuration)),
      m_broad_phase(new btDbvtBroadphase()),
      m_constraint_solver(new btSequentialImpulseConstraintSolver()),
      m_world(
          new btDiscreteDynamicsWorld(
              m_dispatcher, m_broad_phase, m_constraint_solver, m_collision_configuration)) {
    m_world->setGravity(btVector3(0, -9.8f, 0));
}

void Environment::add_item(const Item &item) const { m_world->addRigidBody(item.get_body()); }

step Environment::do_step(const torch::Tensor &action) {
    for (const auto &c: get_controllers()) c->on_input(action);

    m_world->stepSimulation(DELTA_T_MODEL);

    return compute_step();
}

step Environment::reset() {
    reset_engine();
    return compute_step();
}

void Environment::to(const torch::DeviceType device) { curr_device = device; }

Environment::~Environment() {
    for (int i = m_world->getNumCollisionObjects() - 1; i >= 0; i--) {
        btCollisionObject *obj = m_world->getCollisionObjectArray()[i];
        m_world->removeCollisionObject(obj);
        const auto body = dynamic_cast<btRigidBody *>(obj);

        while (body->getNumConstraintRefs()) {
            btTypedConstraint *constraint = body->getConstraintRef(0);
            m_world->removeConstraint(constraint);
            delete constraint;
        }

        m_world->removeRigidBody(body);
        delete body;
    }

    delete m_world;
    delete m_broad_phase;
    delete m_dispatcher;
    delete m_collision_configuration;
    delete m_constraint_solver;
}