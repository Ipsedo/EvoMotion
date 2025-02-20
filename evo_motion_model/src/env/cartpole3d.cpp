//
// Created by samuel on 25/12/22.
//

#include "cartpole3d.h"

#include "../controller/slider.h"

CartPole3d::CartPole3d(
    int num_threads, int seed, float slider_speed, float slider_force_per_kg,
    float chariot_push_force, int reset_frame_nb, float limit_angle, float cart_x_mass,
    float cart_z_mass, float pole_mass, int max_steps)
    : Environment(num_threads), reset_frame_nb(reset_frame_nb),
      chariot_push_force(chariot_push_force), cart_x_scale(0.5f, 0.125f, 0.5f),
      cart_z_scale(0.5f, 0.125f, 0.5f), pole_scale(0.1f, 0.5f, 0.1f), base_scale(10.f, 1.f, 10.f),
      base_pos(0.f, -4.f, 10.f),
      cart_x_pos(base_pos.x(), base_pos.y() + base_scale.y() + cart_x_scale.y(), base_pos.z()),
      cart_z_pos(base_pos.x(), cart_x_pos.y() + cart_x_scale.y() + cart_z_scale.y(), base_pos.z()),
      pole_pos(
          base_pos.x(), cart_z_pos.y() + cart_z_scale.y() + pole_scale.y() - pole_scale.y() / 4.f,
          base_pos.z()),
      base_mass(0.f), rng(seed), rd_uni(0.f, 1.f), last_vel_x(0.f), last_vel_z(0.f), last_ang(0.f),
      last_ang_vel(0.f), last_ang_vel_vec(0.f, 0.f, 0.f), last_vert_ang(0.f),
      last_vert_ang_vel(0.f), last_plan_ang(0.f), last_plan_ang_vec(0.f), limit_angle(limit_angle),
      step_idx(0), max_steps(max_steps) {

    auto base = std::make_shared<RigidBodyItem>(
        "base", std::make_shared<ObjShape>("./resources/obj/cube.obj"),
        glm::vec3(base_pos.x(), base_pos.y(), base_pos.z()),
        glm::vec3(base_scale.x(), base_scale.y(), base_scale.z()), base_mass, TILE_SPECULAR);

    auto cart_x = std::make_shared<RigidBodyItem>(
        "cart_x", std::make_shared<ObjShape>("./resources/obj/cube.obj"),
        glm::vec3(cart_x_pos.x(), cart_x_pos.y(), cart_x_pos.z()),
        glm::vec3(cart_x_scale.x(), cart_x_scale.y(), cart_x_scale.z()), cart_x_mass, SPECULAR);

    auto cart_z = std::make_shared<RigidBodyItem>(
        "cart_z", std::make_shared<ObjShape>("./resources/obj/cube.obj"),
        glm::vec3(cart_z_pos.x(), cart_z_pos.y(), cart_z_pos.z()),
        glm::vec3(cart_z_scale.x(), cart_z_scale.y(), cart_z_scale.z()), cart_z_mass, SPECULAR);

    auto pole = std::make_shared<RigidBodyItem>(
        "pole", std::make_shared<ObjShape>("./resources/obj/cylinder.obj"),
        glm::vec3(pole_pos.x(), pole_pos.y(), pole_pos.z()),
        glm::vec3(pole_scale.x(), pole_scale.y(), pole_scale.z()), pole_mass, SPECULAR);

    items = {base, cart_x, cart_z, pole};

    for (const auto &item: items) m_world->addRigidBody(item->get_body());

    base_rg = base->get_body();
    cart_x_rg = cart_x->get_body();
    cart_z_rg = cart_z->get_body();
    pole_rg = pole->get_body();

    btTransform tr_base;
    tr_base.setIdentity();
    tr_base.setOrigin(btVector3(0.f, base_scale.y(), 0.f));

    btTransform tr_cart_x;
    tr_cart_x.setIdentity();
    tr_cart_x.setOrigin(btVector3(0.f, -cart_x_scale.y(), 0.f));

    slider_x =
        new btSliderConstraint(*base->get_body(), *cart_x->get_body(), tr_base, tr_cart_x, true);

    slider_x->setEnabled(true);
    slider_x->setPoweredLinMotor(true);
    slider_x->setMaxLinMotorForce(slider_force_per_kg * (cart_x_mass + cart_z_mass + pole_mass));
    slider_x->setTargetLinMotorVelocity(0.f);

    slider_x->setLowerLinLimit(-100.f);
    slider_x->setUpperLinLimit(100.f);

    slider_x->setSoftnessOrthoLin(2.f);
    slider_x->setSoftnessOrthoAng(2.f);
    slider_x->setSoftnessDirLin(2.f);
    slider_x->setSoftnessDirAng(2.f);
    slider_x->setSoftnessLimLin(2.f);
    slider_x->setSoftnessLimAng(2.f);

    controllers.push_back(std::make_shared<SliderController>(0, slider_x, slider_speed));

    tr_cart_x.setIdentity();
    tr_cart_x.setOrigin(btVector3(0.f, cart_x_scale.y(), 0.f));
    tr_cart_x.getBasis().setEulerZYX(0.f, M_PI / 2.f, 0.f);

    btTransform tr_cart_z;
    tr_cart_z.setIdentity();
    tr_cart_z.setOrigin(btVector3(0.f, -cart_z_scale.y(), 0.f));
    tr_cart_z.getBasis().setEulerZYX(0.f, M_PI / 2.f, 0.f);

    slider_z = new btSliderConstraint(*cart_x_rg, *cart_z_rg, tr_cart_x, tr_cart_z, true);

    slider_z->setEnabled(true);
    slider_z->setPoweredLinMotor(true);
    slider_z->setMaxLinMotorForce(slider_force_per_kg * (cart_z_mass + pole_mass));
    slider_z->setTargetLinMotorVelocity(0.f);

    slider_z->setLowerLinLimit(-100.f);
    slider_z->setUpperLinLimit(100.f);

    slider_z->setSoftnessOrthoLin(2.f);
    slider_z->setSoftnessOrthoAng(2.f);
    slider_z->setSoftnessDirLin(2.f);
    slider_z->setSoftnessDirAng(2.f);
    slider_z->setSoftnessLimLin(2.f);
    slider_z->setSoftnessLimAng(2.f);

    controllers.push_back(std::make_shared<SliderController>(1, slider_z, slider_speed));

    p2p_constraint = new btPoint2PointConstraint(
        *cart_z_rg, *pole_rg, btVector3(0.f, cart_z_scale.y(), 0.f),
        btVector3(0.f, -pole_scale.y() + pole_scale.y() / 4.f, 0.f));

    base_rg->setActivationState(DISABLE_DEACTIVATION);
    cart_x_rg->setActivationState(DISABLE_DEACTIVATION);
    cart_z_rg->setActivationState(DISABLE_DEACTIVATION);
    pole_rg->setActivationState(DISABLE_DEACTIVATION);

    cart_z_rg->setIgnoreCollisionCheck(pole_rg, true);
    cart_x_rg->setIgnoreCollisionCheck(pole_rg, true);
    cart_x_rg->setIgnoreCollisionCheck(cart_z_rg, true);
    base_rg->setIgnoreCollisionCheck(cart_x_rg, true);
    base_rg->setIgnoreCollisionCheck(cart_z_rg, true);
    base_rg->setIgnoreCollisionCheck(pole_rg, true);

    m_world->addConstraint(slider_x);
    m_world->addConstraint(slider_z);
    m_world->addConstraint(p2p_constraint);
}

std::vector<std::shared_ptr<ShapeItem>> CartPole3d::get_draw_items() {
    std::vector<std::shared_ptr<ShapeItem>> abs_items;
    std::transform(
        items.begin(), items.end(), std::back_inserter(abs_items), [](const auto &i) { return i; });
    return abs_items;
}

std::vector<std::shared_ptr<Controller>> CartPole3d::get_controllers() { return controllers; }

step CartPole3d::compute_step() {
    float pos_x = cart_z_rg->getWorldTransform().getOrigin().x();
    float pos_z = cart_z_rg->getWorldTransform().getOrigin().z();

    float vel_x = cart_z_rg->getLinearVelocity().x();
    float vel_z = cart_z_rg->getLinearVelocity().z();

    const float center_distance =
        sqrt(pow(cart_z_pos.x() - pos_x, 2.f) + pow(cart_z_pos.z() - pos_z, 2.f));
    pos_x = pos_x - cart_z_pos.x();
    pos_z = pos_z - cart_z_pos.z();

    const auto ang_quaternion = pole_rg->getWorldTransform().getRotation();
    float ang_x = 0.f, ang_y = 0.f, ang_z = 0.f;
    ang_quaternion.getEulerZYX(ang_x, ang_y, ang_z);

    btTransform identity;
    identity.setIdentity();
    identity.setOrigin(pole_pos);
    const btVector3 origin = btVector3(0, 1, 0).rotate(
        identity.getRotation().getAxis(), identity.getRotation().getAngle());
    const btVector3 rotated_ang = btVector3(0, 1, 0).rotate(
        pole_rg->getWorldTransform().getRotation().getAxis(),
        pole_rg->getWorldTransform().getRotation().getAngle());

    const float ang = acos(origin.dot(rotated_ang) / (origin.norm() * rotated_ang.norm()));
    float ang_vel = ang - last_ang;

    const auto ang_vel_vec = pole_rg->getAngularVelocity();
    const auto ang_acc_vec = ang_vel_vec - last_ang_vel_vec;

    const auto axis = pole_rg->getWorldTransform().getRotation().getAxis();

    const btVector3 axis_ori(0.f, 1.f, 0.f);
    const float vertical_ang = acos(
        (axis.x() * axis_ori.x() + axis.y() * axis_ori.y() + axis.z() * axis_ori.z())
        / (axis.norm() + axis_ori.norm()));
    float vertical_ang_vel = vertical_ang - last_vert_ang;

    const btVector3 axis_plan(axis.x(), 0.f, axis.z());
    const btVector3 axis_plan_ori(1.f, 0.f, 0.f);
    const float plan_ang = acos(
        (axis_plan.x() * axis_plan_ori.x() + axis_plan.y() * axis_plan_ori.y()
         + axis_plan.z() * axis_plan_ori.z())
        / (axis_plan.norm() + axis_plan_ori.norm()));
    float plan_ang_vel = plan_ang - last_plan_ang;

    const torch::Tensor state = torch::tensor(
        {center_distance / base_scale.x(),
         pos_x / base_scale.x(),
         vel_x,
         vel_x - last_vel_x,
         pos_z / base_scale.z(),
         vel_z,
         vel_z - last_vel_z,
         ang_x / static_cast<float>(M_PI),
         ang_y / static_cast<float>(M_PI),
         ang_z / static_cast<float>(M_PI),
         ang / static_cast<float>(2. * M_PI) - 1.f,
         ang_vel,
         ang_vel - last_ang_vel,
         ang_vel_vec.x(),
         ang_vel_vec.y(),
         ang_vel_vec.z(),
         ang_acc_vec.x(),
         ang_acc_vec.y(),
         ang_acc_vec.z(),
         axis.x(),
         axis.y(),
         axis.z(),
         plan_ang / static_cast<float>(M_PI),
         plan_ang_vel,
         plan_ang_vel - last_plan_ang_vec,
         vertical_ang / static_cast<float>(M_PI),
         vertical_ang_vel,
         vertical_ang_vel - last_vert_ang_vel},
        at::TensorOptions().device(curr_device));

    bool fail = center_distance > base_scale.x() || abs(ang) > limit_angle;
    bool win = step_idx > max_steps;

    const bool done = fail || win;

    float reward = pow((limit_angle - abs(ang)) / limit_angle, 2.f)
                   + pow((base_scale.x() - center_distance) / base_scale.x(), 2.f);
    reward = fail ? -2.f : win ? 2.f : reward;

    last_vel_x = vel_x;
    last_vel_z = vel_z;

    last_ang = ang;
    last_ang_vel = ang_vel;

    last_ang_vel_vec = ang_vel_vec;

    last_plan_ang = plan_ang;
    last_plan_ang_vec = plan_ang_vel;

    last_vert_ang = vertical_ang;
    last_vert_ang_vel = vertical_ang_vel;

    step_idx += 1;

    return {state, reward, done};
}

void CartPole3d::reset_engine() {
    // Remove cart and pendulum rigid body from bullet world
    m_world->removeRigidBody(pole_rg);
    m_world->removeRigidBody(cart_z_rg);
    m_world->removeRigidBody(cart_x_rg);

    // Remove slider and hinge constraint from bullet world
    m_world->removeConstraint(slider_z);
    m_world->removeConstraint(slider_x);
    m_world->removeConstraint(p2p_constraint);

    // Reset cart position, force and velocity
    btTransform tr_cart_x_reset;
    tr_cart_x_reset.setIdentity();
    tr_cart_x_reset.setOrigin(cart_x_pos);

    cart_x_rg->setWorldTransform(tr_cart_x_reset);
    cart_x_rg->getMotionState()->setWorldTransform(tr_cart_x_reset);

    cart_x_rg->setLinearVelocity(btVector3(0.f, 0.f, 0.f));
    cart_x_rg->setAngularVelocity(btVector3(0.f, 0.f, 0.f));
    cart_x_rg->clearForces();

    btTransform tr_cart_z_reset;
    tr_cart_z_reset.setIdentity();
    tr_cart_z_reset.setOrigin(cart_z_pos);

    cart_z_rg->setWorldTransform(tr_cart_z_reset);
    cart_z_rg->getMotionState()->setWorldTransform(tr_cart_z_reset);

    cart_z_rg->setLinearVelocity(btVector3(0.f, 0.f, 0.f));
    cart_z_rg->setAngularVelocity(btVector3(0.f, 0.f, 0.f));
    cart_z_rg->clearForces();

    // Reset pole position, force and velocity
    btTransform tr_pole_reset;
    tr_pole_reset.setIdentity();
    tr_pole_reset.setOrigin(pole_pos);

    pole_rg->setWorldTransform(tr_pole_reset);
    pole_rg->getMotionState()->setWorldTransform(tr_pole_reset);

    pole_rg->setLinearVelocity(btVector3(0.f, 0.f, 0.f));
    pole_rg->setAngularVelocity(btVector3(0.f, 0.f, 0.f));
    pole_rg->clearForces();

    // Add cart, pole and constraints to bullet world
    m_world->addRigidBody(cart_x_rg);
    m_world->addRigidBody(cart_z_rg);
    m_world->addRigidBody(pole_rg);

    m_world->addConstraint(slider_x);
    m_world->addConstraint(slider_z);
    m_world->addConstraint(p2p_constraint);

    slider_z->setPoweredLinMotor(false);
    slider_x->setPoweredLinMotor(false);

    // Apply random force to chariot for reset_frame_nb steps
    // To prevent over-fitting
    const float angle = rd_uni(rng) * static_cast<float>(M_PI) * 2.f;
    const float rand_force = rd_uni(rng) * chariot_push_force;

    const float rand_force_x = cos(angle) * rand_force;
    const float rand_force_z = sin(angle) * rand_force;

    cart_z_rg->applyCentralImpulse(btVector3(rand_force_x, 0.f, rand_force_z));

    for (int i = 0; i < reset_frame_nb; i++) step_world(1.f / 60.f);

    slider_z->setPoweredLinMotor(true);
    slider_x->setPoweredLinMotor(true);

    step_idx = 0;
}

std::vector<int64_t> CartPole3d::get_state_space() { return {28}; }

std::vector<int64_t> CartPole3d::get_action_space() { return {2}; }

std::optional<std::shared_ptr<ShapeItem>> CartPole3d::get_camera_track_item() {
    return std::nullopt;
}
