//
// Created by samuel on 11/08/19.
//

#include "engine.h"

engine::engine(std::vector<item> objects) :
		m_collision_configuration(new btDefaultCollisionConfiguration()),
		m_dispatcher(new btCollisionDispatcher(m_collision_configuration)),
		m_broad_phase(new btDbvtBroadphase()),
		m_constraint_solver(new btSequentialImpulseConstraintSolver()),
		m_world(new btDiscreteDynamicsWorld(
				m_dispatcher,
				m_broad_phase,
				m_constraint_solver,
				m_collision_configuration)) {
	// Earth gravity
	m_world->setGravity(btVector3(0, -9.8f, 0));

	// Add item's rigidbody to bullet world
	for (item t : objects)
		m_world->addRigidBody(t.m_rg_body);
}

void engine::step(float delta) {
	m_world->stepSimulation(delta);
}

engine::~engine() {
	// For all world's physical objects
	for (int i = m_world->getNumCollisionObjects() - 1; i >= 0; i--) {
		// Get i-th rigidbody
		btCollisionObject *obj = m_world->getCollisionObjectArray()[i];
		btRigidBody *rg_body = btRigidBody::upcast(obj);

		if (rg_body->getMotionState()) {
			// Delete all object constraint
			while (rg_body->getNumConstraintRefs()) {
				btTypedConstraint *constraint = rg_body->getConstraintRef(0);
				m_world->removeConstraint(constraint);
				delete constraint;
			}

			// Delete rigidbody's motion state and collision shape
			delete rg_body->getMotionState();
			delete rg_body->getCollisionShape();

			// Remove rigidbody from world
			m_world->removeRigidBody(rg_body);
		}

		// Delete rigidbody
		delete rg_body;
	}
}
