//
// Created by samuel on 11/11/24.
//

#ifndef EVO_MOTION_ACTOR_H
#define EVO_MOTION_ACTOR_H

#include <torch/torch.h>

#include <evo_motion_networks/networks/liquid.h>

// responses

struct actor_response {
    torch::Tensor mu;
    torch::Tensor sigma;
};

struct liquid_actor_response {
    torch::Tensor mu;
    torch::Tensor sigma;
    torch::Tensor new_x_t;
};

// abstract modules

class AbstractActor : public torch::nn::Module {
public:
    virtual actor_response forward(const torch::Tensor &state) = 0;
};

// module

class ActorModule final : public AbstractActor {
public:
    ActorModule(
        std::vector<int64_t> state_space, std::vector<int64_t> action_space, int hidden_size);

    actor_response forward(const torch::Tensor &state) override;

private:
    torch::nn::Sequential head{nullptr};

    torch::nn::Sequential mu{nullptr};
    torch::nn::Sequential sigma{nullptr};
};

class ExpModule final : public torch::nn::Module {
public:
    explicit ExpModule(float epsilon = 0.f);
    torch::Tensor forward(const torch::Tensor &input);

private:
    float epsilon;
};

class SoftPlusEpsModule final : public torch::nn::Module {
public:
    explicit SoftPlusEpsModule(float epsilon = 0.f);
    torch::Tensor forward(const torch::Tensor &input);

private:
    float epsilon;
};

// liquid

class ActorLiquidModule final : public AbstractActor {
public:
    ActorLiquidModule(
        const std::vector<int64_t> &state_space, std::vector<int64_t> action_space, int hidden_size,
        int unfolding_steps);

    void reset_liquid() const;
    torch::Tensor get_x() const;

    actor_response forward(const torch::Tensor &state) override;
    liquid_actor_response forward(const torch::Tensor &x_t, const torch::Tensor &state);

private:
    std::shared_ptr<LiquidCellModule> liquid_network;

    torch::nn::Sequential mu{nullptr};
    torch::nn::Sequential sigma{nullptr};
};

#endif//EVO_MOTION_ACTOR_H
