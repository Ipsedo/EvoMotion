//
// Created by samuel on 19/07/19.
//

#ifndef EVOMOTION_MATRIX_H
#define EVOMOTION_MATRIX_H

#include <vector>
#include <random>

namespace algebra {

    typedef std::vector<std::vector<double>> matrix;

    matrix identity(int n);

    matrix init(int n, double value);

    matrix normal_identity(std::normal_distribution<double> dist, int n);

    matrix sum(std::vector<matrix> m);

    std::vector<double> dot(std::vector<double> v, matrix m);

}

#endif //EVOMOTION_MATRIX_H
