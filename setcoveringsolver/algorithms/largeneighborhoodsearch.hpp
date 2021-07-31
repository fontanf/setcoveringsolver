#pragma once

#include "setcoveringsolver/solution.hpp"

namespace setcoveringsolver
{

struct LargeNeighborhoodSearchOptionalParameters
{
    Info info = Info();

    Counter iteration_limit = -1;
    Counter iteration_without_improvment_limit = -1;
};

struct LargeNeighborhoodSearchOutput: Output
{
    LargeNeighborhoodSearchOutput(const Instance& instance, Info& info): Output(instance, info) { }
    LargeNeighborhoodSearchOutput& algorithm_end(Info& info);

    Counter iterations = 0;
};

LargeNeighborhoodSearchOutput largeneighborhoodsearch(
        Instance& instance,
        std::mt19937_64& generator,
        LargeNeighborhoodSearchOptionalParameters parameters = {});

/******************************************************************************/

struct LargeNeighborhoodSearch2OptionalParameters
{
    Info info = Info();

    Counter number_of_threads = 3;
    Counter iteration_limit = -1;
    Counter iteration_without_improvment_limit = -1;
};

struct LargeNeighborhoodSearch2Output: Output
{
    LargeNeighborhoodSearch2Output(const Instance& instance, Info& info): Output(instance, info) { }
    LargeNeighborhoodSearch2Output& algorithm_end(Info& info);

    Counter iterations = 0;
};

LargeNeighborhoodSearch2Output largeneighborhoodsearch_2(
        Instance& instance,
        LargeNeighborhoodSearch2OptionalParameters parameters = {});

}

