//
// Created by samuel on 08/11/24.
//

#include <filesystem>

#include <evo_motion_networks/agents/soft_actor_critic.h>
#include <evo_motion_networks/functions.h>
#include <evo_motion_networks/networks/actor.h>

/*
 * Agents
 */

SoftActorCriticAgent::SoftActorCriticAgent(
    int seed, const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
    int hidden_size, int batch_size, float lr, float gamma, float tau, int replay_buffer_size)
    : actor(std::make_shared<ActorModule>(state_space, action_space, hidden_size)),
      critic_1(std::make_shared<QNetworkModule>(state_space, action_space, hidden_size)),
      critic_2(std::make_shared<QNetworkModule>(state_space, action_space, hidden_size)),
      target_critic_1(std::make_shared<QNetworkModule>(state_space, action_space, hidden_size)),
      target_critic_2(std::make_shared<QNetworkModule>(state_space, action_space, hidden_size)),
      actor_optimizer(std::make_shared<torch::optim::Adam>(actor->parameters(), lr)),
      critic_1_optimizer(std::make_shared<torch::optim::Adam>(critic_1->parameters(), lr)),
      critic_2_optimizer(std::make_shared<torch::optim::Adam>(critic_2->parameters(), lr)),
      target_entropy(-1.f), entropy_parameter(),
      entropy_optimizer(entropy_parameter.parameters(), lr), curr_device(torch::kCPU), gamma(gamma),
      tau(tau), batch_size(batch_size), replay_buffer(replay_buffer_size, seed),
      curr_episode_step(0), curr_train_step(0L), global_curr_step(0L),
      actor_loss_meter("actor", 16), critic_1_loss_meter("critic_1", 16),
      critic_2_loss_meter("critic_2", 16), entropy_loss_meter("entropy", 16),
      episode_steps_meter("steps", 16) {
    at::manual_seed(seed);

    for (auto n_p: critic_1->named_parameters()) {
        const auto &name = n_p.key();
        const auto &param = n_p.value();
        target_critic_1->named_parameters()[name].data().copy_(param.data());
    }

    for (auto n_p: critic_2->named_parameters()) {
        const auto &name = n_p.key();
        const auto &param = n_p.value();
        target_critic_2->named_parameters()[name].data().copy_(param.data());
    }
}

torch::Tensor SoftActorCriticAgent::act(torch::Tensor state, float reward) {
    const auto [mu, sigma] = actor->forward(state);
    const auto action = truncated_normal_sample(mu, sigma, -1.f, 1.f);

    if (!replay_buffer.empty()) { replay_buffer.update_last(reward, state, false); }
    replay_buffer.add({state, action, 0.f, false, state});

    curr_episode_step++;
    global_curr_step++;

    check_train();

    return action;
}

void SoftActorCriticAgent::check_train() {
    if (global_curr_step % batch_size == 0) {
        std::vector<step_replay_buffer> tmp_replay_buffer = replay_buffer.sample(batch_size);

        std::vector<torch::Tensor> vec_states, vec_actions, vec_rewards, vec_done, vec_next_state;

        for (int i = 0; i < batch_size; i++) {
            const auto &rp = tmp_replay_buffer[i];

            vec_states.push_back(rp.state);
            vec_actions.push_back(rp.action);
            vec_rewards.push_back(
                torch::tensor(rp.reward, at::TensorOptions().device(curr_device)));
            vec_done.push_back(
                torch::tensor(rp.done ? 1.f : 0.f, at::TensorOptions().device(curr_device)));
            vec_next_state.push_back(rp.next_state);
        }

        train(
            torch::stack(vec_states), torch::stack(vec_actions), torch::stack(vec_rewards),
            torch::stack(vec_done), torch::stack(vec_next_state));
    }
}

void SoftActorCriticAgent::train(
    const torch::Tensor &batched_states, const torch::Tensor &batched_actions,
    const torch::Tensor &batched_rewards, const torch::Tensor &batched_done,
    const torch::Tensor &batched_next_state) {

    const auto [next_mu, next_sigma] = actor->forward(batched_next_state);
    const auto next_action = truncated_normal_sample(next_mu, next_sigma, -1.f, 1.f);
    const auto next_log_prob = truncated_normal_pdf(next_action, next_mu, next_sigma, -1.f, 1.f);

    const auto [next_target_q_value_1] = target_critic_1->forward(batched_next_state, next_action);
    const auto [next_target_q_value_2] = target_critic_2->forward(batched_next_state, next_action);

    const auto target_q_values = (batched_rewards
                                  + (1.f - batched_done) * gamma
                                        * torch::mean(
                                            torch::min(next_target_q_value_1, next_target_q_value_2)
                                                - entropy_parameter.alpha() * next_log_prob,
                                            -1))
                                     .detach();

    // critic 1
    const auto [q_value_1] = critic_1->forward(batched_states, batched_actions.detach());
    const auto critic_1_loss =
        torch::mse_loss(q_value_1.squeeze(1), target_q_values, at::Reduction::Mean);

    critic_1_optimizer->zero_grad();
    critic_1_loss.backward();
    critic_1_optimizer->step();

    // critic 2
    const auto [q_value_2] = critic_2->forward(batched_states, batched_actions.detach());
    const auto critic_2_loss =
        torch::mse_loss(q_value_2.squeeze(1), target_q_values, at::Reduction::Mean);

    critic_2_optimizer->zero_grad();
    critic_2_loss.backward();
    critic_2_optimizer->step();

    // policy
    const auto [curr_mu, curr_sigma] = actor->forward(batched_states);
    const auto curr_action = truncated_normal_sample(curr_mu, curr_sigma, -1.f, 1.f);
    const auto curr_log_prob = truncated_normal_pdf(curr_action, curr_mu, curr_sigma, -1.f, 1.f);

    const auto [curr_q_value_1] = critic_1->forward(batched_states, curr_action);
    const auto [curr_q_value_2] = critic_2->forward(batched_states, curr_action);
    const auto q_value = torch::min(curr_q_value_1, curr_q_value_2);

    const auto actor_loss = torch::mean(entropy_parameter.alpha() * curr_log_prob - q_value);

    actor_optimizer->zero_grad();
    actor_loss.backward();
    actor_optimizer->step();

    // entropy
    const auto entropy_loss =
        -torch::mean(entropy_parameter.log_alpha() * (curr_log_prob.detach() + target_entropy));

    entropy_optimizer.zero_grad();
    entropy_loss.backward();
    entropy_optimizer.step();

    // target value soft update
    for (auto n_p: critic_1->named_parameters()) {
        const auto &name = n_p.key();
        const auto &param = n_p.value();

        target_critic_1->named_parameters()[name].data().copy_(
            tau * param.data() + (1.f - tau) * target_critic_1->named_parameters()[name].data());
    }

    for (auto n_p: critic_2->named_parameters()) {
        const auto &name = n_p.key();
        const auto &param = n_p.value();

        target_critic_2->named_parameters()[name].data().copy_(
            tau * param.data() + (1.f - tau) * target_critic_2->named_parameters()[name].data());
    }

    // metrics
    actor_loss_meter.add(actor_loss.cpu().item().toFloat());
    critic_1_loss_meter.add(critic_1_loss.cpu().item().toFloat());
    critic_2_loss_meter.add(critic_2_loss.cpu().item().toFloat());
    entropy_loss_meter.add(entropy_loss.cpu().item().toFloat());

    curr_train_step++;
}

void SoftActorCriticAgent::done(torch::Tensor state, float reward) {
    replay_buffer.update_last(reward, state, true);

    episode_steps_meter.add(static_cast<float>(curr_episode_step));
    curr_episode_step = 0;
}

void SoftActorCriticAgent::save(const std::string &output_folder_path) {
    const std::filesystem::path path(output_folder_path);

    // actor
    const auto actor_model_file = path / "actor.th";
    const auto actor_optimizer_file = path / "actor_optimizer.th";

    torch::serialize::OutputArchive actor_model_archive;
    torch::serialize::OutputArchive actor_optimizer_archive;

    actor->save(actor_model_archive);
    actor_optimizer->save(actor_optimizer_archive);

    actor_model_archive.save_to(actor_model_file);
    actor_optimizer_archive.save_to(actor_optimizer_file);

    // target_critic_1
    const auto target_critic_1_model_file = path / "target_critic_1.th";
    torch::serialize::OutputArchive target_critic_1_model_archive;
    target_critic_1->save(target_critic_1_model_archive);
    target_critic_1_model_archive.save_to(target_critic_1_model_file);

    // target_critic_2
    const auto target_critic_2_model_file = path / "target_critic_2.th";
    torch::serialize::OutputArchive target_critic_2_model_archive;
    target_critic_2->save(target_critic_2_model_archive);
    target_critic_2_model_archive.save_to(target_critic_2_model_file);

    // Critic 1
    const auto critic_1_model_file = path / "critic_1.th";
    const auto critic_1_optimizer_file = path / "critic_1_optimizer.th";

    torch::serialize::OutputArchive critic_1_model_archive;
    torch::serialize::OutputArchive critic_1_optimizer_archive;

    critic_1->save(critic_1_model_archive);
    critic_1_optimizer->save(critic_1_optimizer_archive);

    critic_1_model_archive.save_to(critic_1_model_file);
    critic_1_optimizer_archive.save_to(critic_1_optimizer_file);

    // Critic 2
    const auto critic_2_model_file = path / "critic_2.th";
    const auto critic_2_optimizer_file = path / "critic_2_optimizer.th";

    torch::serialize::OutputArchive critic_2_model_archive;
    torch::serialize::OutputArchive critic_2_optimizer_archive;

    critic_2->save(critic_2_model_archive);
    critic_2_optimizer->save(critic_2_optimizer_archive);

    critic_2_model_archive.save_to(critic_2_model_file);
    critic_2_optimizer_archive.save_to(critic_2_optimizer_file);

    // entropy
    const auto entropy_model_file = path / "entropy.th";
    const auto entropy_optimizer_file = path / "entropy_optimizer.th";

    torch::serialize::OutputArchive entropy_model_archive;
    torch::serialize::OutputArchive entropy_optimizer_archive;

    entropy_parameter.save(entropy_model_archive);
    entropy_optimizer.save(entropy_optimizer_archive);

    entropy_model_archive.save_to(entropy_model_file);
    entropy_optimizer_archive.save_to(entropy_optimizer_file);
}

void SoftActorCriticAgent::load(const std::string &input_folder_path) {
    const std::filesystem::path path(input_folder_path);

    // actor
    const auto actor_model_file = path / "actor.th";
    const auto actor_optimizer_file = path / "actor_optimizer.th";

    torch::serialize::InputArchive actor_model_archive;
    torch::serialize::InputArchive actor_optimizer_archive;

    actor_model_archive.load_from(actor_model_file);
    actor_optimizer_archive.load_from(actor_optimizer_file);

    actor->load(actor_model_archive);
    actor_optimizer->load(actor_optimizer_archive);

    // target_critic_1_network
    const auto target_critic_1_model_file = path / "target_critic_1.th";
    torch::serialize::InputArchive target_critic_1_model_archive;
    target_critic_1_model_archive.load_from(target_critic_1_model_file);
    target_critic_1->load(target_critic_1_model_archive);

    // target_critic_2_network
    const auto target_critic_2_model_file = path / "target_critic_2.th";
    torch::serialize::InputArchive target_critic_2_model_archive;
    target_critic_2_model_archive.load_from(target_critic_2_model_file);
    target_critic_2->load(target_critic_2_model_archive);

    // critic_1
    const auto critic_1_model_file = path / "critic_1.th";
    const auto critic_1_optimizer_file = path / "critic_1_optimizer.th";

    torch::serialize::InputArchive critic_1_model_archive;
    torch::serialize::InputArchive critic_1_optimizer_archive;

    critic_1_model_archive.load_from(critic_1_model_file);
    critic_1_optimizer_archive.load_from(critic_1_optimizer_file);

    critic_1->load(critic_1_model_archive);
    critic_1_optimizer->load(critic_1_optimizer_archive);

    // critic_2
    const auto critic_2_model_file = path / "critic_2.th";
    const auto critic_2_optimizer_file = path / "critic_2_optimizer.th";

    torch::serialize::InputArchive critic_2_model_archive;
    torch::serialize::InputArchive critic_2_optimizer_archive;

    critic_2_model_archive.load_from(critic_2_model_file);
    critic_2_optimizer_archive.load_from(critic_2_optimizer_file);

    critic_2->load(critic_2_model_archive);
    critic_2_optimizer->load(critic_2_optimizer_archive);

    // entropy
    const auto entropy_model_file = path / "entropy.th";
    const auto entropy_optimizer_file = path / "entropy_optimizer.th";

    torch::serialize::InputArchive entropy_model_archive;
    torch::serialize::InputArchive entropy_optimizer_archive;

    entropy_model_archive.load_from(entropy_model_file);
    entropy_optimizer_archive.load_from(entropy_optimizer_file);

    entropy_parameter.load(entropy_model_archive);
    entropy_optimizer.load(entropy_optimizer_archive);
}

std::vector<LossMeter> SoftActorCriticAgent::get_metrics() {
    return {
        actor_loss_meter, critic_1_loss_meter, critic_2_loss_meter, entropy_loss_meter,
        episode_steps_meter};
}

void SoftActorCriticAgent::to(torch::DeviceType device) {
    curr_device = device;
    actor->to(device);
    critic_1->to(device);
    critic_2->to(device);
    target_critic_1->to(device);
    target_critic_2->to(device);
    entropy_parameter.to(device);
}

void SoftActorCriticAgent::set_eval(bool eval) {
    if (eval) {
        actor->eval();
        critic_1->eval();
        critic_2->eval();
        target_critic_1->eval();
        target_critic_2->eval();
        entropy_parameter.eval();
    } else {
        actor->train();
        critic_1->train();
        critic_2->train();
        target_critic_1->train();
        target_critic_2->train();
        entropy_parameter.train();
    }
}

int SoftActorCriticAgent::count_parameters() {
    int count = 0;
    for (const auto &p: actor->parameters()) {
        auto sizes = p.sizes();
        count += std::reduce(sizes.begin(), sizes.end(), 1, std::multiplies<>());
    }
    for (const auto &p: target_critic_1->parameters()) {
        auto sizes = p.sizes();
        count += std::reduce(sizes.begin(), sizes.end(), 1, std::multiplies<>());
    }
    for (const auto &p: target_critic_2->parameters()) {
        auto sizes = p.sizes();
        count += std::reduce(sizes.begin(), sizes.end(), 1, std::multiplies<>());
    }
    for (const auto &p: critic_1->parameters()) {
        auto sizes = p.sizes();
        count += std::reduce(sizes.begin(), sizes.end(), 1, std::multiplies<>());
    }
    for (const auto &p: critic_2->parameters()) {
        auto sizes = p.sizes();
        count += std::reduce(sizes.begin(), sizes.end(), 1, std::multiplies<>());
    }
    return count;
}
