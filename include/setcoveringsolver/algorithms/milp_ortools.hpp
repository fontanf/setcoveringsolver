#pragma once

#include "setcoveringsolver/algorithm.hpp"

namespace setcoveringsolver
{

struct MilpOrtoolsParameters: Parameters
{
    /** Initial solution. */
    const Solution* initial_solution = NULL;
};

const Output milp_ortools(
        const Instance& instance,
        const MilpOrtoolsParameters& parameters = {});

}
