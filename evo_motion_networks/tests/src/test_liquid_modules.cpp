//
// Created by samuel on 10/11/24.
//

#include <gtest/gtest.h>

#include <evo_motion_networks/agents/actor_critic_liquid.h>
#include <evo_motion_networks_tests/test_liquid_modules.h>

/*
 * Liquid Cell
 */

TEST_P(ParamLiquidCellModule, TestLiquidCell) {
    const int input_size = std::get<0>(GetParam());
    const int neuron_number = std::get<1>(GetParam());
    const int unfolding_steps = std::get<2>(GetParam());
    const int batch_size = std::get<3>(GetParam());

    const auto x = torch::randn({batch_size, input_size});

    auto liquid_cell = LiquidCellModule(input_size, neuron_number, unfolding_steps);

    auto out = liquid_cell.forward(x);

    ASSERT_EQ(out.sizes().size(), x.sizes().size());
    ASSERT_EQ(out.size(0), batch_size);
    ASSERT_EQ(out.size(1), neuron_number);
}

INSTANTIATE_TEST_SUITE_P(
    TestLiquidCell, ParamLiquidCellModule,
    testing::Combine(
        testing::Values(1, 2, 3), testing::Values(1, 2, 3), testing::Values(1, 2, 3),
        testing::Values(1, 2, 3)));

/*
 * Actor Critic
 */

// Actor

TEST_P(ParamLiquidModule, TestLiquidActor) {
    const int state_space = std::get<0>(GetParam());
    const int hidden_space = std::get<1>(GetParam());
    const int action_space = std::get<2>(GetParam());
    const int unfolding_steps = std::get<3>(GetParam());

    const auto state = torch::randn({state_space});
    auto actor_liquid =
        ActorLiquidModule({state_space}, {action_space}, hidden_space, unfolding_steps);

    const auto [mu, sigma] = actor_liquid.forward(state);

    ASSERT_EQ(mu.sizes().size(), 1);
    ASSERT_EQ(mu.size(0), action_space);
    ASSERT_TRUE(torch::all(mu >= -1.f).item().toBool());
    ASSERT_TRUE(torch::all(mu <= 1.f).item().toBool());

    ASSERT_EQ(sigma.sizes().size(), 1);
    ASSERT_EQ(sigma.size(0), action_space);
    ASSERT_TRUE(torch::all(sigma > 0).item().toBool());
}

// Critic

TEST_P(ParamLiquidModule, TestLiquidCritic) {
    const int state_space = std::get<0>(GetParam());
    const int hidden_space = std::get<1>(GetParam());
    const int unfolding_steps = std::get<3>(GetParam());

    const auto state = torch::randn({state_space});
    auto critic_liquid = CriticLiquidModule({state_space}, hidden_space, unfolding_steps);

    const auto [out] = critic_liquid.forward(state);

    ASSERT_EQ(out.sizes().size(), 1);
    ASSERT_EQ(out.size(0), 1);
}

// QNetwork

TEST_P(ParamLiquidModule, TestLiquidQNetwork) {
    const int state_space = std::get<0>(GetParam());
    const int hidden_space = std::get<1>(GetParam());
    const int action_space = std::get<2>(GetParam());
    const int unfolding_steps = std::get<3>(GetParam());

    const auto state = torch::randn({state_space});
    const auto action = torch::rand({action_space}) * 2.f - 1.f;
    auto q_net_liquid =
        QNetworkLiquidModule({state_space}, {action_space}, hidden_space, unfolding_steps);

    const auto [out] = q_net_liquid.forward(state, action);

    ASSERT_EQ(out.sizes().size(), 1);
    ASSERT_EQ(out.size(0), 1);
}

INSTANTIATE_TEST_SUITE_P(
    TestLiquid, ParamLiquidModule,
    testing::Combine(
        testing::Values(1, 2, 3), testing::Values(1, 2, 3), testing::Values(1, 2, 100),
        testing::Values(1, 2, 3)));