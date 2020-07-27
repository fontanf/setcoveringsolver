#pragma once

#include "setcoveringsolver/solution.hpp"

namespace setcoveringsolver
{

struct LargeNeighborhoodSearchOptionalParameters
{
    Info info = Info();
};

struct LargeNeighborhoodSearchOutput: Output
{
    LargeNeighborhoodSearchOutput(const Instance& instance, Info& info): Output(instance, info) { }
    LargeNeighborhoodSearchOutput& algorithm_end(Info& info);

    Counter iterations = 0;
};

LargeNeighborhoodSearchOutput largeneighborhoodsearch(
        const Instance& instance,
        std::mt19937_64& generator,
        LargeNeighborhoodSearchOptionalParameters parameters = {});

}


