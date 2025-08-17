#pragma once

#include "setcoveringsolver/algorithm.hpp"

namespace setcoveringsolver
{

#ifdef CBC_FOUND

const Output milp_cbc(
        const Instance& instance,
        const Solution* initial_solution = NULL,
        const Parameters& parameters = {});

#endif

#ifdef HIGHS_FOUND

const Output milp_highs(
        const Instance& instance,
        const Solution* initial_solution = NULL,
        const Parameters& parameters = {});

#endif

#ifdef XPRESS_FOUND

const Output milp_xpress(
        const Instance& instance,
        const Solution* initial_solution = NULL,
        const Parameters& parameters = {});

#endif

}
