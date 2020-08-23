#pragma once

#include "setcoveringsolver/solution.hpp"

namespace setcoveringsolver
{

struct LocalSearchOptionalParameters
{
    Counter thread_number = 3;
    Info info = Info();
};

struct LocalSearchOutput: Output
{
    LocalSearchOutput(const Instance& instance, Info& info): Output(instance, info) { }
    LocalSearchOutput& algorithm_end(Info& info);
};

LocalSearchOutput localsearch(
        Instance& instance,
        std::mt19937_64& generator,
        LocalSearchOptionalParameters parameters = {});

/******************************************************************************/

struct LocalSearch2OptionalParameters
{
    Counter thread_number = 3;
    Info info = Info();
};

struct LocalSearch2Output: Output
{
    LocalSearch2Output(const Instance& instance, Info& info): Output(instance, info) { }
    LocalSearch2Output& algorithm_end(Info& info);
};

LocalSearch2Output localsearch_2(
        Instance& instance,
        std::mt19937_64& generator,
        LocalSearch2OptionalParameters parameters = {});

}

