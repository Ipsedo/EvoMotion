//
// Created by samuel on 19/01/25.
//

#ifndef EVO_MOTION_UTILS_H
#define EVO_MOTION_UTILS_H

#include <algorithm>
#include <functional>
#include <vector>

template<typename Input, typename Output>
std::vector<Output> transform_vector(
    const std::vector<Input> &to_transform, std::function<Output(Input)> transform_function) {
    std::vector<Output> result;
    std::transform(
        to_transform.begin(), to_transform.end(), std::back_inserter(result), transform_function);
    return result;
}

#endif//EVO_MOTION_UTILS_H
