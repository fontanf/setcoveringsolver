#pragma once

#include "setcoveringsolver/algorithm.hpp"

namespace setcoveringsolver
{

struct CpSatOrtoolsParameters: Parameters
{
    /** Initial solution. */
    const Solution* initial_solution = NULL;
};

const Output cp_sat_ortools(
        const Instance& instance,
        const CpSatOrtoolsParameters& parameters = {});

}
