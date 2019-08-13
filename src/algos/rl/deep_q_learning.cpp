//
// Created by samuel on 13/08/19.
//

#include "deep_q_learning.h"


////////////////////////
// Q-Network
////////////////////////

q_network::q_network(torch::IntArrayRef state_space, torch::IntArrayRef action_space) {
    l1 = register_module("l1", torch::nn::Linear(state_space[0], 24));
    l2 = register_module("l2", torch::nn::Linear(24, action_space[0]));
}

torch::Tensor q_network::forward(torch::Tensor input) {
    auto out_l1 = torch::relu(l1->forward(input));
    auto pred = torch::softmax(l2->forward(out_l1), -1);
    return pred;
}


////////////////////////
// DQN agent
////////////////////////

dqn_agent::dqn_agent(torch::IntArrayRef state_space, torch::IntArrayRef action_space) :
        agent(state_space, action_space, 2000),
        target_q_network(m_state_space, m_action_space),
        local_q_network(m_state_space, m_action_space),
        optimizer(torch::optim::Adam(local_q_network.parameters(), 3e-4f)),
        idx_step(0), batch_size(16), gamma(0.99f), tau(1e-5f), update_every(4),
        rd_gen(123456), rd_uni(0.f, 1.f) {}

void dqn_agent::step(torch::Tensor state, torch::Tensor action, float reward, torch::Tensor next_state, bool done) {
    memory_buffer.add(state, action, reward, next_state, done);

    idx_step = (idx_step + 1) % update_every;

    if (idx_step == 0) {
        if (memory_buffer.mem.size() > batch_size) {
            auto t = memory_buffer.sample(batch_size);

            torch::Tensor sample_states = std::get<0>(t);
            torch::Tensor sample_actions = std::get<1>(t);
            torch::Tensor sample_rewards = std::get<2>(t);
            torch::Tensor sample_next_states = std::get<3>(t);
            torch::Tensor sample_dones = std::get<4>(t);

            learn(sample_states, sample_actions, sample_rewards, sample_next_states, sample_dones);
        }
    }
}

torch::Tensor dqn_agent::act(torch::Tensor state, float eps) {
    state = state.unsqueeze(0);

    local_q_network.eval();
    torch::Tensor action_values;
    {
        torch::NoGradGuard no_grad;
        action_values = local_q_network.forward(state).squeeze(0);
    }
    local_q_network.train();

    if (rd_uni(rd_gen) > eps) return action_values;

    return torch::rand(m_action_space);
}

void dqn_agent::learn(torch::Tensor states, torch::Tensor actions, torch::Tensor rewards, torch::Tensor next_states,
                      torch::Tensor dones) {
    // Get max predicted Q values (for next states) from target model
    torch::Tensor q_targets_next = std::get<0>(target_q_network.forward(std::move(next_states)).detach().max(1)).unsqueeze(1);
    // Compute Q targets for current states
    torch::Tensor q_targets = rewards + (gamma * q_targets_next * (1.f - dones.to(torch::kFloat)));

    // Get expected Q values from local model
    torch::Tensor q_expected = local_q_network.forward(std::move(states)).gather(1, actions.argmax(-1).unsqueeze(1));

    torch::Tensor loss_v = torch::mse_loss(q_expected, q_targets);

    optimizer.zero_grad();
    loss_v.backward();
    optimizer.step();

    soft_update();
}

void dqn_agent::soft_update() {
    for (int i = 0; i < target_q_network.parameters().size(); i++) {
        auto target_param = target_q_network.parameters()[i];
        auto local_param = local_q_network.parameters()[i];

        target_param = (tau * local_param + (1.f - tau) * target_param).clone();
    }
}

