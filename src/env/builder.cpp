//
// Created by samuel on 25/12/22.
//

#include "builder.h"

#include <utility>
#include "cartpole.h"
#include "cartpole3d.h"


std::shared_ptr<Environment> EnvBuilder::get() {
    return constructors[env_name](seed);
}

std::vector<std::string> EnvBuilder::env_names() {
    std::vector<std::string> names;

    for (auto [name, constructor]: constructors)
        names.push_back(name);

    return names;
}

EnvBuilder::EnvBuilder(int seed, std::string env_name) :
        seed(seed),
        env_name(std::move(env_name)),
        constructors(
                {
                        {"cartpole",   [](int seed) { return std::make_shared<CartPole>(seed); }},
                        {"cartpole3d", [](int seed) { return std::make_shared<CartPole3d>(seed); }}
                }
        ) {

}

