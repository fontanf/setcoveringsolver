#pragma once

#include "setcoveringsolver/solution.hpp"

namespace setcoveringsolver
{

Output greedy(
        const Instance& instance,
        optimizationtools::Info info = optimizationtools::Info());

Output greedy_lin(
        const Instance& instance,
        optimizationtools::Info info = optimizationtools::Info());

Output greedy_dual(
        const Instance& instance,
        optimizationtools::Info info = optimizationtools::Info());

}

