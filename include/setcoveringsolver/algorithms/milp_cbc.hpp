#pragma once

#include "setcoveringsolver/algorithm.hpp"

#include <coin/CbcModel.hpp>
#include <coin/OsiCbcSolverInterface.hpp>

namespace setcoveringsolver
{

struct MilpCbcParameters: Parameters
{
    /** Initial solution. */
    const Solution* initial_solution = NULL;
};

const Output milp_cbc(
        const Instance& instance,
        const MilpCbcParameters& parameters = {});

}
