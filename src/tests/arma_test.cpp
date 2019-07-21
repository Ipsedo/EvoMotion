//
// Created by samuel on 21/07/19.
//

#include "arma_test.h"

#include <armadillo>
#include <iostream>

void test_armadillo() {
    arma::mat m1(5, 5, arma::fill::eye);
    arma::mat m2(5, 5, arma::fill::eye);
    arma::vec v(5, arma::fill::ones);
    v = v * 5.0;

    v = v + arma::sqrtmat_sympd(m1) * m2 * v;

    std::cout << v << std::endl;
    std::cout << v.size() << std::endl;
    auto a = v * m1;
    std::cout << v * a.t() << std::endl;
}
