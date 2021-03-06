//
// Created by samuel on 13/08/19.
//

#include "deep_q_learning.h"


////////////////////////
// Q-Network
////////////////////////

q_network::q_network(torch::IntArrayRef state_space, torch::IntArrayRef action_space, int hidden_size) {
    l1 = register_module("l1", torch::nn::Linear(state_space[0], hidden_size));
    l2 = register_module("l2", torch::nn::Linear(hidden_size, hidden_size));
    l3 = register_module("l3", torch::nn::Linear(hidden_size, action_space[0]));
    torch::nn::init::xavier_normal_(l1->weight);
    torch::nn::init::xavier_normal_(l2->weight);
    torch::nn::init::uniform_(l3->weight, -1.f, 1.f);
}

torch::Tensor q_network::forward(torch::Tensor input) {
    auto out_l1 = torch::relu(l1->forward(input));
    auto out_l2 = torch::relu(l2->forward(out_l1));
    auto pred = torch::softmax(l3->forward(out_l2), -1);
    return pred;
}


////////////////////////
// DQN agent
// https://github.com/udacity/deep-reinforcement-learning/tree/master/dqn
////////////////////////

dqn_agent::dqn_agent(int seed, torch::IntArrayRef state_space, torch::IntArrayRef action_space, int hidden_size) :
        agent(state_space, action_space, 3000),
        rd_gen(seed), rd_uni(0.f, 1.f),
        local_q_network(m_state_space, m_action_space, hidden_size),
        target_q_network(m_state_space, m_action_space, hidden_size),
        optimizer(torch::optim::Adam(local_q_network.parameters(), 4e-3f)),
        is_cuda(false), batch_size(16), gamma(0.95f), tau(1e-3f), update_every(4),idx_step(0)  {
    // Hard copy target <- local
    torch::NoGradGuard ng;
    target_q_network.l1->weight.data().copy_(local_q_network.l1->weight.data());
    target_q_network.l1->bias.data().copy_(local_q_network.l1->bias.data());

    target_q_network.l2->weight.data().copy_(local_q_network.l2->weight.data());
    target_q_network.l2->bias.data().copy_(local_q_network.l2->bias.data());

    target_q_network.l3->weight.data().copy_(local_q_network.l3->weight.data());
    target_q_network.l3->bias.data().copy_(local_q_network.l3->bias.data());
}

void dqn_agent::step(torch::Tensor state, torch::Tensor action, float reward, torch::Tensor next_state, bool done) {
    memory_buffer.add(state, action, reward, next_state, done);

    // Step counter
    idx_step = (idx_step + 1) % update_every;

    // Update every n step
    if (idx_step == 0) {
        // If memory size is sufficient
        if (memory_buffer.mem.size() > batch_size) {
            // Pick randomly samples
            auto t = memory_buffer.sample(batch_size);

            torch::Tensor sample_states = std::get<0>(t);
            torch::Tensor sample_actions = std::get<1>(t);
            torch::Tensor sample_rewards = std::get<2>(t);
            torch::Tensor sample_next_states = std::get<3>(t);
            torch::Tensor sample_dones = std::get<4>(t);

            if (is_cuda) {
                sample_states = sample_states.to(torch::kCUDA);
                sample_actions = sample_actions.to(torch::kCUDA);
                sample_rewards = sample_rewards.to(torch::kCUDA);
                sample_next_states = sample_next_states.to(torch::kCUDA);
                sample_dones = sample_dones.to(torch::kCUDA);
            }

            // Learn on those samples
            learn(sample_states, sample_actions, sample_rewards, sample_next_states, sample_dones);
        }
    }
}

torch::Tensor dqn_agent::act(torch::Tensor state, float eps) {
    // Exploration
    if (rd_uni(rd_gen) <= eps) return torch::softmax(torch::rand(m_action_space), -1);

    // Exploitation

    // state.sizes() == (state_space)
    // state.sizes() == (1, state_space)
    state = state.unsqueeze(0);

    local_q_network.eval();

    if (is_cuda) state = state.to(torch::kCUDA);

    // No gradient and compute graph
    torch::NoGradGuard no_grad;
    torch::Tensor action_values = local_q_network.forward(state).squeeze(0);

    if (is_cuda) action_values = action_values.to(torch::kCPU);

    return action_values;

}

void dqn_agent::learn(torch::Tensor states, torch::Tensor actions, torch::Tensor rewards, torch::Tensor next_states,
                      torch::Tensor dones) {
    local_q_network.train();

    // Get max predicted Q values (for next states) from target model
    torch::Tensor q_targets_next = std::get<0>(
            target_q_network.forward(std::move(next_states)).detach().max(1)).unsqueeze(1);

    // Compute Q targets for current states
    torch::Tensor q_targets = rewards + (gamma * q_targets_next * (1.f - dones.to(torch::kFloat)));

    // Get expected Q values from local model
    torch::Tensor q_expected = local_q_network.forward(std::move(states)).gather(1, actions.argmax(-1).unsqueeze(1));

    torch::Tensor loss_v = torch::mse_loss(q_expected, q_targets);

    // Erase previous gradient
    optimizer.zero_grad();

    // Perform back propagation
    loss_v.backward();

    // Update weights
    optimizer.step();

    {
        torch::NoGradGuard ng;
        // Update target Q-Network
        target_q_network.l1->weight.data().copy_(
                tau * local_q_network.l1->weight.data() + (1.f - tau) * target_q_network.l1->weight.data());
        target_q_network.l1->bias.data().copy_(
                tau * local_q_network.l1->bias.data() + (1.f - tau) * target_q_network.l1->bias.data());

        target_q_network.l2->weight.data().copy_(
                tau * local_q_network.l2->weight.data() + (1.f - tau) * target_q_network.l2->weight.data());
        target_q_network.l2->bias.data().copy_(
                tau * local_q_network.l2->bias.data() + (1.f - tau) * target_q_network.l2->bias.data());

        target_q_network.l3->weight.data().copy_(
                tau * local_q_network.l3->weight.data() + (1.f - tau) * target_q_network.l3->weight.data());
        target_q_network.l3->bias.data().copy_(
                tau * local_q_network.l3->bias.data() + (1.f - tau) * target_q_network.l3->bias.data());
    }
}


void dqn_agent::save(std::string out_folder_path) {
    // Network files
    std::string local_dqn_file = out_folder_path + "/" + "local_dqn.th";
    std::string target_dqn_file = out_folder_path + "/" + "target_dqn.th";

    torch::serialize::OutputArchive local_dqn_archive;
    torch::serialize::OutputArchive target_qdn_archive;

    // Save networks
    local_q_network.save(local_dqn_archive);
    target_q_network.save(target_qdn_archive);

    local_dqn_archive.save_to(local_dqn_file);
    target_qdn_archive.save_to(target_dqn_file);

    // Optimizer files
    std::string optim_file = out_folder_path + "/" + "optim.th";

    torch::serialize::OutputArchive optim_ouput_archive;

    // Save optimizers
    optimizer.save(optim_ouput_archive);

    optim_ouput_archive.save_to(optim_file);
}

void dqn_agent::load(std::string input_folder_path) {
    // Network files
    std::string local_dqn_file = input_folder_path + "/" + "local_dqn.th";
    std::string target_dqn_file = input_folder_path + "/" + "target_dqn.th";

    torch::serialize::InputArchive local_dqn_archive;
    torch::serialize::InputArchive target_qdn_archive;

    // Load networks
    local_dqn_archive.load_from(local_dqn_file);
    target_qdn_archive.load_from(target_dqn_file);

    local_q_network.load(local_dqn_archive);
    target_q_network.load(target_qdn_archive);

    // Optimizer file
    std::string optim_file = input_folder_path + "/" + "optim.th";

    torch::serialize::InputArchive optim_ouput_archive;

    // Load optimizer
    optim_ouput_archive.load_from(optim_file);

    optimizer.load(optim_ouput_archive);
}

bool dqn_agent::is_discrete() {
    return true;
}

void dqn_agent::cuda() {
    is_cuda = true;

    local_q_network.to(torch::kCUDA);
    target_q_network.to(torch::kCUDA);
}

void dqn_agent::cpu() {
    is_cuda = false;

    local_q_network.to(torch::kCPU);
    target_q_network.to(torch::kCPU);
}
