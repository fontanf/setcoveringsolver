#pragma once

#if GUROBI_FOUND

#include "setcoveringsolver/solution.hpp"

namespace setcoveringsolver
{

struct MilpGurobiOptionalParameters
{
    /** Initial solution. */
    const Solution* initial_solution = NULL;

    /** Reduction parameters. */
    ReductionParameters reduction_parameters;

    /** Info structure. */
    optimizationtools::Info info = optimizationtools::Info();
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
