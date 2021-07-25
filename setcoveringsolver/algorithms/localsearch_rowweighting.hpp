#pragma once

#include "setcoveringsolver/solution.hpp"

namespace setcoveringsolver
{

struct LocalSearchRowWeightingOptionalParameters
{
    Counter thread_number = 3;
    Info info = Info();
};

struct LocalSearchRowWeightingOutput: Output
{
    LocalSearchRowWeightingOutput(const Instance& instance, Info& info): Output(instance, info) { }
    LocalSearchRowWeightingOutput& algorithm_end(Info& info);
};

LocalSearchRowWeightingOutput localsearch_rowweighting(
        Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeightingOptionalParameters parameters = {});

/******************************************************************************/

struct LocalSearchRowWeighting2OptionalParameters
{
    Counter thread_number = 3;
    Info info = Info();
};

struct LocalSearchRowWeighting2Output: Output
{
    LocalSearchRowWeighting2Output(const Instance& instance, Info& info): Output(instance, info) { }
    LocalSearchRowWeighting2Output& algorithm_end(Info& info);
};

LocalSearchRowWeighting2Output localsearch_rowweighting_2(
        Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeighting2OptionalParameters parameters = {});

}

