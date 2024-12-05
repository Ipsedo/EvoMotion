//
// Created by samuel on 10/11/24.
//

#include <gtest/gtest.h>

#include <evo_motion_networks/agents/actor_critic.h>
#include <evo_motion_networks/agents/actor_critic_liquid.h>
#include <evo_motion_networks/agents/ppo_gae.h>
#include <evo_motion_networks/agents/ppo_gae_liquid.h>
#include <evo_motion_networks/agents/ppo_vanilla.h>
#include <evo_motion_networks/agents/soft_actor_critic.h>
#include <evo_motion_networks/agents/soft_actor_critic_liquid.h>
#include <evo_motion_networks_tests/test_agents.h>

// Actor Critic

TEST_P(ParamActorCriticAgent, TestActorCritic) {
    const auto [state_space, action_space, hidden_size, batch_size, train_every] = GetParam();

    auto agent = ActorCriticAgent(
        1234, {state_space}, {action_space}, hidden_size, batch_size, 1e-3f, 0.99f, 0.01f, 0.001f,
        128, 1024, train_every);

    for (int j = 0; j < 5; j++) {
        for (int i = 0; i < batch_size * 2; i++) {
            const auto state = torch::randn({state_space});
            const auto action = agent.act(state, torch::randn({1}).item().toFloat());

            ASSERT_EQ(action.sizes().size(), 1);
            ASSERT_EQ(action.size(0), action_space);
            ASSERT_TRUE(torch::all(~torch::isnan(action)).item().toBool());
            ASSERT_TRUE(torch::all(action >= -1.f).item().toBool());
            ASSERT_TRUE(torch::all(action <= 1.f).item().toBool());
        }
        agent.done(torch::randn({state_space}), torch::randn({1}).item().toFloat());
    }

    for (auto m: agent.get_metrics()) ASSERT_TRUE(m.loss() != 0.f);
}

// Soft Actor Critic

TEST_P(ParamActorCriticAgent, TestSoftActorCritic) {
    const auto [state_space, action_space, hidden_size, batch_size, train_every] = GetParam();

    auto agent = SoftActorCriticAgent(
        1234, {state_space}, {action_space}, hidden_size, batch_size, 1e-3f, 0.9f, 0.005f, 128,
        train_every);

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < batch_size * 2; j++) {
            const auto state = torch::randn({state_space});
            const auto action = agent.act(state, torch::randn({1}).item().toFloat());

            ASSERT_EQ(action.sizes().size(), 1);
            ASSERT_EQ(action.size(0), action_space);
            ASSERT_TRUE(torch::all(~torch::isnan(action)).item().toBool());
            ASSERT_TRUE(torch::all(action >= -1.f).item().toBool());
            ASSERT_TRUE(torch::all(action <= 1.f).item().toBool());
        }

        agent.done(torch::randn({state_space}), torch::randn({1}).item().toFloat());
    }

    for (auto m: agent.get_metrics()) ASSERT_TRUE(m.loss() != 0.f);
}

// Actor Critic Liquid

TEST_P(ParamActorCriticAgent, TestActorCriticLiquid) {
    const auto [state_space, action_space, hidden_size, batch_size, train_every] = GetParam();

    auto agent = ActorCriticLiquidAgent(
        1234, {state_space}, {action_space}, hidden_size, batch_size, 1e-3f, 0.9f, 1.f, 0.1f, 16, 6,
        32, train_every);

    for (int j = 0; j < 5; j++) {
        for (int i = 0; i < batch_size * 2; i++) {
            const auto state = torch::randn({state_space});
            const auto action = agent.act(state, torch::randn({1}).item().toFloat());

            ASSERT_EQ(action.sizes().size(), 1);
            ASSERT_EQ(action.size(0), action_space);
            ASSERT_TRUE(torch::all(~torch::isnan(action)).item().toBool());
            ASSERT_TRUE(torch::all(action >= -1.f).item().toBool());
            ASSERT_TRUE(torch::all(action <= 1.f).item().toBool());
        }

        agent.done(torch::randn({state_space}), torch::randn({1}).item().toFloat());
    }

    for (auto m: agent.get_metrics()) ASSERT_TRUE(m.loss() != 0.f);
}

// Soft Actor Critic Liquid

TEST_P(ParamActorCriticAgent, TestSoftActorCriticLiquid) {
    const auto [state_space, action_space, hidden_size, batch_size, train_every] = GetParam();

    auto agent = SoftActorCriticLiquidAgent(
        1234, {state_space}, {action_space}, hidden_size, batch_size, 1e-3f, 0.9f, 0.005f, 6, 128,
        train_every);

    for (int j = 0; j < 5; j++) {
        for (int i = 0; i < batch_size * 2; i++) {
            const auto state = torch::randn({state_space});
            const auto action = agent.act(state, 1.f);

            ASSERT_EQ(action.sizes().size(), 1);
            ASSERT_EQ(action.size(0), action_space);
            ASSERT_TRUE(torch::all(~torch::isnan(action)).item().toBool());
            ASSERT_TRUE(torch::all(action >= -1.f).item().toBool());
            ASSERT_TRUE(torch::all(action <= 1.f).item().toBool());
        }

        agent.done(torch::randn({state_space}), 1.f);
    }

    for (auto m: agent.get_metrics()) ASSERT_TRUE(m.loss() != 0.f);
}

// PPO vanilla

TEST_P(ParamActorCriticAgent, TestPpoVanilla) {
    const auto [state_space, action_space, hidden_size, batch_size, train_every] = GetParam();

    auto agent = PpoVanillaAgent(
        1234, {state_space}, {action_space}, hidden_size, 0.99f, 0.2f, 0.01f, 0.5f, 4, batch_size,
        1e-3f);

    for (int j = 0; j < 5; j++) {
        for (int i = 0; i < batch_size * 2; i++) {
            const auto state = torch::randn({state_space});
            const auto action = agent.act(state, 1.f);

            ASSERT_EQ(action.sizes().size(), 1);
            ASSERT_EQ(action.size(0), action_space);
            ASSERT_TRUE(torch::all(~torch::isnan(action)).item().toBool());
            ASSERT_TRUE(torch::all(action >= -1.f).item().toBool());
            ASSERT_TRUE(torch::all(action <= 1.f).item().toBool());
        }

        agent.done(torch::randn({state_space}), 1.f);
    }

    for (auto m: agent.get_metrics()) ASSERT_TRUE(m.loss() != 0.f);
}

// PPO vanilla

TEST_P(ParamActorCriticAgent, TestPpoGae) {
    const auto [state_space, action_space, hidden_size, batch_size, train_every] = GetParam();

    auto agent = PpoGaeAgent(
        1234, {state_space}, {action_space}, hidden_size, 0.99f, 0.95f, 0.2f, 0.01f, 0.5f, 1,
        batch_size, 1, batch_size * 2, 1e-3f, 0.5f);

    for (int i = 0; i < batch_size; i++) {
        for (int j = 0; j < 2; j++) {
            const auto state = torch::randn({state_space});
            const auto action = agent.act(state, 1.f);

            ASSERT_EQ(action.sizes().size(), 1);
            ASSERT_EQ(action.size(0), action_space);
            ASSERT_TRUE(torch::all(~torch::isnan(action)).item().toBool());
            ASSERT_TRUE(torch::all(action >= -1.f).item().toBool());
            ASSERT_TRUE(torch::all(action <= 1.f).item().toBool());
        }

        agent.done(torch::randn({state_space}), 1.f);
    }

    for (auto m: agent.get_metrics()) ASSERT_TRUE(m.loss() != 0.f);
}

TEST_P(ParamActorCriticAgent, TestPpoLiquidGae) {
    const auto [state_space, action_space, hidden_size, batch_size, train_every] = GetParam();

    auto agent = PpoGaeLiquidAgent(
        1234, {state_space}, {action_space}, hidden_size, 6, 0.99f, 0.95f, 0.2f, 0.01f, 0.5f, 1,
        batch_size, 1, batch_size * 2, 1e-3f, 0.5f);

    for (int i = 0; i < batch_size; i++) {
        for (int j = 0; j < 2; j++) {
            const auto state = torch::randn({state_space});
            const auto action = agent.act(state, 1.f);

            ASSERT_EQ(action.sizes().size(), 1);
            ASSERT_EQ(action.size(0), action_space);
            ASSERT_TRUE(torch::all(~torch::isnan(action)).item().toBool());
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
        testing::Values(2, 3), testing::Values(2, 3), testing::Values(2, 3), testing::Values(2, 3),
        testing::Values(2, 3)));
