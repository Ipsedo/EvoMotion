//
// Created by samuel on 25/12/22.
//

#ifndef EVO_MOTION_BUILDER_H
#define EVO_MOTION_BUILDER_H

#include <functional>
#include <memory>

#include "../model/environment.h"

class EnvBuilder {
public:
    std::shared_ptr<Environment> get();
    explicit EnvBuilder(int seed, std::string env_name);

private:
    int seed;
    std::string env_name;
    std::map<std::string, std::function<std::shared_ptr<Environment>(int)>> constructors;
};

#endif //EVO_MOTION_BUILDER_H
