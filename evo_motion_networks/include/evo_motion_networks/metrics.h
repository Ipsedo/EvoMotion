//
// Created by samuel on 20/12/22.
//

#ifndef EVO_MOTION_METRICS_H
#define EVO_MOTION_METRICS_H

#include <filesystem>
#include <optional>
#include <vector>

template<class R, class... I>
class Meter {
private:
    const char csv_sep;
    std::string name;
    std::optional<int> window_size;
    long curr_step;

protected:
    std::vector<R> results;

    virtual R process_value(I... inputs) = 0;

    virtual std::string loss_to_string(R loss_value) = 0;

public:
    explicit Meter(const std::string &name, std::optional<int> window_size);

    virtual void add(I... inputs);

    virtual void set_window_size(std::optional<int> new_window_size);

    void to_csv(const std::filesystem::path &output_directory);

    virtual R loss();
};

class LossMeter : public Meter<float, float> {
public:
    explicit LossMeter(const std::string &name, std::optional<int> window_size);

protected:
    float process_value(float value) override;
    std::string loss_to_string(float loss_value) override;
};

#endif//EVO_MOTION_METRICS_H
