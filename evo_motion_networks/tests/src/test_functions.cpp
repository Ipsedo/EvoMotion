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
    torch::Tensor t = std::get<0>(GetParam());
    float epsilon = std::get<1>(GetParam());

    torch::Tensor rand = rand_eps(t, epsilon);

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

// PDF Normal

TEST_P(ParamTestNormal, TestNormalPDFFun) {
    const auto sizes = GetParam();

    const auto x = torch::randn(sizes);
    const auto mu = torch::randn(sizes);
    const auto sigma = torch::randn(sizes).exp();

    const auto out = normal_pdf(x, mu, sigma);

    ASSERT_EQ(sizes.size(), out.sizes().size());

    for (int i = 0; i < sizes.size(); i++) { ASSERT_EQ(sizes[i], out.size(i)); }

    ASSERT_TRUE(torch::all(out >= 0.f).item().toBool());
}

INSTANTIATE_TEST_SUITE_P(
    TestNormalPDF, ParamTestNormal,
    testing::Values(
        std::vector<int64_t>{1, 2, 3}, std::vector<int64_t>{9}, std::vector<int64_t>{2, 6}));

// CDF Normal

TEST_P(ParamTestNormal, TestNormalCDFFun) {
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
    TestNormalCDF, ParamTestNormal,
    testing::Values(
        std::vector<int64_t>{1, 2, 3}, std::vector<int64_t>{9}, std::vector<int64_t>{2, 6}));

/*
 * Truncated Normal dist
 */

// Sample

TEST_P(ParamTestTruncNormal, TestTruncNormalSampleFun) {
    const auto sizes = std::get<0>(GetParam());
    const float min_value = std::get<1>(GetParam());
    const float max_value = std::get<2>(GetParam());

    const auto mu = torch::rand(sizes) * (max_value - min_value) + min_value;
    const auto sigma = torch::randn(sizes).exp();

    const auto out = truncated_normal_sample(mu, sigma, min_value, max_value);

    for (int i = 0; i < sizes.size(); i++) { ASSERT_EQ(sizes[i], out.size(i)); }

    ASSERT_TRUE(torch::all(out >= min_value).item().toBool());
    ASSERT_TRUE(torch::all(out <= max_value).item().toBool());
}

INSTANTIATE_TEST_SUITE_P(
    TestTruncNormalSample, ParamTestTruncNormal,
    testing::Combine(
        testing::Values(
            std::vector<int64_t>{1, 2, 3}, std::vector<int64_t>{9}, std::vector<int64_t>{2, 6}),
        testing::Values(-2.f, -1.f, -0.1f), testing::Values(0.1f, 1.f, 2.f)));

// PDF

TEST_P(ParamTestTruncNormal, TestTruncNormalPDFFun) {
    const auto sizes = std::get<0>(GetParam());
    const float min_value = std::get<1>(GetParam());
    const float max_value = std::get<2>(GetParam());

    const auto x = torch::rand(sizes) * (max_value - min_value) + min_value;
    const auto mu = torch::rand(sizes) * (max_value - min_value) + min_value;
    const auto sigma = torch::randn(sizes).exp();

    const auto out = truncated_normal_pdf(x, mu, sigma, min_value, max_value);

    for (int i = 0; i < sizes.size(); i++) { ASSERT_EQ(sizes[i], out.size(i)); }

    ASSERT_TRUE(torch::all(out >= 0.f).item().toBool());
}

INSTANTIATE_TEST_SUITE_P(
    TestTruncNormalPDF, ParamTestTruncNormal,
    testing::Combine(
        testing::Values(
            std::vector<int64_t>{1, 2, 3}, std::vector<int64_t>{9}, std::vector<int64_t>{2, 6}),
        testing::Values(-2.f, -1.f, -0.1f), testing::Values(0.1f, 1.f, 2.f)));

// Entropy

TEST_P(ParamTestTruncNormal, TestTruncNormalEntropyFun) {
    const auto sizes = std::get<0>(GetParam());
    const float min_value = std::get<1>(GetParam());
    const float max_value = std::get<2>(GetParam());

    const auto mu = torch::rand(sizes) * (max_value - min_value) + min_value;
    const auto sigma = torch::randn(sizes).exp();

    const auto out = truncated_normal_entropy(mu, sigma, min_value, max_value);

    for (int i = 0; i < sizes.size(); i++) { ASSERT_EQ(sizes[i], out.size(i)); }
}

INSTANTIATE_TEST_SUITE_P(
    TestTruncNormalEntropy, ParamTestTruncNormal,
    testing::Combine(
        testing::Values(
            std::vector<int64_t>{1, 2, 3}, std::vector<int64_t>{9}, std::vector<int64_t>{2, 6}),
        testing::Values(-2.f, -1.f, -0.1f), testing::Values(0.1f, 1.f, 2.f)));