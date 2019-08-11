//
// Created by samuel on 11/08/19.
//

#ifndef EVOMOTION_ENVIRONMENT_H
#define EVOMOTION_ENVIRONMENT_H

#include <btBulletDynamicsCommon.h>
#include "../view/obj_mtl_vbo.h"
#include "../view/renderer.h"
#include "../model/engine.h"

struct environment {

    engine m_engine;
    renderer m_renderer;
    std::vector<item> m_items;

    bool will_draw;

    environment(renderer renderer, std::vector<item> items, bool will_draw);

    void step(float delta);
};

#endif //EVOMOTION_ENVIRONMENT_H
