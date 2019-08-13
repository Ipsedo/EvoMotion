//
// Created by samuel on 13/08/19.
//

#include "agent.h"

replay_buffer::replay_buffer(int max_size) :
max_size(max_size), mem() {}

void replay_buffer::add(torch::Tensor state, torch::Tensor action, float reward, torch::Tensor next_state, bool done) {
    memory m{std::move(state), std::move(action), reward, std::move(next_state), done};

    mem.push_back(m);

    if (mem.size() > max_size)
        mem.pop_front();
}

std::tuple<torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor> replay_buffer::sample(int batch_size) {
    std::vector<memory> tmp;

    if (batch_size < 1) {
        std::cout << "Need sample batch size > 1 !" << std::endl;
        exit(1);
    }

    for (int i = 0; i < batch_size; i++) {
        auto idx = rand() % mem.size();
        tmp.push_back(mem[idx]);
    }

    torch::Tensor states = tmp[0].state.unsqueeze(0);
    torch::Tensor actions = tmp[0].action.unsqueeze(0);
    torch::Tensor rewards = torch::tensor({tmp[0].reward}).unsqueeze(0);
    torch::Tensor new_states = tmp[0].next_state.unsqueeze(0);
    torch::Tensor dones = torch::tensor({tmp[0].done}).unsqueeze(0);

    for (int i = 1; i < tmp.size(); i++) {
        auto m = tmp[i];
        states = torch::cat({states, m.state.unsqueeze(0)}, 0);
        actions = torch::cat({actions, m.action.unsqueeze(0)}, 0);
        rewards = torch::cat({rewards, torch::tensor({m.reward}).unsqueeze(0)}, 0);
        new_states = torch::cat({new_states, m.next_state.unsqueeze(0)}, 0);
        dones = torch::cat({dones, torch::tensor({m.done}).unsqueeze(0)}, 0);
    }

    return std::tuple<torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor>(
            states, actions, rewards, new_states, dones);
}

// Agent
agent::agent(torch::IntArrayRef state_space, torch::IntArrayRef action_space, int buffer_size) :
        m_state_space(state_space), m_action_space(action_space), memory_buffer(buffer_size) {}


// Random agent
random_agent::random_agent(torch::IntArrayRef state_space,
        torch::IntArrayRef action_space) : agent(state_space, action_space, 10) {
}

void random_agent::step(torch::Tensor state, torch::Tensor action, float reward, torch::Tensor next_state,
                        bool done) {
    // Do nothing for random agent;
    memory_buffer.add(state, action, reward, next_state, done);

    auto t = memory_buffer.sample(10);
}

torch::Tensor random_agent::act(torch::Tensor state, float eps) {
    return torch::rand(m_action_space);
}

// DQN agent

dqn_agent::dqn_agent(torch::IntArrayRef state_space, torch::IntArrayRef action_space) :
    agent(state_space, action_space, 5000),
    target_q_network(m_state_space, m_action_space),
    local_q_network(m_state_space, m_action_space),
    optimizer(torch::optim::Adam(local_q_network.parameters(), 3e-4f)),
    idx_step(0) {}

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

    if ((float(rand()) / RAND_MAX) > eps) return action_values;

    return torch::rand(m_action_space);
}

void dqn_agent::learn(torch::Tensor states, torch::Tensor actions, torch::Tensor rewards, torch::Tensor next_states,
                      torch::Tensor dones) {
    // Get max predicted Q values (for next states) from target model
    torch::Tensor q_targets_next = std::get<0>(target_q_network.forward(next_states).detach().max(1)).unsqueeze(1);
    // Compute Q targets for current states
    torch::Tensor q_targets = rewards + (gamma * q_targets_next * (1.f - dones.to(torch::kFloat)));

    // Get expected Q values from local model
    torch::Tensor q_expected = local_q_network.forward(states).gather(1, actions.argmax(-1).unsqueeze(1));

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

        target_param = tau * local_param + (1.f - tau) * target_param;
    }
}
