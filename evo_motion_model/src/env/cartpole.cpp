//
// Created by samuel on 18/12/22.
//

#include "cartpole.h"

#include <evo_motion_model/item.h>

#include "../controller/slider.h"

CartPole::CartPole(
    int num_threads, int seed, float slider_speed, float slider_force, float chariot_push_force,
    float limit_angle, int reset_frame_nb, float chariot_mass, float pendulum_mass, int max_steps)
    : Environment(num_threads), chariot_push_force(chariot_push_force), limit_angle(limit_angle),
      reset_frame_nb(reset_frame_nb), rng(seed), rd_uni(0.f, 1.f), step_idx(0),
      max_steps(max_steps), last_vel(0.f), last_ang_vel(0.f) {
    float base_height = 2.f, base_pos = -4.f;

    float pendulum_height = 0.7f, pendulum_width = 0.1f, pendulum_offset = pendulum_height / 4.f;

    float chariot_height = 0.25f, chariot_width = 0.5f;

    chariot_pos = base_pos + base_height + chariot_height;
    pendulum_pos = chariot_pos + chariot_height + pendulum_height - pendulum_offset;

    // Create items
    // (init graphical and physical objects)
    Item base(
        "base", std::make_shared<ObjShape>("./resources/obj/cube.obj"),
        glm::vec3(0.f, base_pos, 10.f), glm::vec3(10.f, base_height, 10.f), 0.f, TILE_SPECULAR);

    Item chariot(
        "chariot", std::make_shared<ObjShape>("./resources/obj/cube.obj"),
        glm::vec3(0.f, chariot_pos, 10.f), glm::vec3(chariot_width, chariot_height, chariot_width),
        chariot_mass, SPECULAR);

    Item pendulum(
        "pendulum", std::make_shared<ObjShape>("./resources/obj/cube.obj"),
        glm::vec3(0.f, pendulum_pos, 10.f),
        glm::vec3(pendulum_width, pendulum_height, pendulum_width), pendulum_mass, SPECULAR);

    // Environment item vector
    items = {base, chariot, pendulum};

    for (const auto &i: items) add_item(i);

    // For slider between chariot and base
    btTransform tr_base;
    tr_base.setIdentity();
    tr_base.setOrigin(btVector3(0.f, base_height, 0.f));

    btTransform tr_chariot;
    tr_chariot.setIdentity();
    tr_chariot.setOrigin(btVector3(0.f, -chariot_height, 0.f));

    slider =
        new btSliderConstraint(*base.get_body(), *chariot.get_body(), tr_base, tr_chariot, true);

    controllers.push_back(std::make_shared<SliderController>(0, slider, slider_speed));

    slider->setEnabled(true);
    slider->setPoweredLinMotor(true);
    slider->setMaxLinMotorForce(slider_force);
    slider->setTargetLinMotorVelocity(0.f);
    slider->setLowerLinLimit(-10.f);
    slider->setUpperLinLimit(10.f);

    // For hinge between pendule and chariot
    btVector3 axis(0.f, 0.f, 1.f);
    hinge = new btHingeConstraint(
        *chariot.get_body(), *pendulum.get_body(), btVector3(0.f, chariot_height, 0.f),
        btVector3(0.f, -pendulum_height + pendulum_offset, 0.f), axis, axis, true);

    base_rg = base.get_body();
    chariot_rg = chariot.get_body();
    pendulum_rg = pendulum.get_body();

    base_rg->setActivationState(DISABLE_DEACTIVATION);
    chariot_rg->setActivationState(DISABLE_DEACTIVATION);
    pendulum_rg->setActivationState(DISABLE_DEACTIVATION);

    chariot_rg->setIgnoreCollisionCheck(pendulum_rg, true);
    base_rg->setIgnoreCollisionCheck(chariot_rg, true);
    base_rg->setIgnoreCollisionCheck(pendulum_rg, true);

    m_world->addConstraint(slider);
    m_world->addConstraint(hinge);
}

std::vector<Item> CartPole::get_items() { return items; }

std::vector<std::shared_ptr<Controller>> CartPole::get_controllers() { return controllers; }

step CartPole::compute_step() {
    const float pos = chariot_rg->getWorldTransform().getOrigin().x();
    const float base_pos = base_rg->getWorldTransform().getOrigin().x();
    float center_distance = abs(pos - base_pos);
    float vel = chariot_rg->getLinearVelocity().x();

    const float ang = pendulum_rg->getWorldTransform().getRotation().getAngle();
    float ang_vel = pendulum_rg->getAngularVelocity().z();

    const torch::Tensor state = torch::tensor(
        {center_distance / 10.f, (pos - base_pos) / 10.f, vel, vel - last_vel,
         ang / static_cast<float>(2. * M_PI) - 1.f, ang_vel, ang_vel - last_ang_vel},
        at::TensorOptions().device(curr_device));

    bool fail = pos > 10.f || pos < -10.f || ang > limit_angle || ang < -limit_angle;
    bool win = step_idx > max_steps;
    const bool done = fail || win;
    float reward = (limit_angle - abs(ang)) / limit_angle + (10.f - center_distance) / 10.f;
    reward = fail ? -2.f : win ? 2.f : reward;

    last_vel = vel;
    last_ang_vel = ang_vel;

    step_idx += 1;

    return {state, reward, done};
}

void CartPole::reset_engine() {
    // Remove chariot and pendulum rigid body from bullet world
    m_world->removeRigidBody(chariot_rg);
    m_world->removeRigidBody(pendulum_rg);

    // Remove slider and hinge constraint from bullet world
    m_world->removeConstraint(slider);
    m_world->removeConstraint(hinge);

    // Reset chariot position, force and velocity
    btTransform tr_chariot_reset;
    tr_chariot_reset.setIdentity();
    tr_chariot_reset.setOrigin(btVector3(0.f, chariot_pos, 10.f));

    chariot_rg->setWorldTransform(tr_chariot_reset);
    chariot_rg->getMotionState()->setWorldTransform(tr_chariot_reset);

    chariot_rg->setLinearVelocity(btVector3(0.f, 0.f, 0.f));
    chariot_rg->setAngularVelocity(btVector3(0.f, 0.f, 0.f));
    chariot_rg->clearForces();

    // Reset pendule position, force and velocity
    btTransform tr_pendule_reset;
    tr_pendule_reset.setIdentity();
    tr_pendule_reset.setOrigin(btVector3(0.f, pendulum_pos, 10.f));

    pendulum_rg->setWorldTransform(tr_pendule_reset);
    pendulum_rg->getMotionState()->setWorldTransform(tr_pendule_reset);

    pendulum_rg->setLinearVelocity(btVector3(0.f, 0.f, 0.f));
    pendulum_rg->setAngularVelocity(btVector3(0.f, 0.f, 0.f));
    pendulum_rg->clearForces();

    // Add chariot, pendule and constraints to bullet world
    m_world->addRigidBody(chariot_rg);
    m_world->addRigidBody(pendulum_rg);
    m_world->addConstraint(slider);
    m_world->addConstraint(hinge);

    slider->setPoweredLinMotor(false);

    // Apply random force to chariot for reset_frame_nb steps
    // To prevent over-fitting

    const float rand_force = rd_uni(rng) * chariot_push_force * 2.f - chariot_push_force;
    chariot_rg->applyCentralImpulse(btVector3(rand_force, 0.f, 0.f));
    //chariot_rg->setLinearVelocity(btVector3(rand_force, 0, 0));

    for (int i = 0; i < reset_frame_nb; i++) step_world(1.f / 60.f);

    slider->setPoweredLinMotor(true);

    step_idx = 0;
}

std::vector<int64_t> CartPole::get_state_space() { return {7}; }

std::vector<int64_t> CartPole::get_action_space() { return {1}; }

std::optional<Item> CartPole::get_camera_track_item() { return std::nullopt; }
