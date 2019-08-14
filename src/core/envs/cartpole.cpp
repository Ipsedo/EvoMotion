//
// Created by samuel on 13/08/19.
//

#include "cartpole.h"

CartPoleEnv::CartPoleEnv(int seed) : rd_gen(seed), rd_uni(0.f, 1.f), slider_speed(5.f), chariot_push_force(100.f),
    Environment(renderer(1920, 1080), init_cartpole())  {
    m_engine.m_world->addConstraint(slider);
    m_engine.m_world->addConstraint(hinge);
}

std::vector<item> CartPoleEnv::init_cartpole() {
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

    chariot_pos = base_pos + base_height + chariot_height;
    pendule_pos = chariot_pos + chariot_height + pendule_height;

    item base = create_item_box(glm::vec3(0.f, base_pos, 10.f), glm::mat4(1.f), glm::vec3(10.f, base_height, 10.f), 0.f);

    item chariot = create_item_box(glm::vec3(0.f, chariot_pos, 10.f),
                                   glm::mat4(1.f),
                                   glm::vec3(chariot_width, chariot_height, chariot_width), 1.f);
    item pendule = create_item_box(glm::vec3(0.f, pendule_pos, 10.f),
                                   glm::mat4(1.f),
                                   glm::vec3(pendule_width, pendule_height, pendule_width), 0.5f);

    std::vector<item> items{base, chariot, pendule};

    // For slider between chariot and base
    btTransform tr_base;
    tr_base.setIdentity();
    tr_base.setOrigin(btVector3(0.f, base_height, 0.f));

    btTransform tr_chariot;
    tr_chariot.setIdentity();
    tr_chariot.setOrigin(btVector3(0.f, -chariot_height, 0.f));

    slider = new btSliderConstraint(*base.m_rg_body, *chariot.m_rg_body, tr_base, tr_chariot, true);

    slider->setEnabled(true);
    slider->setPoweredLinMotor(true);
    slider->setMaxLinMotorForce(300.f);
    slider->setTargetLinMotorVelocity(0.f);
    slider->setLowerLinLimit(-10.f);
    slider->setUpperLinLimit(10.f);

    // For hinge between pendule and chariot
    btVector3 axis(0.f, 0.f, 1.f);
    hinge = new btHingeConstraint(*chariot.m_rg_body, *pendule.m_rg_body,
                                        btVector3(0.f, chariot_height, 0.f),
                                        btVector3(0.f, -pendule_height, 0.f),
                                        axis, axis, true);

    base_rg = base.m_rg_body;
    chariot_rg = chariot.m_rg_body;
    pendule_rg = pendule.m_rg_body;

    chariot_rg->setIgnoreCollisionCheck(pendule_rg, true);
    base_rg->setIgnoreCollisionCheck(chariot_rg, true);
    base_rg->setIgnoreCollisionCheck(pendule_rg, true);

    return items;
}

torch::IntArrayRef CartPoleEnv::action_space() {
    return torch::IntArrayRef({2});
}

torch::IntArrayRef CartPoleEnv::state_space() {
    return torch::IntArrayRef({4});
}

CartPoleEnv::~CartPoleEnv() {

}

void CartPoleEnv::act(torch::Tensor action) {
    int act_idx = action.argmax(-1).item().toInt();
    slider->setTargetLinMotorVelocity((act_idx == 0 ? 1.f : -1.f) * slider_speed);
}

env_step CartPoleEnv::compute_new_state() {
    float pos = chariot_rg->getWorldTransform().getOrigin().x();
    float vel = chariot_rg->getLinearVelocity().x();

    float ang = pendule_rg->getWorldTransform().getRotation().getAngle();
    float ang_vel = pendule_rg->getAngularVelocity().z();

    torch::Tensor state = torch::tensor({pos, vel, ang, ang_vel});

    bool done = pos > 8.f || pos < -8.f || ang > M_PI * 0.5 || ang < -M_PI * 0.5;
    float reward = abs(float(M_PI * 0.5) - abs(ang) / float(M_PI * 0.5));
    //float reward = done ? 0.f : 1.f;

    env_step new_state {state, reward, done};
    return new_state;
}

env_step CartPoleEnv::reset_engine() {
    slider->setTargetLinMotorVelocity(0.f);

    btTransform tr_chariot_reset;
    tr_chariot_reset.setIdentity();
    tr_chariot_reset.setOrigin(btVector3(0.f, chariot_pos, 10.f));

    chariot_rg->setWorldTransform(tr_chariot_reset);
    chariot_rg->getMotionState()->setWorldTransform(tr_chariot_reset);

    chariot_rg->setLinearVelocity(btVector3(0.f, 0.f, 0.f));
    chariot_rg->setAngularVelocity(btVector3(0.f, 0.f, 0.f));
    chariot_rg->clearForces();

    float dir = rd_uni(rd_gen) - 0.5f;
    chariot_rg->applyCentralImpulse(btVector3(0.f, dir * chariot_push_force, 0.f));

    // Pendule
    btTransform tr_pendule_reset;
    tr_pendule_reset.setIdentity();
    tr_pendule_reset.setOrigin(btVector3(0.f, pendule_pos, 10.f));

    pendule_rg->setWorldTransform(tr_pendule_reset);
    pendule_rg->getMotionState()->setWorldTransform(tr_pendule_reset);

    pendule_rg->setLinearVelocity(btVector3(0.f, 0.f, 0.f));
    pendule_rg->setAngularVelocity(btVector3(0.f, 0.f, 0.f));
    pendule_rg->clearForces();

    return compute_new_state();
}
