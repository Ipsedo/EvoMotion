//
// Created by samuel on 19/07/19.
//

#include "vector_test.h"
#include "../algebra/operators.h"

using namespace std;

using algebra::operator+;
using algebra::operator*;

void tests_vector_operator() {
    vector<vector<float>> v1;
    vector<vector<float>> v2;

    int size = 10;

    for (int i = 0; i < size; i++) {
        vector<float> tmp1;
        vector<float> tmp2;

        for (int j = 0; j < size; j++) {
            tmp1.push_back(2.f);
            tmp2.push_back(i * size + j);
        }

        v1.push_back(tmp1);
        v2.push_back(tmp2);
    }

    vector<vector<float>> res = v1 * v2;
    for (int i = 0; i < res.size(); i++) {
        for (int j = 0; j < res[i].size(); j++)
            cout << res[i][j] << ", ";
        cout << endl;
    }
    cout << endl;

    res = v1 * 10.f;
    for (int i = 0; i < res.size(); i++) {
        for (int j = 0; j < res[i].size(); j++)
            cout << res[i][j] << ", ";
        cout << endl;
    }
    cout << endl;

    vector<float> v3;
    for (int i = 0; i < size; i++)
        v3.push_back(i + 1.f);
    res = v2 * v3 + 0.5f;
    for (int i = 0; i < res.size(); i++) {
        for (int j = 0; j < res[i].size(); j++)
            cout << res[i][j] << ", ";
        cout << endl;
    }
}

