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

std::tuple<torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor>
        replay_buffer::sample(int batch_size) {
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
    torch::Tensor dones = torch::tensor({tmp[0].done ? 1.f : 0.f}).unsqueeze(0);

    for (int i = 1; i < tmp.size(); i++) {
        auto m = tmp[i];
        states = torch::cat({states, m.state.unsqueeze(0)}, 0);
        actions = torch::cat({actions, m.action.unsqueeze(0)}, 0);
        rewards = torch::cat({rewards, torch::tensor({m.reward}).unsqueeze(0)}, 0);
        new_states = torch::cat({new_states, m.next_state.unsqueeze(0)}, 0);
        dones = torch::cat({dones, torch::tensor({tmp[0].done ? 1.f : 0.f}).unsqueeze(0)}, 0);
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
