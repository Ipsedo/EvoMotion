//
// Created by samuel on 11/08/19.
//

#ifndef EVOMOTION_RES_H
#define EVOMOTION_RES_H

#include <string>

#ifdef _WIN32
#define EVOMOTION_SEP '\\'
#else
#define EVOMOTION_SEP '/'
#endif

std::string init_exec_root(char *argv);

extern std::string exec_root;

#endif //EVOMOTION_RES_H
