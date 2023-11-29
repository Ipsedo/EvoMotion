//
// Created by samuel on 20/12/22.
//

#ifndef EVO_MOTION_METRICS_H
#define EVO_MOTION_METRICS_H

#include <optional>
#include <vector>

template<class R, class... I>
class Meter {
private:
    std::optional<int> window_size;

protected:
    std::vector<R> results;

    virtual R process_value(I... inputs) = 0;

public:
    explicit Meter(std::optional<int> window_size);

    virtual void add(I... inputs);

    virtual void set_window_size(std::optional<int> new_window_size);
};


class LossMeter : public Meter<float, float> {
public:
    explicit LossMeter(std::optional<int> window_size);

    float loss();

protected:
    float process_value(float value) override;
};

#endif //EVO_MOTION_METRICS_H
