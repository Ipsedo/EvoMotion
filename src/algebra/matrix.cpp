//
// Created by samuel on 19/07/19.
//

#include "matrix.h"
#include "functions.h"
#include "operators.h"


algebra::matrix algebra::identity(int n) {
    algebra::matrix id(static_cast<unsigned long>(n));

    for (int i = 0; i < n; i++) {
        std::vector<double> line(static_cast<unsigned long>(n), 0.0);
        line[i] = 1.0;

        id[i] = line;
    }

    return id;
}

std::vector<double> algebra::dot(std::vector<double> v, algebra::matrix m) {
    std::vector<double> res(m[0].size());

    for (int i = 0; i < m[0].size(); i++)
        for (int j = 0; j < v.size(); j++)
            res[i] += v[j] * m[j][i];
    return res;
}

algebra::matrix algebra::init(int n, double value) {
    return algebra::matrix(n, std::vector<double>(n, value));
}

algebra::matrix algebra::normal_identity(std::normal_distribution<double> dist, int n) {
    return algebra::matrix(); //TODO
}

algebra::matrix algebra::sum(std::vector<algebra::matrix> m) {
    return algebra::matrix(); //TODO
}
