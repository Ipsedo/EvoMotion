//
// Created by samuel on 19/07/19.
//

#include "CMA_ES.h"

#include <vector>
#include <algorithm>
#include <chrono>

// https://en.wikipedia.org/wiki/CMA-ES
// https://hal.inria.fr/hal-01155533/document

CMA_ES::CMA_ES(coco_problem_s *p) :
    __generator((unsigned long)(std::chrono::duration_cast<std::chrono::milliseconds>
            (std::chrono::system_clock::now().time_since_epoch()).count())),
    __normal_dist(0., 1.), __uniform_dist(0., 1.),
    __problem(p),
    __step(0),
    __min_values(coco_problem_get_smallest_values_of_interest(p)),
    __max_values(coco_problem_get_largest_values_of_interest(p)),
    __n((unsigned int)(coco_problem_get_dimension(p))), __lambda(100), __mu(__lambda / 2),
    __x(__init_x()),
    __w(__init_w()),
    __mu_w(1.0 / arma::sum(__w  % __w)),
    __c_sigma(__mu_w / (double(__n) + __mu_w)), // papier inria
    //__c_sigma(3.0 / double(__n)), // wiki
    __d(1.0 + sqrt(__mu_w / double(__n))),
    __c_c((4.0 + __mu_w / double(__n)) / (double(__n) + 4 + 2 * __mu_w / double(__n))),
    __c_1(2.0 / (double(__n * __n) + __mu_w)), // papier inria
    //__c_1(2.0 / double(__n * __n)), // wiki
    __c_mu(__mu_w / (double(__n * __n) + __mu_w)), // papier inria
    //__c_mu(__mu_w / (double(__n * __n))), // wiki
    __c_m(1.0),
    __s_sigma(arma::vec(__n, arma::fill::zeros)),
    __s_c(arma::vec(__n, arma::fill::zeros)),
    __C(arma::mat(__n, __n, arma::fill::eye)),
    __sigma(3.0) {
}

individual CMA_ES::__init_x() {
    arma::vec res(__n, arma::fill::zeros);

    for (int i = 0; i < __n; i++)
        res[i] = __uniform_dist(__generator) * (__max_values[i] - __min_values[i]) + __min_values[i];

    double fitness;
    coco_evaluate_function(__problem, res.memptr(), &fitness);

    return {res, arma::randn<arma::vec>(__n), fitness};
}

arma::vec CMA_ES::__init_w() {
    arma::vec w(__mu, arma::fill::zeros);
    arma::vec w_prime(__mu, arma::fill::zeros);

    w_prime = log(double(__lambda) / 2.0 + 0.5) - arma::log(arma::linspace(1.0, __mu, __mu));

    double sum = arma::sum(w_prime);

    w = w_prime / sum;

    return w;
}

bool CMA_ES::step() {
    std::vector<individual> childs(static_cast<unsigned long>(__lambda));

    arma::mat C_sqrt;

    if (!arma::sqrtmat_sympd(C_sqrt, __C)) // TODO vrai check limites numeriques
        return false;

    for (int k = 0; k < __lambda; k++) {
        childs[k].z = arma::randn<arma::vec>(__n);

        childs[k].geno = __x.geno + __sigma * C_sqrt * childs[k].z;

        coco_evaluate_function(__problem, childs[k].geno.memptr(), &childs[k].fitness);
    }

    sort(childs.begin(), childs.end(), [](individual i1, individual i2){ return i1.fitness < i2.fitness; });
    std::vector<individual> pop(&childs[0], &childs[__mu]);

    arma::vec sum_z(__n, arma::fill::zeros);

    for (int k = 0; k < pop.size(); k++)
        sum_z += __w[k] * pop[k].z;

    __x.geno += __c_m * __sigma * C_sqrt * sum_z;

    __s_sigma = ( - __c_sigma) * __s_sigma + sqrt(__c_sigma * (2.0 - __c_sigma)) * sqrt(__mu_w) * sum_z;

    double h_sigma = pow(arma::norm(__s_sigma), 2.0) / double(__n) < 2.0 + 4.0 / (double(__n) + 1) ? 1.0 : 0.0;

    arma::vec sum_c_z(__n, arma::fill::zeros);
    for (int k = 0; k < pop.size(); k++)
        sum_c_z += __w[k] * C_sqrt * pop[k].z;

    __s_c = (1 - __c_c) * __s_c + h_sigma * sqrt(__c_c * (2.0 - __c_c)) * sqrt(__mu_w) * sum_c_z;

    __sigma *= exp(((__c_sigma / __d) / 2.0)
            * (pow(arma::norm(__s_sigma), 2.0) /
            double(__n) - 1.0)); // inria
            /*(sqrt(double(__n)) * (1.0 - 1.0 / (4.0 * double(__n))
            + 1.0 / (21.0 * pow(double(__n), 2.0)))) - 1.0)); // wiki */


    double c_h = __c_1 * (1.0 - h_sigma * h_sigma) * __c_c * (2.0 - __c_c);
    arma::mat sum_c(__n, __n, arma::fill::zeros);
    for (int k = 0; k < pop.size(); k++)
        sum_c += __w[k] * C_sqrt * pop[k].z * (C_sqrt * pop[k].z).t();

    __C = (1.0 - __c_1 + c_h - __c_mu) * __C + __c_1 * __s_c * __s_c.t() + __c_mu * sum_c;

    __step++;

    return true;
}
