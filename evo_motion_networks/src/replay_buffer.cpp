//
// Created by samuel on 11/11/24.
//

#include <evo_motion_networks/replay_buffer.h>

/*
 * Abstract replay buffer
 */

template<typename ReplayBufferType, class... UpdateArgs>
AbstractReplayBuffer<ReplayBufferType, UpdateArgs...>::AbstractReplayBuffer(int size, int seed)
    : size(size), memory(), rand_gen(seed) {}

template<typename ReplayBufferType, class... UpdateArgs>
std::vector<ReplayBufferType>
AbstractReplayBuffer<ReplayBufferType, UpdateArgs...>::sample(int batch_size) {
    std::vector<ReplayBufferType> tmp_replay_buffer(memory);
    std::shuffle(tmp_replay_buffer.begin(), tmp_replay_buffer.end(), rand_gen);

    std::vector<ReplayBufferType> result;
    for (int i = 0; i < batch_size; i++) result.push_back(tmp_replay_buffer[i]);

    return result;
}

template<typename ReplayBufferType, class... UpdateArgs>
void AbstractReplayBuffer<ReplayBufferType, UpdateArgs...>::add(ReplayBufferType buffer) {
    memory.push_back(buffer);

    while (memory.size() > size) memory.erase(memory.begin());
}

template<typename ReplayBufferType, class... UpdateArgs>
void AbstractReplayBuffer<ReplayBufferType, UpdateArgs...>::update_last(UpdateArgs... args) {
    ReplayBufferType last = memory.back();
    memory.erase(memory.end());
    memory.push_back(update_last_item(last, args...));
}

template<typename ReplayBufferType, class... UpdateArgs>
bool AbstractReplayBuffer<ReplayBufferType, UpdateArgs...>::empty() {
    return memory.empty();
}

/*
 * Linear module replay buffer
 */

ReplayBuffer::ReplayBuffer(int size, int seed) : AbstractReplayBuffer(size, seed) {}

step_replay_buffer ReplayBuffer::update_last_item(
    step_replay_buffer last_item, float reward, torch::Tensor next_state, bool done) {
    last_item.reward = reward;
    last_item.next_state = next_state;
    last_item.done = done;

    return last_item;
}

/*void ReplayBuffer::update_last(float reward, torch::Tensor next_state, bool done) {
    AbstractReplayBuffer::update_last(reward, next_state, done);
}*/

/*
 * Liquid replay buffer
 */

LiquidReplayBuffer::LiquidReplayBuffer(int size, int seed) : AbstractReplayBuffer(size, seed) {}

liquid_step_replay_buffer LiquidReplayBuffer::update_last_item(
    liquid_step_replay_buffer last_item, float reward, torch::Tensor next_state, bool done) {
    last_item.reward = reward;
    last_item.next_state = next_state;
    last_item.done = done;
    return last_item;
}

/*void LiquidReplayBuffer::update_last(float reward, torch::Tensor next_state, bool done) {
    AbstractReplayBuffer::update_last(reward, next_state, done);
}*/
