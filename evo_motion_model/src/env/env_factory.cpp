//
// Created by samuel on 25/12/22.
//

#include "./env_factory.h"

#include <utility>

#include "./cartpole.h"
#include "./cartpole3d.h"
#include "./constants.h"
#include "./robot_jump.h"
#include "./robot_walk.h"

/*
 * Env factory
 */

EnvironmentFactory::EnvironmentFactory(std::map<std::string, std::string> parameters)
    : parameters(std::move(parameters)) {}

template<typename Value>
Value EnvironmentFactory::generic_get_value(
    std::function<Value(const std::string &)> converter, const std::string &key,
    Value default_value) {
    if (parameters.find(key) == parameters.end()) return default_value;
    return converter(parameters[key]);
}

int EnvironmentFactory::get_value(const std::string &key, const int default_value) {
    return generic_get_value<int>([](const auto &e) { return std::stoi(e); }, key, default_value);
}

float EnvironmentFactory::get_value(const std::string &key, const float default_value) {
    return generic_get_value<float>([](const auto &e) { return std::stof(e); }, key, default_value);
}

std::string EnvironmentFactory::get_value(const std::string &key, std::string default_value) {
    return generic_get_value<std::string>(
        [](const auto &s) { return s; }, key, std::move(default_value));
}

/*
 * Factories
 */

CartPoleFactory::CartPoleFactory(std::map<std::string, std::string> parameters)
    : EnvironmentFactory(std::move(parameters)) {}

std::shared_ptr<Environment> CartPoleFactory::get_env(int num_threads, int seed) {
    return std::make_shared<CartPole>(
        num_threads, seed, get_value("slider_speed", 16.f), get_value("slider_force", 64.f),
        get_value("chariot_push_force", 2.f),
        get_value("limit_angle", static_cast<float>(M_PI * 0.5)), get_value("reset_frame_nb", 8),
        get_value("chariot_mass", 1.f), get_value("pendulum_mass", 1.f),
        get_value("mas_steps", 60 * 60));
}

CartPole3dFactory::CartPole3dFactory(std::map<std::string, std::string> parameters)
    : EnvironmentFactory(std::move(parameters)) {}

std::shared_ptr<Environment> CartPole3dFactory::get_env(int num_threads, int seed) {
    return std::make_shared<CartPole3d>(
        num_threads, seed, get_value("slider_speed", 16.f), get_value("slider_force_per_kg", 32.f),
        get_value("chariot_push_force", 2.f), get_value("reset_frame_nb", 8),
        get_value("limit_angle", static_cast<float>(M_PI) / 2.f), get_value("cart_x_mass", 1.f),
        get_value("cart_z_mass", 1.f), get_value("pole_mass", 1.f),
        get_value("max_steps", 60 * 60));
}

RobotWalkFactory::RobotWalkFactory(std::map<std::string, std::string> parameters)
    : EnvironmentFactory(std::move(parameters)) {}

std::shared_ptr<Environment> RobotWalkFactory::get_env(int num_threads, int seed) {
    return std::make_shared<RobotWalk>(
        num_threads, seed,
        get_value(
            "skeleton_json_path",
            std::filesystem::path(RESOURCES_PATH) / "./resources/skeleton/spider_new.json"),
        get_value("initial_remaining_seconds", 1.f), get_value("max_episode_seconds", 30.f),
        get_value("target_velocity", 5e-1f), get_value("minimal_velocity", 1e-1f),
        get_value("reset_frames", 10));
}

/*
 * Robot Jump
 */

RobotJumpFactory::RobotJumpFactory(std::map<std::string, std::string> parameters)
    : EnvironmentFactory(std::move(parameters)) {}
std::shared_ptr<Environment> RobotJumpFactory::get_env(int num_threads, int seed) {
    return std::make_shared<RobotJump>(
        num_threads, seed,
        get_value(
            "skeleton_json_path",
            std::filesystem::path(RESOURCES_PATH) / "./resources/skeleton/spider_new.json"),
        get_value("minimal_velocity", 1e-1f), get_value("target_velocity", 0.5f),
        get_value("max_seconds", 30.f), get_value("initial_seconds", 1.f),
        get_value("reset_seconds", 1.f / 6.f));
}

/*
 * Env list
 */

std::map<
    std::string,
    std::function<std::shared_ptr<EnvironmentFactory>(std::map<std::string, std::string>)>>
    ENV_FACTORY_CONSTRUCTORS = {
        {"cartpole", std::make_shared<CartPoleFactory, std::map<std::string, std::string>>},
        {"cartpole3d", std::make_shared<CartPole3dFactory, std::map<std::string, std::string>>},
        {"robot_walk", std::make_shared<RobotWalkFactory, std::map<std::string, std::string>>},
        {"robot_jump", std::make_shared<RobotJumpFactory, std::map<std::string, std::string>>},
};

std::shared_ptr<EnvironmentFactory> get_environment_factory(
    const std::string &env_name, std::map<std::string, std::string> parameters) {
    if (ENV_FACTORY_CONSTRUCTORS.find(env_name) == ENV_FACTORY_CONSTRUCTORS.end())
        throw std::invalid_argument(env_name);
    return ENV_FACTORY_CONSTRUCTORS[env_name](std::move(parameters));
}