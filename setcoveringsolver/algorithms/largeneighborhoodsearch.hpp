#pragma once

#include "setcoveringsolver/solution.hpp"

namespace setcoveringsolver
{

struct LargeNeighborhoodSearchOptionalParameters
{
    /** Maximum number of iterations. */
    Counter maximum_number_of_iterations = -1;

    /** Maximum number of iterations without improvement. */
    Counter maximum_number_of_iterations_without_improvement = -1;

    /** Goal. */
    Cost goal;

    /** Info structure. */
    optimizationtools::Info info = optimizationtools::Info();
};

struct LargeNeighborhoodSearchOutput: Output
{
    LargeNeighborhoodSearchOutput(
            const Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    LargeNeighborhoodSearchOutput& algorithm_end(
            optimizationtools::Info& info);

    /** Number of iterations. */
    Counter number_of_iterations = 0;
};

LargeNeighborhoodSearchOutput largeneighborhoodsearch(
        Instance& instance,
        LargeNeighborhoodSearchOptionalParameters parameters = {});

}

