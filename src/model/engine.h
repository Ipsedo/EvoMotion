//
// Created by samuel on 11/08/19.
//

#ifndef EVOMOTION_ENGINE_H
#define EVOMOTION_ENGINE_H

#include <btBulletDynamicsCommon.h>
#include <vector>
#include "item.h"

struct engine {
    btDefaultCollisionConfiguration *m_collision_configuration;
    btCollisionDispatcher *m_dispatcher;
    btBroadphaseInterface *m_broad_phase;
    btSequentialImpulseConstraintSolver *m_constraint_solver;
    btDiscreteDynamicsWorld *m_world;

    explicit engine(std::vector<item> objects);
    void step(float delta);
    ~engine();
};

#endif //EVOMOTION_ENGINE_H
