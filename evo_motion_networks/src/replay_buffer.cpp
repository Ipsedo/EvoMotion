//
// Created by samuel on 11/11/24.
//

#include <evo_motion_networks/replay_buffer.h>

/*
 * Abstract replay buffer
 */

template<class ReplayBufferType, class... UpdateArgs>
AbstractReplayBuffer<ReplayBufferType, UpdateArgs...>::AbstractReplayBuffer(
    const int size, const int seed)
    : size(size), memory(), rand_gen(seed) {}

template<class ReplayBufferType, class... UpdateArgs>
std::vector<ReplayBufferType>
AbstractReplayBuffer<ReplayBufferType, UpdateArgs...>::sample(const int batch_size) {
    std::vector<ReplayBufferType> tmp_replay_buffer(memory);
    std::shuffle(tmp_replay_buffer.begin(), tmp_replay_buffer.end(), rand_gen);

    std::vector<ReplayBufferType> result;
    for (int i = 0; i < batch_size && i < tmp_replay_buffer.size(); i++)
        result.push_back(tmp_replay_buffer[i]);

    return result;
}

template<class ReplayBufferType, class... UpdateArgs>
void AbstractReplayBuffer<ReplayBufferType, UpdateArgs...>::add(ReplayBufferType buffer) {
    memory.push_back(buffer);

    while (memory.size() > size) memory.erase(memory.begin());
}

template<class ReplayBufferType, class... UpdateArgs>
void AbstractReplayBuffer<ReplayBufferType, UpdateArgs...>::update_last(UpdateArgs... args) {
    ReplayBufferType last = memory.back();
    memory.erase(memory.end());
    memory.push_back(update_last_item(last, args...));
}

template<class ReplayBufferType, class... UpdateArgs>
bool AbstractReplayBuffer<ReplayBufferType, UpdateArgs...>::empty() {
    return memory.empty();
}

template<class ReplayBufferType, class... UpdateArgs>
AbstractReplayBuffer<ReplayBufferType, UpdateArgs...>::~AbstractReplayBuffer() {
    memory.clear();
}

/*
 * Abstract trajectory replay buffer
 */

template<typename EpisodeStep, class... UpdateArgs>
AbstractTrajectoryBuffer<EpisodeStep, UpdateArgs...>::AbstractTrajectoryBuffer(
    const int size, const int seed)
    : size(size), memory(), rand_gen(seed) {}

template<typename EpisodeStep, class... UpdateArgs>
episode_trajectory<EpisodeStep> AbstractTrajectoryBuffer<EpisodeStep, UpdateArgs...>::sample() {
    return sample(1)[0];
}

template<typename EpisodeStep, class... UpdateArgs>
std::vector<episode_trajectory<EpisodeStep>>
AbstractTrajectoryBuffer<EpisodeStep, UpdateArgs...>::sample(const int batch_size) {
    std::vector<episode_trajectory<EpisodeStep>> filtered;
    std::copy_if(memory.begin(), memory.end(), std::back_inserter(filtered), [](auto t) {
        return t.trajectory.size() > 1;
    });

    std::vector<int> index(filtered.size() - 1);
    std::iota(index.begin(), index.end(), 0);
    std::shuffle(index.begin(), index.end(), rand_gen);

    std::vector<episode_trajectory<EpisodeStep>> result;

    for (int i = 0; i < batch_size && i < index.size(); i++) {
        const auto trajectory = filtered[index[i]];
        result.push_back(trajectory);
    }

    return result;
}

template<typename EpisodeStep, class... UpdateArgs>
episode_trajectory<EpisodeStep> AbstractTrajectoryBuffer<EpisodeStep, UpdateArgs...>::last() {
    return memory.back();
}

template<typename EpisodeStep, class... UpdateArgs>
void AbstractTrajectoryBuffer<EpisodeStep, UpdateArgs...>::new_trajectory() {
    memory.push_back({});

    while (memory.size() > size) memory.erase(memory.begin());
}

template<typename EpisodeStep, class... UpdateArgs>
void AbstractTrajectoryBuffer<EpisodeStep, UpdateArgs...>::add(EpisodeStep step) {
    memory.back().trajectory.push_back(step);
}

template<typename EpisodeStep, class... UpdateArgs>
void AbstractTrajectoryBuffer<EpisodeStep, UpdateArgs...>::update_last(UpdateArgs... args) {
    auto last_step = memory.back().trajectory.back();
    memory.back().trajectory.erase(memory.back().trajectory.end());
    memory.back().trajectory.push_back(update_last_step(last_step, args...));
}

template<typename EpisodeStep, class... UpdateArgs>
bool AbstractTrajectoryBuffer<EpisodeStep, UpdateArgs...>::empty() {
    return memory.empty();
}

template<typename EpisodeStep, class... UpdateArgs>
bool AbstractTrajectoryBuffer<EpisodeStep, UpdateArgs...>::trajectory_empty() {
    return empty() || memory.back().trajectory.empty();
}

template<typename EpisodeStep, class... UpdateArgs>
AbstractTrajectoryBuffer<EpisodeStep, UpdateArgs...>::~AbstractTrajectoryBuffer() {
    memory.clear();
}

template<typename EpisodeStep, class... UpdateArgs>
bool AbstractTrajectoryBuffer<EpisodeStep, UpdateArgs...>::enough_trajectory(int batch_size) {
    std::vector<episode_trajectory<EpisodeStep>> filtered;
    std::copy_if(memory.begin(), memory.end(), std::back_inserter(filtered), [](auto t) {
        return t.trajectory.size() > 1;
    });
    return filtered.size() >= batch_size;
}

/*
 * Linear module replay buffer
 */

ReplayBuffer::ReplayBuffer(const int size, const int seed) : AbstractReplayBuffer(size, seed) {}

episode_step ReplayBuffer::update_last_item(
    episode_step last_item, const float reward, const torch::Tensor next_state, const bool done) {
    last_item.reward = reward;
    last_item.next_state = next_state;
    last_item.done = done;

    return last_item;
}

/*
 * Liquid replay buffer
 */

template<typename LiquidMemory>
LiquidReplayBuffer<LiquidMemory>::LiquidReplayBuffer(int size, int seed)
    : AbstractReplayBuffer<liquid_episode_step<LiquidMemory>, float, torch::Tensor, bool>(
          size, seed) {}

template<typename LiquidMemory>
liquid_episode_step<LiquidMemory> LiquidReplayBuffer<LiquidMemory>::update_last_item(
    liquid_episode_step<LiquidMemory> last_item, float reward, torch::Tensor next_state,
    bool done) {
    last_item.replay_buffer.reward = reward;
    last_item.replay_buffer.next_state = next_state;
    last_item.replay_buffer.done = done;
    return last_item;
}

// Trajectory

TrajectoryReplayBuffer::TrajectoryReplayBuffer(const int size, const int seed)
    : AbstractTrajectoryBuffer(size, seed) {}

ppo_episode_step TrajectoryReplayBuffer::update_last_step(
    ppo_episode_step last_step, const float reward, const bool done,
    const torch::Tensor next_value) {
    last_step.reward = reward;
    last_step.done = done;
    last_step.next_value = next_value;

    return last_step;
}

bool TrajectoryReplayBuffer::is_finish(const ppo_episode_step step) { return step.done; }

template<typename EpisodeStep, class... UpdateArgs>
void AbstractTrajectoryBuffer<EpisodeStep, UpdateArgs...>::clear() {
    memory.clear();
}
