//
// Created by samuel on 20/12/22.
//

#include <fstream>
#include <numeric>

#include <evo_motion_networks/metrics.h>

template<class R, class... I>
Meter<R, I...>::Meter(const std::string &name, std::optional<int> window_size)
    : csv_sep(','), name(name), window_size(window_size), results(), curr_step(0L) {}

template<class R, class... I>
void Meter<R, I...>::add(I... inputs) {
    if (window_size.has_value() && results.size() >= window_size) results.erase(results.begin());

    results.push_back(process_value(inputs...));
    curr_step++;
}

template<class R, class... I>
R Meter<R, I...>::loss() {
    R sum = std::accumulate(results.begin(), results.end(), 0);
    return sum / results.size();
}

template<class R, class... I>
void Meter<R, I...>::set_window_size(std::optional<int> new_window_size) {
    window_size = new_window_size;
}

template<class R, class... I>
void Meter<R, I...>::to_csv(const std::filesystem::path &output_directory) {
    auto csv_path = output_directory / (name + ".csv");

    if (!std::filesystem::exists(csv_path)) {
        std::ofstream csv_file(csv_path.string());
        csv_file << "step" << std::string(csv_sep) << "loss" << std::endl;
        csv_file.close();
    }

    std::ofstream csv_file(csv_path.string());
    csv_file << std::string(curr_step) + std::string(csv_sep) + loss_to_string(loss())
             << std::endl;
    csv_file.close();
}

/*
 * Single value loss
 */

LossMeter::LossMeter(const std::string &name, std::optional<int> window_size)
    : Meter(name, window_size) {}

float LossMeter::process_value(float value) { return value; }

std::string LossMeter::loss_to_string(float loss_value) { return std::to_string(loss_value); }
