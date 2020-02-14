//
// Created by samuel on 11/08/19.
//

#ifndef EVOMOTION_ENV_LIST_H
#define EVOMOTION_ENV_LIST_H

// group all environment

#include "environment.h"

class EnvEnum {
public:
    static EnvEnum CONTINUOUS_CARTPOLE();
    static EnvEnum PENDULUM();
    static EnvEnum DISCRETE_CARTPOLE();
    static EnvEnum HUNTING();


private:
    const std::string name;
    const std::function<Environment *()> get_env_fun;
    EnvEnum(std::string name, std::function<Environment *()> get_env_fun);

public:

    std::string get_name() const ;
    Environment * get_env() const ;

    static EnvEnum from_str(std::string env_value);

};

#endif //EVOMOTION_ENV_LIST_H
