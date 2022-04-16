#pragma once

#include "setcoveringsolver/solution.hpp"

namespace setcoveringsolver
{

struct LocalSearchRowWeightingOptionalParameters
{
    /** Maximum number of iterations. */
    Counter maximum_number_of_iterations = -1;
    /** Maximum number of iterations without improvement. */
    Counter maximum_number_of_iterations_without_improvement = -1;
    /** Info structure. */
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

    /** Number of iterations. */
    Counter number_of_iterations = 0;
};

LocalSearchRowWeightingOutput localsearch_rowweighting(
        Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeightingOptionalParameters parameters = {});

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct LocalSearchRowWeighting2OptionalParameters
{
    /** Maximum number of iterations. */
    Counter maximum_number_of_iterations = -1;
    /** Maximum number of iterations without improvement. */
    Counter maximum_number_of_iterations_without_improvement = -1;
    /** Info structure. */
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

    /** Number of iterations. */
    Counter number_of_iterations = 0;
};

LocalSearchRowWeighting2Output localsearch_rowweighting_2(
        Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeighting2OptionalParameters parameters = {});

}

