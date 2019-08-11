//
// Created by samuel on 11/08/19.
//

#include "env_list.h"
#include "../model/item.h"

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

    auto act = [](torch::Tensor action, std::vector<item> items) {
        return;
    };
    auto step = [](std::vector<item> items) {
        return env_step();
    };
    return environment(rend, items, true, act, step);
}

environment cartpole_env() {
    auto rend = renderer(1920, 1080);
    rend.init();

    // TODO fix object relative place with correct constraint anchor pos
    item base = create_item_box(glm::vec3(0.f, -4.f, 10.f), glm::mat4(1.f), glm::vec3(10.f, 2.f, 10.f), 0.f);

    float size_pendule = 0.5;
    float width_pendule = 0.05f;

    float size_chariot = 0.12f;
    float width_chariot = 0.5f;

    float pos_chariot = -3.f;

    item chariot = create_item_box(glm::vec3(0.f, pos_chariot, 10.f),
            glm::mat4(1.f),
            glm::vec3(width_chariot, size_chariot, width_chariot), 1.f);
    item pendule = create_item_box(glm::vec3(0.f, pos_chariot + size_chariot * 0.5f - size_pendule * 0.5f, 10.f),
            glm::mat4(1.f),
            glm::vec3(width_pendule, size_pendule, width_pendule), 0.1f);

    std::vector<item> items{base, chariot, pendule};

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
    slider->setTargetLinMotorVelocity(0.f);
    slider->setLowerLinLimit(-10.f);
    slider->setUpperLinLimit(10.f);

    // For hinge between pendule and chariot
    btVector3 axis(0.f, 0.f, 1.f);
    auto *hinge = new btHingeConstraint(*chariot.m_rg_body, *pendule.m_rg_body,
                                        btVector3(0.f, size_chariot * 0.5f, 0.f),
                                        btVector3(0.f, -size_pendule * 0.5f, 0.f),
                                        axis, axis, true);

    float force_scale = 2.f;

    auto act = [slider, force_scale](torch::Tensor action, std::vector<item> items) {
        slider->setTargetLinMotorVelocity((action.item().toDouble() > 0 ? 1.f : -1.f) * force_scale);
    };

    btRigidBody *chariot_rg = chariot.m_rg_body;
    btRigidBody *pendule_rg = pendule.m_rg_body;
    chariot_rg->setIgnoreCollisionCheck(pendule_rg, true);

    auto step = [chariot_rg, pendule_rg](std::vector<item> items) {
        float pos = chariot_rg->getWorldTransform().getOrigin().x();
        float vel = chariot_rg->getLinearVelocity().x();

        float ang = pendule_rg->getWorldTransform().getRotation().getAngle();
        float ang_vel = pendule_rg->getAngularVelocity().z();

        torch::Tensor state = torch::zeros(4);
        state[0] = pos;
        state[1] = vel;
        state[2] = ang;
        state[3] = ang_vel;

        bool done = pos > 8.f || pos < -8.f || ang > 0.9 || ang < -0.9;

        env_step new_state {state, done ? 0.f : 1.f, done};
        return new_state;
    };

    environment env(rend, items, true, act, step);
    env.m_engine.m_world->addConstraint(slider);
    env.m_engine.m_world->addConstraint(hinge);

    return env;

}
