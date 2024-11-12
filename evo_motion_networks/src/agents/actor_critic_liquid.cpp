//
// Created by samuel on 29/03/24.
//

#include <evo_motion_networks/agents/actor_critic_liquid.h>
#include <evo_motion_networks/functions.h>

ActorCriticLiquidAgent::ActorCriticLiquidAgent(
    const int seed, const std::vector<int64_t> &state_space,
    const std::vector<int64_t> &action_space, int neuron_number, const int batch_size, float lr,
    const float gamma, float entropy_start_factor, float entropy_end_factor, long entropy_steps,
    int unfolding_steps, int replay_buffer_size, int train_every)
    : actor(std::make_shared<ActorLiquidModule>(
          state_space, action_space, neuron_number, unfolding_steps)),
      actor_optimizer(std::make_shared<torch::optim::Adam>(actor->parameters(), lr)),
      critic(std::make_shared<CriticLiquidModule>(state_space, neuron_number, unfolding_steps)),
      critic_optimizer(std::make_shared<torch::optim::Adam>(critic->parameters(), lr)),
      gamma(gamma), entropy_start_factor(entropy_start_factor),
      entropy_end_factor(entropy_end_factor), entropy_steps(entropy_steps),
      curr_device(torch::kCPU), batch_size(batch_size), replay_buffer(replay_buffer_size, seed),
      policy_loss_meter("policy", 64), entropy_meter("entropy", 64),
      critic_loss_meter("critic", 64), episode_steps_meter("steps", 64), curr_episode_step(0),
      curr_train_step(0L), global_curr_step(0L), train_every(train_every) {}

void ActorCriticLiquidAgent::check_train() {
    if (global_curr_step % train_every == train_every - 1) {
        std::vector<liquid_episode_step<liquid_a2c_memory>> tmp_replay_buffer =
            replay_buffer.sample(batch_size);

        std::vector<torch::Tensor> vec_states, vec_actions, vec_rewards, vec_done, vec_next_state,
            input_actor_x, input_critic_x, output_actor_x, output_critic_x;

        for (const auto &rp: tmp_replay_buffer) {
            vec_states.push_back(rp.replay_buffer.state);
            vec_actions.push_back(rp.replay_buffer.action);
            vec_rewards.push_back(
                torch::tensor(rp.replay_buffer.reward, at::TensorOptions().device(curr_device)));
            vec_done.push_back(torch::tensor(
                rp.replay_buffer.done ? 1.f : 0.f, at::TensorOptions().device(curr_device)));
            vec_next_state.push_back(rp.replay_buffer.next_state);

            input_actor_x.push_back(rp.x_t.actor_x_t);
            input_critic_x.push_back(rp.x_t.critic_x_t);

            output_actor_x.push_back(rp.next_x_t.actor_x_t);
            output_critic_x.push_back(rp.next_x_t.critic_x_t);
        }

        train(
            torch::stack(vec_states), torch::stack(vec_actions),
            torch::stack(vec_rewards).unsqueeze(1), torch::stack(vec_done).unsqueeze(1),
            torch::stack(vec_next_state), {torch::cat(input_actor_x), torch::cat(input_critic_x)},
            {torch::cat(output_actor_x), torch::cat(output_critic_x)});
    }
}

void ActorCriticLiquidAgent::train(
    const torch::Tensor &batched_states, const torch::Tensor &batched_actions,
    const torch::Tensor &batched_rewards, const torch::Tensor &batched_done,
    const torch::Tensor &batched_next_state, const liquid_a2c_memory &curr_memory,
    const liquid_a2c_memory &next_memory) {

    const auto [next_value, next_next_critic_x] =
        critic->forward(next_memory.critic_x_t.detach(), batched_next_state);
    const auto [value, next_critic_x] =
        critic->forward(curr_memory.critic_x_t.detach(), batched_states);

    const auto target = batched_rewards + (1.f - batched_done) * gamma * next_value;

    const auto critic_loss = torch::mse_loss(value, target, at::Reduction::Mean);

    critic_optimizer->zero_grad();
    critic_loss.backward();
    critic_optimizer->step();

    const auto [mu, sigma, next_actor_x] =
        actor->forward(curr_memory.actor_x_t.detach(), batched_states);
    const auto log_prob =
        torch::log(truncated_normal_pdf(batched_actions.detach(), mu, sigma, -1.f, 1.f));
    const auto policy_entropy =
        truncated_normal_entropy(mu, sigma, -1.f, 1.f)
        * exponential_decrease(
            curr_train_step, entropy_steps, entropy_start_factor, entropy_end_factor);
    const auto policy_loss = log_prob * (target - value).detach().unsqueeze(-1);

    const auto actor_loss = -torch::mean(policy_loss + policy_entropy);

    actor_optimizer->zero_grad();
    actor_loss.backward();
    actor_optimizer->step();

    policy_loss_meter.add(-policy_loss.sum(-1).mean().cpu().item().toFloat());
    entropy_meter.add(-policy_entropy.sum(-1).mean().cpu().item().toFloat());
    critic_loss_meter.add(critic_loss.cpu().item().toFloat());

    curr_train_step++;
}

torch::Tensor ActorCriticLiquidAgent::act(torch::Tensor state, float reward) {

    const liquid_a2c_memory input_memory{actor->get_x(), critic->get_x()};

    const auto [mu, sigma] = actor->forward(state);
    auto action = truncated_normal_sample(mu, sigma, -1.f, 1.f);
    const auto _ = critic->forward(state);

    const liquid_a2c_memory next_memory{actor->get_x(), critic->get_x()};

    if (!replay_buffer.empty()) { replay_buffer.update_last(reward, state, false); }
    replay_buffer.add({{state, action, 0.f, false, state}, input_memory, next_memory});

    curr_episode_step++;
    global_curr_step++;

    check_train();

    return action;
}

void ActorCriticLiquidAgent::done(torch::Tensor state, const float reward) {
    replay_buffer.update_last(reward, state, true);

    actor->reset_liquid();
    critic->reset_liquid();

    episode_steps_meter.add(static_cast<float>(curr_episode_step));
    curr_episode_step = 0;
}

void ActorCriticLiquidAgent::save(const std::string &output_folder_path) {
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

    // critic
    const auto critic_model_file = path / "critic.th";
    const auto critic_optimizer_file = path / "critic_optimizer.th";

    torch::serialize::OutputArchive critic_model_archive;
    torch::serialize::OutputArchive critic_optimizer_archive;

    critic->save(critic_model_archive);
    critic_optimizer->save(critic_optimizer_archive);

    critic_model_archive.save_to(critic_model_file);
    critic_optimizer_archive.save_to(critic_optimizer_file);
}

void ActorCriticLiquidAgent::load(const std::string &input_folder_path) {
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

    // critic
    const auto critic_model_file = path / "critic.th";
    const auto critic_optimizer_file = path / "critic_optimizer.th";

    torch::serialize::InputArchive critic_model_archive;
    torch::serialize::InputArchive critic_optimizer_archive;

    critic_model_archive.load_from(critic_model_file);
    critic_optimizer_archive.load_from(critic_optimizer_file);

    critic->load(critic_model_archive);
    critic_optimizer->load(critic_optimizer_archive);
}

std::vector<LossMeter> ActorCriticLiquidAgent::get_metrics() {
    return {policy_loss_meter, entropy_meter, critic_loss_meter, episode_steps_meter};
}

void ActorCriticLiquidAgent::to(const torch::DeviceType device) {
    curr_device = device;
    actor->to(curr_device);
    critic->to(curr_device);
}

void ActorCriticLiquidAgent::set_eval(const bool eval) {
    if (eval) {
        actor->eval();
        critic->eval();
    } else {
        actor->train();
        critic->train();
    }
}

int ActorCriticLiquidAgent::count_parameters() {
    int count = 0;
    for (const auto &p: actor->parameters()) {
        auto sizes = p.sizes();
        count += std::reduce(sizes.begin(), sizes.end(), 1, std::multiplies<>());
    }
    for (const auto &p: critic->parameters()) {
        auto sizes = p.sizes();
        count += std::reduce(sizes.begin(), sizes.end(), 1, std::multiplies<>());
    }
    return count;
}

/*
 * SAC
 */

/*SoftActorCriticLiquidAgent::SoftActorCriticLiquidAgent(
    int seed, const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
    int hidden_size, int batch_size, float lr, float gamma, float tau, int unfolding_steps)
    : SoftActorCriticAgent(
          seed, state_space, action_space, hidden_size, batch_size, lr, gamma, tau) {

    actor = std::make_shared<ActorLiquidModule>(
        state_space, action_space, hidden_size, unfolding_steps);
    actor_optimizer = std::make_shared<torch::optim::Adam>(actor->parameters(), lr);

    critic_1 = std::make_shared<QNetworkLiquidModule>(
        state_space, action_space, hidden_size, unfolding_steps);
    critic_1_optimizer = std::make_shared<torch::optim::Adam>(critic_1->parameters(), lr);

    critic_2 = std::make_shared<QNetworkLiquidModule>(
        state_space, action_space, hidden_size, unfolding_steps);
    critic_2_optimizer = std::make_shared<torch::optim::Adam>(critic_2->parameters(), lr);

    target_critic_1 = std::make_shared<QNetworkLiquidModule>(
        state_space, action_space, hidden_size, unfolding_steps);

    target_critic_2 = std::make_shared<QNetworkLiquidModule>(
        state_space, action_space, hidden_size, unfolding_steps);

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

void SoftActorCriticLiquidAgent::done(const torch::Tensor state, const float reward) {
    SoftActorCriticAgent::done(state, reward);

    std::dynamic_pointer_cast<ActorLiquidModule>(actor)->reset_liquid();
    std::dynamic_pointer_cast<QNetworkLiquidModule>(critic_1)->reset_liquid();
    std::dynamic_pointer_cast<QNetworkLiquidModule>(critic_2)->reset_liquid();
    std::dynamic_pointer_cast<QNetworkLiquidModule>(target_critic_1)->reset_liquid();
    std::dynamic_pointer_cast<QNetworkLiquidModule>(target_critic_2)->reset_liquid();
}*/
