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
        std::mt19937_64& generator,
        LargeNeighborhoodSearchOptionalParameters parameters = {});

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct LargeNeighborhoodSearch2OptionalParameters
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

struct LargeNeighborhoodSearch2Output: Output
{
    LargeNeighborhoodSearch2Output(
            const Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    LargeNeighborhoodSearch2Output& algorithm_end(
            optimizationtools::Info& info);

    /** Number of iterations. */
    Counter number_of_iterations = 0;
};

LargeNeighborhoodSearch2Output largeneighborhoodsearch_2(
        Instance& instance,
        LargeNeighborhoodSearch2OptionalParameters parameters = {});

}

