#pragma once

#if CBC_FOUND

#include "setcoveringsolver/setcovering/solution.hpp"

#include <CbcModel.hpp>
#include <OsiCbcSolverInterface.hpp>

namespace setcoveringsolver
{
namespace setcovering
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
}

#endif

