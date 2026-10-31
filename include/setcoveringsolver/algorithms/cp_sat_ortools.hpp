#pragma once

#include "setcoveringsolver/algorithm.hpp"

namespace setcoveringsolver
{

struct CpSatOrtoolsParameters: Parameters
{
};

const Output cp_sat_ortools(
        const Instance& instance,
        const Solution* initial_solution = nullptr,
        const CpSatOrtoolsParameters& parameters = {});

}
