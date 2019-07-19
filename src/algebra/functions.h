//
// Created by samuel on 19/07/19.
//

#ifndef EVOMOTION_BASIC_FUNCTION_H
#define EVOMOTION_BASIC_FUNCTION_H

#include <vector>
#include <random>

namespace algebra {

    std::vector<double>
    rand(std::uniform_real_distribution<double> dist, std::default_random_engine generator, int n, double min, double max);

    double sum(std::vector<double> x);

}

#endif //EVOMOTION_BASIC_FUNCTION_H
