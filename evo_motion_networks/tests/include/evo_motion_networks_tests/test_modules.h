//
// Created by samuel on 10/01/25.
//

#ifndef EVO_MOTION_TEST_MODULES_H
#define EVO_MOTION_TEST_MODULES_H

#include <gtest/gtest.h>

class BatchRenormalizationParam : public testing::TestWithParam<std::tuple<int, int, bool, int>> {};

#endif//EVO_MOTION_TEST_MODULES_H
