//
// Created by samuel on 25/12/22.
//

#include <utility>

#include <evo_motion_model/env_builder.h>

#include "./env_test_muscle.h"
#include "cartpole.h"
#include "cartpole3d.h"


std::shared_ptr<Environment> EnvBuilder::get() { return constructors[env_name](seed); }

EnvBuilder::EnvBuilder(int seed, std::string env_name)
    : seed(seed), env_name(std::move(env_name)),
      constructors(
          {{"cartpole", std::make_shared<CartPole, int>},
           {"cartpole3d", std::make_shared<CartPole3d, int>},
           {"test_muscle", std::make_shared<MuscleEnv, int>}}) {}
