//
// Created by samuel on 11/08/19.
//

#include "env_list.h"
#include "../model/item.h"

environment create_test_env() {
    int nb_box = 10;

    float max_height = 10.f, min_height = 5.f, max_scale = 0.1f;

    auto rend = renderer(1920, 1080);

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
    return environment(rend, items, torch::IntArrayRef({1}), torch::IntArrayRef({1}) ,act, step, [](std::vector<item> v){return;});
}

environment create_cartpole_env() {
    auto rend = renderer(1920, 1080);

    /**
     * /!\ BE CAREFUL /!\
     *
     * En raison de précision flottante, ne mettre que des multiple de 2^n.
     *
     * Sinon les premières itérations de la librairie bullet vont générer un "sursaut"
     * dans le mouvement des objets pour satisfaire les contraintes physiques (slider et hinge).
     * Produit une explosion numerique dans le state qui peut entrainer un reset de l'environnement.
     */
    float base_height = 2.f, base_pos = -4.f;

    float pendule_height = 0.5f, pendule_width = 0.0625f;

    float chariot_height = 0.125f, chariot_width = 0.5f;

    float chariot_pos = base_pos + base_height + chariot_height;
    float pendule_pos = chariot_pos + chariot_height + pendule_height;

    item base = create_item_box(glm::vec3(0.f, base_pos, 10.f), glm::mat4(1.f), glm::vec3(10.f, base_height, 10.f), 0.f);

    item chariot = create_item_box(glm::vec3(0.f, chariot_pos, 10.f),
            glm::mat4(1.f),
            glm::vec3(chariot_width, chariot_height, chariot_width), 1.f);
    item pendule = create_item_box(glm::vec3(0.f, pendule_pos, 10.f),
            glm::mat4(1.f),
            glm::vec3(pendule_width, pendule_height, pendule_width), 0.1f);

    std::vector<item> items{base, chariot, pendule};

    // For slider between chariot and base
    btTransform tr_base;
    tr_base.setIdentity();
    tr_base.setOrigin(btVector3(0.f, base_height, 0.f));

    btTransform tr_chariot;
    tr_chariot.setIdentity();
    tr_chariot.setOrigin(btVector3(0.f, -chariot_height, 0.f));

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
                                        btVector3(0.f, chariot_height, 0.f),
                                        btVector3(0.f, -pendule_height, 0.f),
                                        axis, axis, true);

    float force_scale = 2.f;

    auto act = [slider, force_scale](torch::Tensor action, std::vector<item> items) {
        slider->setTargetLinMotorVelocity((action.item().toDouble() > 0 ? 1.f : -1.f) * force_scale);
    };

    btRigidBody *base_rg = base.m_rg_body;
    btRigidBody *chariot_rg = chariot.m_rg_body;
    btRigidBody *pendule_rg = pendule.m_rg_body;
    chariot_rg->setIgnoreCollisionCheck(pendule_rg, true);
    base_rg->setIgnoreCollisionCheck(chariot_rg, true);
    base_rg->setIgnoreCollisionCheck(pendule_rg, true);

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

    auto reset = [chariot_rg, pendule_rg, chariot_pos, pendule_pos, slider](std::vector<item> v){
        btTransform tr_chariot_reset;
        tr_chariot_reset.setIdentity();
        tr_chariot_reset.setOrigin(btVector3(0.f, chariot_pos, 10.f));
        chariot_rg->getMotionState()->setWorldTransform(tr_chariot_reset);
        chariot_rg->setWorldTransform(tr_chariot_reset);
        chariot_rg->clearForces();
        chariot_rg->setLinearVelocity(btVector3(0, 0, 0));
        chariot_rg->setAngularVelocity(btVector3(0, 0, 0));

        btTransform tr_pendule_reset;
        tr_pendule_reset.setIdentity();
        tr_pendule_reset.setOrigin(btVector3(0.f, pendule_pos, 10.f));
        pendule_rg->getMotionState()->setWorldTransform(tr_pendule_reset);
        pendule_rg->setWorldTransform(tr_pendule_reset);
        pendule_rg->clearForces();
        pendule_rg->setLinearVelocity(btVector3(0, 0, 0));
        pendule_rg->setAngularVelocity(btVector3(0, 0, 0));

        slider->setTargetLinMotorVelocity(0.f);
    };

    auto action_sizes = torch::IntArrayRef({1});
    auto states_sizes = torch::IntArrayRef({4});

    environment env(rend, items, action_sizes, states_sizes, act, step, reset);
    env.m_engine.m_world->addConstraint(slider);
    env.m_engine.m_world->addConstraint(hinge);

    return env;

}
