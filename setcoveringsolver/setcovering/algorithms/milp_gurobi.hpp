#pragma once

#if GUROBI_FOUND

#include "setcoveringsolver/setcovering/solution.hpp"

namespace setcoveringsolver
{
namespace setcovering
{

struct MilpGurobiParameters: Parameters
{
    /** Initial solution. */
    const Solution* initial_solution = NULL;
};

const Output milp_gurobi(
        const Instance& instance,
        const MilpGurobiParameters& parameters = {});

}
}

#endif
