//
// Created by samuel on 18/12/22.
//

#include <torch/torch.h>

#include <evo_motion_model/environment.h>

#include "./constants.h"

InitBtThread::InitBtThread(const int num_threads) {
    btSetTaskScheduler(btCreateDefaultTaskScheduler());
    btGetTaskScheduler()->setNumThreads(num_threads);
    cci.m_defaultMaxPersistentManifoldPoolSize = 8192;
    cci.m_defaultMaxCollisionAlgorithmPoolSize = 8192;
}

btDefaultCollisionConstructionInfo InitBtThread::get_cci() const { return cci; }

Environment::Environment(const int num_threads)
    : num_threads(num_threads), init_thread(num_threads), curr_device(torch::kCPU),
      m_collision_configuration(new btDefaultCollisionConfiguration(init_thread.get_cci())),
      m_dispatcher(new btCollisionDispatcherMt(m_collision_configuration, 40)),
      m_broad_phase(new btDbvtBroadphase()),
      m_pool_solver(new btConstraintSolverPoolMt(num_threads)),
      m_constraint_solver(new btSequentialImpulseConstraintSolverMt()),
      m_world(new btDiscreteDynamicsWorldMt(
          m_dispatcher, m_broad_phase, m_pool_solver, m_constraint_solver,
          m_collision_configuration)) {
    m_world->setGravity(btVector3(0, -9.8f, 0));
}

void Environment::add_item(const Item &item) const { m_world->addRigidBody(item.get_body()); }

step Environment::do_step(const torch::Tensor &action) {
    for (const auto &c: get_controllers()) c->on_input(action);

    step_world(DELTA_T_MODEL);

    return compute_step();
}

void Environment::step_world(const float delta) const {
    m_world->stepSimulation(delta, num_threads, delta);
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