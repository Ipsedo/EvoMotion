//
// Created by samuel on 10/11/24.
//

#include <gtest/gtest.h>

#include <evo_motion_networks/agents/actor_critic.h>
#include <evo_motion_networks/agents/actor_critic_liquid.h>
#include <evo_motion_networks/agents/soft_actor_critic.h>
#include <evo_motion_networks_tests/test_agents.h>

// Actor Critic

TEST_P(ParamActorCriticAgent, TestActorCritic) {
    const int state_space = std::get<0>(GetParam());
    const int action_space = std::get<1>(GetParam());
    const int hidden_size = std::get<2>(GetParam());
    const int batch_size = std::get<3>(GetParam());

    auto agent = ActorCriticAgent(
        1234, {state_space}, {action_space}, hidden_size, batch_size, 1.f, 0.9f, 1.f, 0.1f, 16);

    for (int i = 0; i < batch_size * 2; i++) {
        for (int j = 0; j < 5; j++) {
            const auto state = torch::randn({state_space});
            const auto action = agent.act(state, 1.f);

            ASSERT_EQ(action.sizes().size(), 1);
            ASSERT_EQ(action.size(0), action_space);
            ASSERT_TRUE(torch::all(action >= -1.f).item().toBool());
            ASSERT_TRUE(torch::all(action <= 1.f).item().toBool());
        }

        agent.done(torch::randn({state_space}), 1.f);
    }

    for (auto m: agent.get_metrics()) ASSERT_TRUE(m.loss() != 0.f);
}

// Soft Actor Critic

TEST_P(ParamActorCriticAgent, TestSoftActorCritic) {
    const int state_space = std::get<0>(GetParam());
    const int action_space = std::get<1>(GetParam());
    const int hidden_size = std::get<2>(GetParam());
    const int batch_size = std::get<3>(GetParam());

    auto agent = SoftActorCriticAgent(
        1234, {state_space}, {action_space}, hidden_size, batch_size, 1.f, 0.9f, 0.005f);

    for (int i = 0; i < batch_size * 2; i++) {
        for (int j = 0; j < 5; j++) {
            const auto state = torch::randn({state_space});
            const auto action = agent.act(state, 1.f);

            ASSERT_EQ(action.sizes().size(), 1);
            ASSERT_EQ(action.size(0), action_space);
            ASSERT_TRUE(torch::all(action >= -1.f).item().toBool());
            ASSERT_TRUE(torch::all(action <= 1.f).item().toBool());
        }

        agent.done(torch::randn({state_space}), 1.f);
    }

    for (auto m: agent.get_metrics()) ASSERT_TRUE(m.loss() != 0.f);
}

// Actor Critic Liquid

TEST_P(ParamActorCriticAgent, TestActorCriticLiquid) {
    const int state_space = std::get<0>(GetParam());
    const int action_space = std::get<1>(GetParam());
    const int hidden_size = std::get<2>(GetParam());
    const int batch_size = std::get<3>(GetParam());

    auto agent = ActorCriticLiquidAgent(
        1234, {state_space}, {action_space}, hidden_size, batch_size, 1.f, 0.9f, 1.f, 0.1f, 16, 6);

    for (int i = 0; i < batch_size * 2; i++) {
        for (int j = 0; j < 5; j++) {
            const auto state = torch::randn({state_space});
            const auto action = agent.act(state, 1.f);

            ASSERT_EQ(action.sizes().size(), 1);
            ASSERT_EQ(action.size(0), action_space);
            ASSERT_TRUE(torch::all(action >= -1.f).item().toBool());
            ASSERT_TRUE(torch::all(action <= 1.f).item().toBool());
        }

        agent.done(torch::randn({state_space}), 1.f);
    }

    for (auto m: agent.get_metrics()) ASSERT_TRUE(m.loss() != 0.f);
}

// Soft Actor Critic Liquid

TEST_P(ParamActorCriticAgent, TestSoftActorCriticLiquid) {
    const int state_space = std::get<0>(GetParam());
    const int action_space = std::get<1>(GetParam());
    const int hidden_size = std::get<2>(GetParam());
    const int batch_size = std::get<3>(GetParam());

    auto agent = SoftActorCriticLiquidAgent(
        1234, {state_space}, {action_space}, hidden_size, batch_size, 1.f, 0.9f, 0.005f, 6);

    for (int i = 0; i < batch_size * 2; i++) {
        for (int j = 0; j < 5; j++) {
            const auto state = torch::randn({state_space});
            const auto action = agent.act(state, 1.f);

            ASSERT_EQ(action.sizes().size(), 1);
            ASSERT_EQ(action.size(0), action_space);
            ASSERT_TRUE(torch::all(action >= -1.f).item().toBool());
            ASSERT_TRUE(torch::all(action <= 1.f).item().toBool());
        }

        agent.done(torch::randn({state_space}), 1.f);
    }

    for (auto m: agent.get_metrics()) ASSERT_TRUE(m.loss() != 0.f);
}

// Create parametrized tests

INSTANTIATE_TEST_SUITE_P(
    TestAgent, ParamActorCriticAgent,
    testing::Combine(
        testing::Values(1, 2, 3), testing::Values(1, 2, 100), testing::Values(1, 2, 3),
        testing::Values(1, 2, 3)));
