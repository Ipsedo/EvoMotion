//
// Created by samuel on 10/11/24.
//

#ifndef EVO_MOTION_TEST_METRICS_H
#define EVO_MOTION_TEST_METRICS_H

#include <gtest/gtest.h>

class ParamLossMetric : public testing::TestWithParam<std::tuple<std::vector<float>, int, float>> {
};

#endif//EVO_MOTION_TEST_METRICS_H
