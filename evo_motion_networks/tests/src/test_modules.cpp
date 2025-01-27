//
// Created by samuel on 10/01/25.
//

#include <evo_motion_networks/networks/norm.h>
#include <evo_motion_networks_tests/test_modules.h>

// Batch renormalization test

TEST_P(BatchRenormalizationParam, TestBatchRenormModule) {
    const auto [num_features, batch_size, affine, warmup_steps] = GetParam();

    auto brn = BatchRenormalization(num_features, 1e-5, 0.01, affine, warmup_steps);
    brn.train(true);

    const auto x = torch::randn({batch_size, num_features});

    const auto out = brn.forward(x);

    ASSERT_EQ(out.sizes().size(), 2);
    ASSERT_EQ(out.size(0), batch_size);
    ASSERT_EQ(out.size(1), num_features);

    brn.train(false);

    const auto new_out = brn.forward(x);
    ASSERT_EQ(new_out.sizes().size(), 2);
    ASSERT_EQ(new_out.size(0), batch_size);
    ASSERT_EQ(new_out.size(1), num_features);

    const auto x_single = torch::randn({1, num_features});
    const auto out_single = brn.forward(x_single);
    ASSERT_EQ(out_single.sizes().size(), 2);
    ASSERT_EQ(out_single.size(0), 1);
    ASSERT_EQ(out_single.size(1), num_features);
}

INSTANTIATE_TEST_SUITE_P(
    TestBatchRenorm, BatchRenormalizationParam,
    testing::Combine(
        testing::Values(1, 2, 3), testing::Values(1, 2), testing::Values(true, false),
        testing::Values(1, 2)));