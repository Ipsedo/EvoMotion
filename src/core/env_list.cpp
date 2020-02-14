//
// Created by samuel on 14/02/20.
//

#include "env_list.h"
#include "envs/cartpole.h"
#include "envs/pendulum.h"
#include "envs/hunt.h"

EnvEnum EnvEnum::CONTINUOUS_CARTPOLE() {
    return EnvEnum("ContinuousCartpole",
                    []() { return new ContinuousCartPoleEnv(static_cast<int>(time(nullptr))); });
}

EnvEnum EnvEnum::PENDULUM() {
    return EnvEnum("Pendulum",
                    []() { return new PendulumEnv(static_cast<int>(time(nullptr))); });
}

EnvEnum EnvEnum::DISCRETE_CARTPOLE() {
    return EnvEnum("DiscreteCartpole",
                    []() { return new DiscreteCartPoleEnv(static_cast<int>(time(nullptr))); });
}

EnvEnum EnvEnum::HUNTING() {
    return EnvEnum("Hunting", []() { return new HuntingEnv(static_cast<int>(time(nullptr))); });
}

EnvEnum::EnvEnum(std::string name, std::function<Environment *()> get_env_fun)
: name(std::move(name)), get_env_fun(std::move(get_env_fun)) {}

std::string EnvEnum::get_name() const { return name; }

Environment *EnvEnum::get_env() const { return get_env_fun(); }

EnvEnum EnvEnum::from_str(std::string env_value) {
    auto env_list = { EnvEnum::CONTINUOUS_CARTPOLE(), EnvEnum::DISCRETE_CARTPOLE(), EnvEnum::PENDULUM(), EnvEnum::HUNTING() };

    for (auto env : env_list)
        if (env.get_name() == env_value)
            return env;

    std::cerr << "Environment (" << env_value << ") not recognized !" << std::endl;
    return EnvEnum::DISCRETE_CARTPOLE();
}
