//
// Created by samuel on 10/11/24.
//

#ifndef EVO_MOTION_TEST_LIQUID_MODULES_H
#define EVO_MOTION_TEST_LIQUID_MODULES_H

#include <gtest/gtest.h>

class ParamLiquidCellModule : public testing::TestWithParam<std::tuple<int, int, int, int>> {};

class ParamLiquidModule : public testing::TestWithParam<std::tuple<int, int, int, int>> {};

#endif//EVO_MOTION_TEST_LIQUID_MODULES_H
