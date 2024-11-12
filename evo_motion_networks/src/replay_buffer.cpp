//
// Created by samuel on 11/11/24.
//

#include <evo_motion_networks/replay_buffer.h>

/*
 * Abstract replay buffer
 */

template<class ReplayBufferType, class... UpdateArgs>
AbstractReplayBuffer<ReplayBufferType, UpdateArgs...>::AbstractReplayBuffer(int size, int seed)
    : size(size), memory(), rand_gen(seed) {}

template<class ReplayBufferType, class... UpdateArgs>
std::vector<ReplayBufferType>
AbstractReplayBuffer<ReplayBufferType, UpdateArgs...>::sample(int batch_size) {
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

/*
 * Linear module replay buffer
 */

ReplayBuffer::ReplayBuffer(int size, int seed) : AbstractReplayBuffer(size, seed) {}

episode_step ReplayBuffer::update_last_item(
    episode_step last_item, float reward, torch::Tensor next_state, bool done) {
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
