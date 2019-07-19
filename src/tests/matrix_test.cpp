//
// Created by samuel on 20/07/19.
//

#include <iostream>
#include "matrix_test.h"
#include "../algebra/matrix.h"

void test_matrix() {
    int n = 10;
    algebra::matrix m = algebra::identity(n);

    std::vector<double> res(n);

    for (int i = 0; i < n; i++)
        res[i] = i;

    auto res_2 = algebra::dot(res, m);

    for (int i = 0; i < n; i++)
        std::cout << res_2[i] << ", ";
    std::cout << std::endl;
}
