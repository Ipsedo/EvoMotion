//
// Created by samuel on 10/11/24.
//

#include <gtest/gtest.h>

#include <evo_motion_networks/agents/actor_critic.h>
#include <evo_motion_networks/agents/soft_actor_critic.h>
#include <evo_motion_networks_tests/test_linear_modules.h>

// Actor

TEST_P(ParamLinearModule, TestActorModule) {
    const int state_space = std::get<0>(GetParam());
    const int hidden_size = std::get<1>(GetParam());
    const int action_space = std::get<2>(GetParam());

    const auto state = torch::randn({state_space});
    auto actor = ActorModule({state_space}, {action_space}, hidden_size);

    const auto [mu, sigma] = actor.forward(state);

    ASSERT_EQ(mu.sizes().size(), 1);
    ASSERT_EQ(mu.size(0), action_space);
    ASSERT_TRUE(torch::all(mu >= -1.f).item().toBool());
    ASSERT_TRUE(torch::all(mu <= 1.f).item().toBool());

    ASSERT_EQ(sigma.sizes().size(), 1);
    ASSERT_EQ(sigma.size(0), action_space);
    ASSERT_TRUE(torch::all(sigma > 0).item().toBool());
}

TEST_P(ParamLinearModule, TestBatchedActorModule) {
    const int state_space = std::get<0>(GetParam());
    const int hidden_size = std::get<1>(GetParam());
    const int action_space = std::get<2>(GetParam());
    const int batch_size = std::get<3>(GetParam());

    const auto state = torch::randn({batch_size, state_space});
    auto actor = ActorModule({state_space}, {action_space}, hidden_size);

    const auto [mu, sigma] = actor.forward(state);

    ASSERT_EQ(mu.sizes().size(), 2);
    ASSERT_EQ(mu.size(0), batch_size);
    ASSERT_EQ(mu.size(1), action_space);
    ASSERT_TRUE(torch::all(mu >= -1.f).item().toBool());
    ASSERT_TRUE(torch::all(mu <= 1.f).item().toBool());

    ASSERT_EQ(sigma.sizes().size(), 2);
    ASSERT_EQ(sigma.size(0), batch_size);
    ASSERT_EQ(sigma.size(1), action_space);
    ASSERT_TRUE(torch::all(sigma > 0).item().toBool());
}

// Critic

TEST_P(ParamLinearModule, TestCriticModule) {
    const int state_space = std::get<0>(GetParam());
    const int hidden_size = std::get<1>(GetParam());

    const auto state = torch::randn({state_space});
    auto critic = CriticModule({state_space}, hidden_size);

    const auto [out] = critic.forward(state);

    ASSERT_EQ(out.sizes().size(), 1);
    ASSERT_EQ(out.size(0), 1);
}

TEST_P(ParamLinearModule, TestBatchedCriticModule) {
    const int state_space = std::get<0>(GetParam());
    const int hidden_size = std::get<1>(GetParam());
    const int batch_size = std::get<3>(GetParam());

    const auto state = torch::randn({batch_size, state_space});
    auto critic = CriticModule({state_space}, hidden_size);

    const auto [out] = critic.forward(state);

    ASSERT_EQ(out.sizes().size(), 2);
    ASSERT_EQ(out.size(0), batch_size);
    ASSERT_EQ(out.size(1), 1);
}

// Q-Network

TEST_P(ParamLinearModule, TestQNetworkModule) {
    const int state_space = std::get<0>(GetParam());
    const int hidden_size = std::get<1>(GetParam());
    const int action_space = std::get<2>(GetParam());

    const auto state = torch::randn({state_space});
    const auto action = torch::rand({action_space}) * 2.f - 1.f;

    auto q_net = QNetworkModule({state_space}, {action_space}, hidden_size);

    const auto [out] = q_net.forward(state, action);

    ASSERT_EQ(out.sizes().size(), 1);
    ASSERT_EQ(out.size(0), 1);
}

TEST_P(ParamLinearModule, TestBatchedQNetworkModule) {
    const int state_space = std::get<0>(GetParam());
    const int hidden_size = std::get<1>(GetParam());
    const int action_space = std::get<2>(GetParam());
    const int batch_size = std::get<3>(GetParam());

    const auto state = torch::randn({batch_size, state_space});
    const auto action = torch::rand({batch_size, action_space}) * 2.f - 1.f;

    auto q_net = QNetworkModule({state_space}, {action_space}, hidden_size);

    const auto [out] = q_net.forward(state, action);

    ASSERT_EQ(out.sizes().size(), 2);
    ASSERT_EQ(out.size(0), batch_size);
    ASSERT_EQ(out.size(1), 1);
}

// Create parametrized tests

INSTANTIATE_TEST_SUITE_P(
    TestLinearModule, ParamLinearModule,
    testing::Combine(
        testing::Values(1, 2, 3), testing::Values(1, 2, 3), testing::Values(1, 2, 100),
        testing::Values(1, 2, 3)));