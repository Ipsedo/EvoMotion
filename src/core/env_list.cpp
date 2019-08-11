//
// Created by samuel on 11/08/19.
//

#include "env_list.h"
#include "../model/body.h"

environment create_test_env() {
    int nb_box = 10;

    float max_height = 10.f, min_height = 5.f, max_scale = 0.1f;

    auto rend = renderer(1920, 1080);
    rend.init();

    std::vector<item> items;
    for (int i = 0; i < nb_box; i++) {
        glm::vec3 pos(0.f, (float(rand()) / RAND_MAX) * (max_height - min_height) + max_height, 10.f);
        items.push_back(create_item_box(pos, glm::mat4(1.f), glm::vec3(max_scale * float(rand()) / RAND_MAX), 10.f));
    }

    items.push_back(create_item_box(glm::vec3(0.f, -3.f, 10.f), glm::mat4(1.f), glm::vec3(1.f), 0.f));

    return environment(rend, items, true);
}

environment cartpole_env() {
    auto rend = renderer(1920, 1080);
    rend.init();

    item base = create_item_box(glm::vec3(0.f, -1.f - 3.f, 10.f), glm::mat4(1.f), glm::vec3(10.f, 2.f, 10.f), 0.f);

    item chariot = create_item_box(glm::vec3(0.f, 0.6f - 2.f, 10.f), glm::mat4(1.f), glm::vec3(2.f, 0.25f, 1.f), 1.f);
    item pendule = create_item_box(glm::vec3(0.f, 2.f - 2.f, 10.f), glm::mat4(1.f), glm::vec3(0.1f, 1.f, 0.1f), 1.f);

    std::vector<item> items{base, chariot, pendule};

    environment env(rend, items, true);

    // For slider between chariot and base
    btTransform tr_base;
    tr_base.setIdentity();
    tr_base.setOrigin(btVector3(0.f, 3.f, 0.f));

    btTransform tr_chariot;
    tr_chariot.setIdentity();

    auto *slider = new btSliderConstraint(*base.m_rg_body, *chariot.m_rg_body, tr_base, tr_chariot, true);

    slider->setEnabled(true);
    slider->setPoweredLinMotor(true);
    slider->setMaxLinMotorForce(10.f);
    slider->setTargetLinMotorVelocity(-1.f);
    slider->setLowerLinLimit(-10.f);
    slider->setUpperLinLimit(10.f);

    env.m_engine.m_world->addConstraint(slider);

    // For hinge between pendule and chariot
    auto *hinge = new btHingeConstraint(*chariot.m_rg_body, *pendule.m_rg_body,
            btVector3(0.f, 0.4f, 0.f), btVector3(0.f, -1.1f, 0.f),
            btVector3(0.f, 0.f, 1.f), btVector3(0.f, 0.f, 1.f), true);

    env.m_engine.m_world->addConstraint(hinge);

    return env;

}
