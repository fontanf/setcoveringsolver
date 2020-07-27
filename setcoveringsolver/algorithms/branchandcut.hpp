#pragma once

#include "setcoveringsolver/solution.hpp"

namespace setcoveringsolver
{

struct BranchAndCutOptionalParameters
{
    Info info = Info();

    const Solution* initial_solution = NULL;
};

struct BranchAndCutOutput: Output
{
    BranchAndCutOutput(const Instance& instance, Info& info): Output(instance, info) { }
    BranchAndCutOutput& algorithm_end(Info& info);
};

BranchAndCutOutput branchandcut(
        const Instance& instance, BranchAndCutOptionalParameters p = {});

}

