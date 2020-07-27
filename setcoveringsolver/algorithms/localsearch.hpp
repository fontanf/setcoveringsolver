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

}

