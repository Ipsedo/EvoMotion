//
// Created by samuel on 11/08/19.
//

#include "engine.h"

engine::engine(std::vector<item> objects) {
    m_collision_configuration = new btDefaultCollisionConfiguration();
    m_dispatcher = new btCollisionDispatcher(m_collision_configuration);
    m_broad_phase = new btDbvtBroadphase();
    m_constraint_solver = new btSequentialImpulseConstraintSolver();

    m_world = new btDiscreteDynamicsWorld(
            m_dispatcher,
            m_broad_phase,
            m_constraint_solver,
            m_collision_configuration);
    m_world->setGravity(btVector3(0, -9.8f, 0));

    for (item t : objects)
        m_world->addRigidBody(t.m_rg_body);
}

void engine::step(float delta) {
    m_world->stepSimulation(delta);
}

engine::~engine() {
    for (int i = m_world->getNumCollisionObjects() - 1; i >= 0; i--) {
        btCollisionObject *obj = m_world->getCollisionObjectArray()[i];
        btRigidBody *rg_body = btRigidBody::upcast(obj);

        if (rg_body->getMotionState()) {
            while (rg_body->getNumConstraintRefs()) {
                btTypedConstraint *constraint = rg_body->getConstraintRef(0);
                m_world->removeConstraint(constraint);
                delete constraint;
            }

            delete rg_body->getMotionState();
            delete rg_body->getCollisionShape();
            m_world->removeRigidBody(rg_body);
        }

        delete rg_body;
    }
}
