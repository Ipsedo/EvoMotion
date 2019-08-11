//
// Created by samuel on 11/08/19.
//

#ifndef EVOMOTION_ENGINE_H
#define EVOMOTION_ENGINE_H

#include <btBulletDynamicsCommon.h>
#include <vector>
#include "body.h"

struct engine {
    btBroadphaseInterface *m_broad_phase;
    btCollisionDispatcher *m_dispatcher;
    btDefaultCollisionConfiguration *m_collision_configuration;
    btSequentialImpulseConstraintSolver *m_constraint_solver;
    btDiscreteDynamicsWorld *m_world;

    explicit engine(std::vector<item> objects);
    void step(float delta);
    ~engine();
};

#endif //EVOMOTION_ENGINE_H
