//
// Created by samuel on 10/01/25.
//

#include <evo_motion_networks/networks/norm.h>

BatchRenormalization::BatchRenormalization(
    int num_features, const float epsilon, const float momentum, const bool affine, const int warmup_steps)
    : running_mean(register_buffer("running_mean", torch::zeros({num_features}))),
      running_std(register_buffer("running_std", torch::ones({num_features}))),
      weight(register_parameter("weight", torch::ones({num_features}))),
      bias(register_parameter("bias", torch::zeros({num_features}))), epsilon(epsilon),
      momentum(momentum), affine(affine), curr_step(0L), warmup_steps(warmup_steps),
      r_max_init(1.0), r_max_end(3.0), d_max_init(0.0), d_max_end(5.0) {}

torch::Tensor BatchRenormalization::forward(const torch::Tensor &x) {
    torch::Tensor out;

    if (is_training()) {
        const auto batch_mean = x.mean(0);
        const auto batch_std = x.std(0, false) + epsilon;

        const auto r =
            torch::clamp(batch_std.detach() / running_std, 1.0 / r_max(), r_max()).detach();
        const auto d =
            torch::clamp((batch_mean.detach() - running_mean) / (running_std + epsilon), -d_max(), d_max())
                .detach();

        out = (x - batch_mean) / batch_std * r + d;

        running_mean = (1.f - momentum) * running_mean + momentum * batch_mean.detach();
        running_std = (1.f - momentum) * running_std + momentum * batch_std.detach();

        curr_step = std::min(curr_step + 1, warmup_steps);
    } else {
        out = (x - running_mean) / (running_std + epsilon);
    }

    if (affine) out = weight * out + bias;

    return out;
}

float BatchRenormalization::r_max() const {
    return r_max_init
           + static_cast<float>(curr_step) * (r_max_end - r_max_init)
                 / static_cast<float>(warmup_steps);
}

float BatchRenormalization::d_max() const {
    return d_max_init
           + static_cast<float>(curr_step) * (d_max_end - d_max_init)
                 / static_cast<float>(warmup_steps);
}