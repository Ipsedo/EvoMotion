//
// Created by samuel on 19/07/19.
//

#include "CMA_ES.h"

#include <algorithm>
#include <chrono>

CMA_ES::CMA_ES(coco_problem_s *p) :
    __generator((unsigned long)(std::chrono::duration_cast<std::chrono::milliseconds>
            (std::chrono::system_clock::now().time_since_epoch()).count())),
    __normal_dist(0., 1.), __uniform_dist(0., 1.),
    __problem(p),
    __min_values(coco_problem_get_smallest_values_of_interest(p)),
    __max_values(coco_problem_get_largest_values_of_interest(p)),
    __n(static_cast<unsigned int>(coco_problem_get_dimension(p))), __lambda(10), __mu(__lambda / 2),
    __x(__init_x()),
    /*__w(__init_w(vector<individual>())), //TODO piger algo : log rank - w_k
    __mu_w(1.0 / arma::sum(__w  * __w)),
    __c_sigma(__mu_w / (double(__n) + __mu_w)),
    __d(1.0 + sqrt(__mu_w / double(__n))),
    __c_c((4.0 + __mu_w / double(__n)) / (double(__n) + 4 + 2 * __mu_w / double(__n))),
    __c_1(2.0 / (double(__n * __n) + __mu_w)),
    __c_mu(__mu_w / (double(__n * __n) + __mu_w)),
    __c_m(1.0),*/
    __s_sigma(arma::vec(__n, arma::fill::zeros)),
    __s_c(arma::vec(__n, arma::fill::zeros)),
    __C(arma::mat(__n, __n, arma::fill::eye)),
    __sigma(arma::randu<arma::vec>(__n)) {

}

individual CMA_ES::__init_x() {
    arma::vec res(__n, arma::fill::zeros);

    for (int i = 0; i < __n; i++)
        res[i] = __uniform_dist(__generator) * (__max_values[i] - __min_values[i]) + __min_values[i];

    double fitness;
    coco_evaluate_function(__problem, res.memptr(), &fitness);

    return {res, arma::randn<arma::vec>(__n), fitness};
}

arma::vec CMA_ES::__init_w(std::vector<individual> pop) {
    arma::vec w(arma::uword(__mu), arma::fill::zeros);
    arma::vec w_prime(arma::uword(__mu), arma::fill::zeros);

    for (int k = 0; k < __mu; k++)
        w_prime[k] = log(double(__lambda) / 2.0 + 0.5) - log(pop[k].fitness); // log rank(f(x_k)) ???

    double sum = arma::sum(w_prime);

    w = w_prime / sum;

    return w;
}

void CMA_ES::__init_values(std::vector<individual> pop) {
    __w = __init_w(std::move(pop));
    __mu_w = 1.0 / arma::sum(__w  % __w);
    __c_sigma = __mu_w / (double(__n) + __mu_w);
    __d = 1.0 + sqrt(__mu_w / double(__n));
    __c_c = (4.0 + __mu_w / double(__n)) / (double(__n) + 4 + 2 * __mu_w / double(__n));
    __c_1 = 2.0 / (double(__n * __n) + __mu_w);
    __c_mu = __mu_w / (double(__n * __n) + __mu_w);
    __c_m = 1.0;
}

void CMA_ES::step() {
    vector<individual> childs(static_cast<unsigned long>(__lambda));

    //std::cout << __C << std::endl;
    arma::mat C_sqrt = arma::sqrtmat_sympd(__C);

    for (int k = 0; k < __lambda; k++) {
        childs[k].z = arma::randn(arma::uword(__n));

        childs[k].geno = __x.geno + C_sqrt * childs[k].z % __sigma;

        coco_evaluate_function(__problem, childs[k].geno.memptr(), &childs[k].fitness);
    }

    sort(childs.begin(), childs.end(), [](individual i1, individual i2){ return i1.fitness < i2.fitness; });
    vector<individual> pop(&childs[0], &childs[__mu]);

    //if (__started) {
        __init_values(pop);
    //    __started = true;
    //}

    arma::vec sum_z(__n, arma::fill::zeros);

    for (int k = 0; k < pop.size(); k++)
        sum_z += __w[k] * pop[k].z;

    __x.geno += __c_m * (C_sqrt * __sigma) % sum_z;

    __s_sigma = ( - __c_sigma) * __s_sigma + sqrt(__c_sigma * (2.0 - __c_sigma)) * sqrt(__mu_w) * sum_z;

    double h_sigma = pow(arma::norm(__s_sigma), 2.0) / double(__n) < 2.0 + 4.0 / (double(__n) + 1) ? 1.0 : 0.0;

    arma::vec sum_c_z(__n, arma::fill::zeros);
    for (int k = 0; k < pop.size(); k++)
        sum_c_z += __w[k] * C_sqrt * pop[k].z;

    __s_c = (1 - __c_c) * __s_c + h_sigma * sqrt(__c_c * (2.0 - __c_c)) * sqrt(__mu_w) * sum_c_z;

    __sigma *= pow((arma::norm(__s_sigma) / double(__n) - 1.0), ((__c_sigma / __d) / 2.0));

    double c_h = __c_1 * (1.0 - h_sigma * h_sigma) * __c_c * (2.0 - __c_c);
    arma::mat sum_c(__n, __n, arma::fill::zeros);
    for (int k = 0; k < pop.size(); k++)
        sum_c += __w[k] * C_sqrt * pop[k].z * (C_sqrt * pop[k].z).t();

    __C = (1.0 - __c_1 + c_h - __c_mu) * __C + __c_1 * __s_c * __s_c.t() + __c_mu * sum_c;
}
