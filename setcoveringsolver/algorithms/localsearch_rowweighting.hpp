#pragma once

#include "setcoveringsolver/solution.hpp"

namespace setcoveringsolver
{

struct LocalSearchRowWeightingOptionalParameters
{
    Counter number_of_threads = 3;
    optimizationtools::Info info = optimizationtools::Info();
};

struct LocalSearchRowWeightingOutput: Output
{
    LocalSearchRowWeightingOutput(
            const Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    LocalSearchRowWeightingOutput& algorithm_end(
            optimizationtools::Info& info);
};

LocalSearchRowWeightingOutput localsearch_rowweighting(
        Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeightingOptionalParameters parameters = {});

/******************************************************************************/

struct LocalSearchRowWeighting2OptionalParameters
{
    Counter number_of_threads = 3;
    optimizationtools::Info info = optimizationtools::Info();
};

struct LocalSearchRowWeighting2Output: Output
{
    LocalSearchRowWeighting2Output(
            const Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    LocalSearchRowWeighting2Output& algorithm_end(
            optimizationtools::Info& info);
};

LocalSearchRowWeighting2Output localsearch_rowweighting_2(
        Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeighting2OptionalParameters parameters = {});

}

