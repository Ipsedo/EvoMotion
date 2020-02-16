//
// Created by samuel on 16/02/20.
//

#include "env_enum.h"
#include "envs/cartpole.h"
#include "envs/pendulum.h"
#include "envs/hunt.h"

EnvEnum::EnvEnum(std::string name, std::function<Environment *()> get_env_fun)
        : name(std::move(name)), get_env_fun(std::move(get_env_fun)) {}

std::string EnvEnum::get_name() const { return name; }

Environment *EnvEnum::get_env() const { return get_env_fun(); }

EnvEnum EnvEnum::from_str(std::string env_value) {
    auto env_list = EnvEnum::get_values();

    for (auto env : env_list)
        if (env.get_name() == env_value)
            return env;

    std::cerr << "Environment (" << env_value << ") not recognized !" << std::endl;
    return EnvEnum::DISCRETE_CARTPOLE;
}

std::vector<EnvEnum> EnvEnum::get_values() {
    return { EnvEnum::CONTINUOUS_CARTPOLE, EnvEnum::DISCRETE_CARTPOLE, EnvEnum::PENDULUM, EnvEnum::HUNTING };
}

std::vector<std::string> EnvEnum::get_names() {
    auto envs = EnvEnum::get_values();
    std::vector<std::string> env_names(envs.size());

    std::transform(envs.begin(), envs.end(), env_names.begin(), [](EnvEnum e){ return e.get_name(); });

    return env_names;
}

const EnvEnum EnvEnum::CONTINUOUS_CARTPOLE
        = EnvEnum("ContinuousCartpole",
                  []() { return new ContinuousCartPoleEnv(static_cast<long>(time(nullptr))); });

const EnvEnum EnvEnum::DISCRETE_CARTPOLE
        = EnvEnum("DiscreteCartpole",
                  []() { return new DiscreteCartPoleEnv(static_cast<long>(time(nullptr))); });

const EnvEnum EnvEnum::PENDULUM
        = EnvEnum("Pendulum",
                  []() { return new PendulumEnv(static_cast<long>(time(nullptr))); });

const EnvEnum EnvEnum::HUNTING
        = EnvEnum("Hunting",
                  []() { return new HuntingEnv(static_cast<long>(time(nullptr))); });