//
// Created by samuel on 16/02/20.
//

#ifndef EVOMOTION_ENVENUM_H
#define EVOMOTION_ENVENUM_H

#include <string>

#include "environment.h"

class EnvEnum {

private:
    const std::string name;
    const std::function<Environment *()> get_env_fun;
    EnvEnum(std::string name, std::function<Environment *()> get_env_fun);

public:

    std::string get_name() const ;
    Environment * get_env() const ;

    static EnvEnum from_str(std::string env_value);

    static std::vector<EnvEnum> get_values();
    static std::vector<std::string> get_names();

public:
    static const EnvEnum CONTINUOUS_CARTPOLE;
    static const EnvEnum PENDULUM;
    static const EnvEnum DISCRETE_CARTPOLE;
    static const EnvEnum HUNTING;

};


#endif //EVOMOTION_ENVENUM_H
