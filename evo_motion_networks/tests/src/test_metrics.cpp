//
// Created by samuel on 10/11/24.
//

#include <evo_motion_networks/metrics.h>
#include <evo_motion_networks_tests/test_metrics.h>

TEST_P(ParamLossMetric, TestLossMetric) {
    const auto values = std::get<0>(GetParam());
    const auto window_size = std::get<1>(GetParam());
    const auto expected_value = std::get<2>(GetParam());

    LossMeter meter("test", window_size);

    for (auto v: values) meter.add(v);

    ASSERT_EQ(meter.loss(), expected_value);
}

// Create parametrized tests

INSTANTIATE_TEST_SUITE_P(
    TestLossMetric, ParamLossMetric,
    testing::Values(
        std::make_tuple(std::vector<float>{1.f, 2.f, 1.f, 2.f}, 4, 1.5f),
        std::make_tuple(std::vector<float>{1.f, 2.f, 1.f, 2.f}, 2, 1.5f),
        std::make_tuple(std::vector<float>{1.f, 1.f, 2.f, 2.f}, 2, 2.f)));