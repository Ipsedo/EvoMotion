//
// Created by samuel on 20/12/22.
//

#include <fstream>
#include <numeric>
#include <utility>

#include <evo_motion_networks/metrics.h>

template<class R, class... I>
Meter<R, I...>::Meter(std::string name, const std::optional<int> window_size)
    : csv_sep(','), name(std::move(name)), window_size(window_size), curr_step(0L), results() {}

template<class R, class... I>
void Meter<R, I...>::add(I... inputs) {
    while (window_size.has_value() && results.size() >= window_size) results.erase(results.begin());

    results.push_back(process_value(inputs...));
    curr_step++;
}

template<class R, class... I>
R Meter<R, I...>::loss() {
    R sum = std::accumulate(results.begin(), results.end(), 0);
    return sum / results.size();
}

template<class R, class... I>
void Meter<R, I...>::set_window_size(const std::optional<int> new_window_size) {
    window_size = new_window_size;
}

template<class R, class... I>
void Meter<R, I...>::to_csv(const std::filesystem::path &output_directory) {
    auto csv_path = output_directory / (name + ".csv");

    if (!std::filesystem::exists(csv_path)) {
        std::ofstream csv_file(csv_path.string());
        csv_file << "step" << csv_sep << "loss" << std::endl;
        csv_file.close();
    }

    std::ofstream csv_file(csv_path.string());
    csv_file << curr_step << csv_sep << loss_to_string(loss()) << std::endl;
    csv_file.close();
}

template<class R, class... I>
Meter<R, I...>::~Meter() = default;

/*
 * Single value loss
 */

LossMeter::LossMeter(const std::string &name, const std::optional<int> window_size)
    : Meter(name, window_size) {}

float LossMeter::process_value(const float value) { return value; }

std::string LossMeter::loss_to_string(const float loss_value) { return std::to_string(loss_value); }

LossMeter::~LossMeter() = default;
