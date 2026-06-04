#pragma once

#include <string>

#include <gtest/gtest.h>

namespace tests::common
{

template<typename T>
std::string param_name(const ::testing::TestParamInfo<T>& info)
{
    return std::string(info.param.name);
}

}  // namespace tests::common
