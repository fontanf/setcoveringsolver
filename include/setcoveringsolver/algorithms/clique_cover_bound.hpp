#pragma once

#include "setcoveringsolver/algorithm.hpp"

namespace setcoveringsolver
{

const Output clique_cover_bound(
        const Instance& instance,
        const Solution* initial_solution = nullptr,
        const Parameters& parameters = {});

}
