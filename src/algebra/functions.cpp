//
// Created by samuel on 19/07/19.
//

#include <algorithm>
#include "functions.h"

double algebra::sum(std::vector<double> x) {
    double sum = 0.0;
    std::for_each(x.begin(), x.end(), [&sum](double v){sum += v; });
    return sum;
}

std::vector<double>
algebra::rand(std::uniform_real_distribution<double> dist, std::default_random_engine generator, int n, double min, double max) {
    double range = max - min;

    std::vector<double> res(n);

    for (int i = 0; i < n; i++)
        res[i] = dist(generator) * range + min;

    return res;
}
