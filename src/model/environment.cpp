//
// Created by samuel on 18/12/22.
//

#include "./model/environment.h"

Environment::Environment(torch::IntArrayRef state_space, torch::IntArrayRef action_space, bool is_continuous) :
        state_space(state_space), action_space(action_space), continuous(is_continuous),
        m_collision_configuration(new btDefaultCollisionConfiguration()),
        m_dispatcher(new btCollisionDispatcher(m_collision_configuration)),
        m_broad_phase(new btDbvtBroadphase()),
        m_constraint_solver(new btSequentialImpulseConstraintSolver()),
        m_world(new btDiscreteDynamicsWorld(
                m_dispatcher,
                m_broad_phase,
                m_constraint_solver,
                m_collision_configuration)) {

    m_world->setGravity(btVector3(0, -9.8f, 0));

}

void Environment::add_item(Item item) {
    m_world->addRigidBody(item.get_body());
}

step Environment::do_step(const torch::Tensor& action, float delta) {
    for (const auto& c : get_controllers())
        c->on_input(action);

    m_world->stepSimulation(delta);

    return compute_step();
}

step Environment::reset() {
    reset_engine();
    return compute_step();
}

torch::IntArrayRef Environment::get_state_space() {
    return state_space;
}

torch::IntArrayRef Environment::get_action_space() {
    return action_space;
}

bool Environment::is_continuous() {
    return continuous;
}
