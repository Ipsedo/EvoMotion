//
// Created by samuel on 10/11/24.
//

#ifndef EVO_MOTION_TEST_AGENTS_H
#define EVO_MOTION_TEST_AGENTS_H

#include <gtest/gtest.h>

class ParamActorCriticAgent : public testing::TestWithParam<std::tuple<int, int, int, int>> {};

#endif//EVO_MOTION_TEST_AGENTS_H
