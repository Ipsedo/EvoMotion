//
// Created by samuel on 20/12/22.
//

#include <numeric>

#include "./networks/metrics.h"

template<class R, class... I>
Meter<R, I...>::Meter(std::optional<int> window_size) : window_size(window_size), results() {

}

template<class R, class... I>
void Meter<R, I...>::add(I... inputs) {
    if (window_size.has_value() && results.size() >= window_size)
        results.erase(results.begin());

    results.push_back(process_value(inputs...));
}

template<class R, class... I>
void Meter<R, I...>::set_window_size(std::optional<int> new_window_size) {
    window_size = new_window_size;
}

LossMeter::LossMeter(std::optional<int> window_size) : Meter(window_size) {

}



float LossMeter::loss() {
    float sum = std::accumulate(results.begin(), results.end(), 0.f);
    return sum / (float) results.size();
}

float LossMeter::process_value(float value) {
    return value;
}
