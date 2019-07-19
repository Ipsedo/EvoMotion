//
// Created by samuel on 19/07/19.
//

#include "CMA_ES.h"

#include <algorithm>
#include <chrono>
#include "../algebra/operators.h"
#include "../algebra/functions.h"

using algebra::operator*;
using algebra::operator+;

CMA_ES::CMA_ES(coco_problem_s *p) :
    __generator((unsigned long)(std::chrono::duration_cast<std::chrono::milliseconds>
            (std::chrono::system_clock::now().time_since_epoch()).count())),
    __normal_dist(0., 1.), __uniform_dist(0., 1.),
    __problem(p),
    __min_values(coco_problem_get_smallest_values_of_interest(p)),
    __max_values(coco_problem_get_largest_values_of_interest(p)),
    __n(static_cast<int>(coco_problem_get_dimension(p))), __lambda(10), __mu(__lambda / 2),
    __x(__init_x()),
    __w(__init_w(vector<individual>())), //TODO piger algo : log rank - w_k
    __mu_w(1.0 / algebra::sum(__w  * __w)),
    __c_sigma(__mu_w / (double(__n) + __mu_w)),
    __d(1.0 + sqrt(__mu_w / double(__n))),
    __c_c((4.0 + __mu_w / double(__n)) / (double(__n) + 4 + 2 * __mu_w / double(__n))),
    __c_1(2.0 / (double(__n * __n) + __mu_w)),
    __c_mu(__mu_w / (double(__n * __n) + __mu_w)),
    __c_m(1.0),
    __s_sigma(vector<double>(static_cast<unsigned long>(__n), 0.0)),
    __s_c(vector<double>(static_cast<unsigned long>(__n), 0.0)),
    __C(algebra::identity(__n)),
    __sigma(algebra::rand(__uniform_dist, __generator, __n, 1e-5, 2.0)) {

}

individual CMA_ES::__init_x() {
    vector<double> res(static_cast<unsigned long>(__n));

    for (int i = 0; i < __n; i++)
        res[i] = __uniform_dist(__generator) * (__max_values[i] - __min_values[i]) + __min_values[i];

    double fitness;
    coco_evaluate_function(__problem, res.data(), &fitness);

    return {res, fitness};
}

vector<double> CMA_ES::__init_w(std::vector<individual> pop) {
    vector<double> w_prime(static_cast<unsigned long>(__mu));

    for (int k = 0; k < __mu; k++)
        w_prime[k] = log(static_cast<double>(__lambda) / 2.0 + 0.5) - log(pop[k].fitness); // log rank(f(x_k)) ???

    vector<double> w(static_cast<unsigned long>(__mu));

    double sum = algebra::sum(w_prime);

    for (int k = 0; k < __mu; k++)
        w[k] = w_prime[k] / sum;

    return w;
}

void CMA_ES::step() {
    vector<algebra::matrix> z(static_cast<unsigned long>(__lambda));
    vector<individual> childs(static_cast<unsigned long>(__lambda));

    for (int k = 0; k < __lambda; k++) {
        z[k] = algebra::normal_identity(__normal_dist, __n);
        childs[k].geno = __x.geno + 0.0; //TODO compute square root matrix :,(

        coco_evaluate_function(__problem, childs[k].geno.data(), &childs[k].fitness);
    }

    sort(childs.begin(), childs.end(), [](individual i1, individual i2){ return i1.fitness < i2.fitness; });
    vector<individual> pop(&childs[0], &childs[__mu]);

    __w = __init_w(pop);
    __x.geno = algebra::dot(__sigma, algebra::sum(z * __w) *__C * __c_m * __sigma + __x.geno); //TODO compute square root matrix :,(

}
