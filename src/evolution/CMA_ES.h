//
// Created by samuel on 19/07/19.
//

#ifndef EVOMOTION_CMA_ES_H
#define EVOMOTION_CMA_ES_H

#include <vector>
#include <random>
#include "individual.h"
#include "../coco.h"

using namespace std;

class CMA_ES {
private:
    /*
     * std::random stuff
     */
    normal_distribution<double> __normal_dist;
    uniform_real_distribution<double> __uniform_dist;
    default_random_engine __generator;

    /*
     * COCO stuff
     */
    coco_problem_t *__problem;
    const double *__min_values;
    const double *__max_values;

    /*
     * CMA-ES stuff
     */

    unsigned int __n;

    int __lambda;
    int __mu;

    individual __x;

    arma::vec __w;

    double __mu_w;
    double __c_sigma;
    double __d;
    double __c_c;
    double __c_1;
    double __c_mu;
    double __c_m;

    arma::vec __s_sigma;
    arma::vec __s_c;
    arma::mat __C;
    double __sigma;

    int __step;

    /*
     * Init methods
     */

    individual __init_x();
    arma::vec __init_w();

public:
    explicit CMA_ES(coco_problem_s *p);

    void step();
};


#endif //EVOMOTION_CMA_ES_H
