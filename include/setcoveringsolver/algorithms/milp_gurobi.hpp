#pragma once

#include "setcoveringsolver/algorithm.hpp"

namespace setcoveringsolver
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
