#pragma once

#if GUROBI_FOUND

#include "setcoveringsolver/solution.hpp"

namespace setcoveringsolver
{

struct MilpGurobiOptionalParameters
{
    optimizationtools::Info info = optimizationtools::Info();

    const Solution* initial_solution = NULL;
};

struct MilpGurobiOutput: Output
{
    MilpGurobiOutput(
            const Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    MilpGurobiOutput& algorithm_end(optimizationtools::Info& info);
};

MilpGurobiOutput milp_gurobi(
        const Instance& instance,
        MilpGurobiOptionalParameters parameters = {});

}

#endif
