//
// Created by samuel on 19/07/19.
//

#ifndef EVOMOTION_CMA_ES_H
#define EVOMOTION_CMA_ES_H

#include <vector>
#include <random>
#include "../coco.h"
#include "individual.h"
#include "../algebra/matrix.h"

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

    int __n;

    int __lambda;
    int __mu;

    individual __x;

    vector<double> __w;

    double __mu_w;
    double __c_sigma;
    double __d;
    double __c_c;
    double __c_1;
    double __c_mu;
    double __c_m;

    vector<double> __s_sigma;
    vector<double> __s_c;
    algebra::matrix __C;
    vector<double> __sigma;

    individual __init_x();

    vector<double> __init_w(std::vector<individual> pop);

public:
    explicit CMA_ES(coco_problem_s *p);

    void step();
};


#endif //EVOMOTION_CMA_ES_H
