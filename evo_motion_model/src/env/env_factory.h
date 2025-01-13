//
// Created by samuel on 09/11/24.
//

#ifndef EVO_MOTION_ENV_FACTORY_H
#define EVO_MOTION_ENV_FACTORY_H

#include <evo_motion_model/environment.h>

class CartPoleFactory : public EnvironmentFactory {
public:
    explicit CartPoleFactory(std::map<std::string, std::string> parameters);

    std::shared_ptr<Environment> get_env(int num_threads, int seed) override;
};

class CartPole3dFactory : public EnvironmentFactory {
public:
    explicit CartPole3dFactory(std::map<std::string, std::string> parameters);

    std::shared_ptr<Environment> get_env(int num_threads, int seed) override;
};

class RobotWalkFactory : public EnvironmentFactory {
public:
    explicit RobotWalkFactory(std::map<std::string, std::string> parameters);

    std::shared_ptr<Environment> get_env(int num_threads, int seed) override;
};

class RobotJumpFactory : public EnvironmentFactory {
public:
    explicit RobotJumpFactory(std::map<std::string, std::string> parameters);

    std::shared_ptr<Environment> get_env(int num_threads, int seed) override;
};

#endif//EVO_MOTION_ENV_FACTORY_H
