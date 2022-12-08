//
// Created by samuel on 14/02/20.
//

#include "res.h"

#include "string_utils.h"

std::string init_exec_root(char *argv) {
    std::string exec_path(argv);

    auto path = split(exec_path, EVOMOTION_SEP);

    std::string res = path[0];
    for (auto elt : std::vector<std::string>(path.begin() + 1, path.end() - 1))
        res += EVOMOTION_SEP + elt;

    return res;
}
