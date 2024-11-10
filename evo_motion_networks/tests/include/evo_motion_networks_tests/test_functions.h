//
// Created by samuel on 10/11/24.
//

#ifndef EVO_MOTION_TEST_FUNCTIONS_H
#define EVO_MOTION_TEST_FUNCTIONS_H

#include <gtest/gtest.h>

class ParamTestRand : public testing::TestWithParam<std::tuple<torch::Tensor, float>> {};

class ParamTestNormal : public testing::TestWithParam<std::vector<int64_t>> {};

class ParamTestTruncNormal
    : public testing::TestWithParam<std::tuple<std::vector<int64_t>, float, float>> {};

#endif//EVO_MOTION_TEST_FUNCTIONS_H
