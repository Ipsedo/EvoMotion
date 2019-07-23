//
// Created by samuel on 19/07/19.
//

#ifndef EVOMOTION_INDIVIDUAL_H
#define EVOMOTION_INDIVIDUAL_H

#define ARMA_DONT_PRINT_ERRORS
#include <armadillo>

struct individual {
    arma::vec geno;
    arma::vec z;
    double fitness;
};

#endif //EVOMOTION_INDIVIDUAL_H
