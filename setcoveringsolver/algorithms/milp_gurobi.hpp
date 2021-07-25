#pragma once

#if GUROBI_FOUND

#include "setcoveringsolver/solution.hpp"

namespace setcoveringsolver
{

struct MilpGurobiOptionalParameters
{
    Info info = Info();

    const Solution* initial_solution = NULL;
};

struct MilpGurobiOutput: Output
{
    MilpGurobiOutput(const Instance& instance, Info& info): Output(instance, info) { }
    MilpGurobiOutput& algorithm_end(Info& info);
};

MilpGurobiOutput milp_gurobi(
        const Instance& instance, MilpGurobiOptionalParameters p = {});

}

#endif
