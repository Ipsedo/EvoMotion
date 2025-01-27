//
// Created by samuel on 10/11/24.
//

#include <gtest/gtest.h>
#include <torch/torch.h>

#include <evo_motion_networks/functions.h>
#include <evo_motion_networks_tests/test_functions.h>

/*
 * Test uniform random
 */

TEST_P(ParamTestRand, TestRandFun) {
    const auto [t, epsilon] = GetParam();

    const torch::Tensor rand = rand_eps(t, epsilon);

    ASSERT_EQ(t.sizes().size(), rand.sizes().size());

    for (int i = 0; i < t.sizes().size(); i++) { ASSERT_EQ(t.size(i), rand.size(i)); }

    ASSERT_TRUE(torch::all(rand >= epsilon).item().toBool());
    ASSERT_TRUE(torch::all(rand <= 1.f - epsilon).item().toBool());
}

INSTANTIATE_TEST_SUITE_P(
    TestRand, ParamTestRand,
    testing::Combine(
        testing::Values(torch::rand({1, 2, 3}), torch::randn({3, 2, 1, 4})),
        testing::Values(0.1f, 0.3f)));

/*
 * Test normal dist
 */

TEST_P(ParamTestNormal, TestPDF) {
    const auto sizes = GetParam();

    const auto x = torch::randn(sizes);
    const auto mu = torch::randn(sizes);
    const auto sigma = torch::randn(sizes).exp();

    const auto out = normal_pdf(x, mu, sigma);

    ASSERT_EQ(sizes.size(), out.sizes().size());

    for (int i = 0; i < sizes.size(); i++) { ASSERT_EQ(sizes[i], out.size(i)); }

    ASSERT_TRUE(torch::all(out >= 0.f).item().toBool());
}

TEST_P(ParamTestNormal, TestCDF) {
    const auto sizes = GetParam();

    const auto x = torch::randn(sizes);
    const auto mu = torch::randn(sizes);
    const auto sigma = torch::randn(sizes).exp();

    const auto out = normal_cdf(x, mu, sigma);

    ASSERT_EQ(sizes.size(), out.sizes().size());

    for (int i = 0; i < sizes.size(); i++) { ASSERT_EQ(sizes[i], out.size(i)); }

    ASSERT_TRUE(torch::all(out >= 0.f).item().toBool());
    ASSERT_TRUE(torch::all(out <= 1.f).item().toBool());
}

INSTANTIATE_TEST_SUITE_P(
    TestNormal, ParamTestNormal,
    testing::Values(
        std::vector<int64_t>{1, 2, 3}, std::vector<int64_t>{9}, std::vector<int64_t>{2, 6}));

/*
 * Truncated Normal dist
 */

// Sample

TEST_P(ParamTestTruncNormal, TestSample) {
    const auto [sizes, min_value, max_value] = GetParam();

    const auto mu = torch::rand(sizes) * (max_value - min_value) + min_value;
    const auto sigma = torch::softplus(torch::rand(sizes) * 60.f - 30.f);

    const auto out = truncated_normal_sample(mu, sigma, min_value, max_value);

    for (int i = 0; i < sizes.size(); i++) { ASSERT_EQ(sizes[i], out.size(i)); }

    ASSERT_TRUE(torch::all(out >= min_value).item().toBool());
    ASSERT_TRUE(torch::all(out <= max_value).item().toBool());

    ASSERT_TRUE(torch::all(torch::logical_not(torch::isnan(out))).item().toBool());
    ASSERT_TRUE(torch::all(torch::logical_not(torch::isinf(out))).item().toBool());
}

// PDF

TEST_P(ParamTestTruncNormal, TestPDF) {
    const auto [sizes, min_value, max_value] = GetParam();

    const auto mu = torch::rand(sizes) * (max_value - min_value) + min_value;
    const auto sigma = torch::softplus(torch::rand(sizes) * 60.f - 30.f);
    const auto x = truncated_normal_sample(mu, sigma, min_value, max_value);

    const auto out = truncated_normal_pdf(x, mu, sigma, min_value, max_value);
    const auto out_log = truncated_normal_log_pdf(x, mu, sigma, min_value, max_value);

    for (int i = 0; i < sizes.size(); i++) {
        ASSERT_EQ(sizes[i], out.size(i));
        ASSERT_EQ(sizes[i], out_log.size(i));
    }

    ASSERT_TRUE(torch::all(out > 0.f).item().toBool());

    ASSERT_TRUE(torch::all(torch::logical_not(torch::isnan(out))).item().toBool());
    ASSERT_TRUE(torch::all(torch::logical_not(torch::isnan(out_log))).item().toBool());

    ASSERT_TRUE(torch::all(torch::logical_not(torch::isinf(out))).item().toBool());
    ASSERT_TRUE(torch::all(torch::logical_not(torch::isinf(out_log))).item().toBool());
}

// Entropy

TEST_P(ParamTestTruncNormal, TestEntropy) {
    const auto [sizes, min_value, max_value] = GetParam();

    const auto mu = torch::rand(sizes) * (max_value - min_value) + min_value;
    const auto sigma = torch::softplus(torch::rand(sizes) * 60.f - 30.f);

    const auto out = truncated_normal_entropy(mu, sigma, min_value, max_value);

    for (int i = 0; i < sizes.size(); i++) { ASSERT_EQ(sizes[i], out.size(i)); }

    ASSERT_TRUE(torch::all(torch::logical_not(torch::isnan(out))).item().toBool());
    ASSERT_TRUE(torch::all(torch::logical_not(torch::isinf(out))).item().toBool());
}

// Create parametrized tests

INSTANTIATE_TEST_SUITE_P(
    TestTruncNormal, ParamTestTruncNormal,
    testing::Combine(
        testing::Values(
            std::vector<int64_t>{1, 2, 3}, std::vector<int64_t>{1000, 1000},
            std::vector<int64_t>{6}),
        testing::Values(-2.f, -1.f, -0.1f), testing::Values(0.1f, 1.f, 2.f)));