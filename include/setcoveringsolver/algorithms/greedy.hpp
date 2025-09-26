#pragma once

#include "setcoveringsolver/algorithm.hpp"

namespace setcoveringsolver
{

Output greedy(
        const Instance& instance,
        const Parameters& parameters = {});

Output greedy_lin(
        const Instance& instance,
        const Parameters& parameters = {});

Output greedy_reverse(
        const Instance& instance,
        const Parameters& parameters = {});

Output greedy_dual(
        const Instance& instance,
        const Parameters& parameters = {});

Output greedy_dual_sort(
        const Instance& instance,
        const Parameters& parameters = {});

Output greedy_gwmin(
        const Instance& instance,
        const Parameters& parameters = {});

Output greedy_or_greedy_reverse(
        const Instance& instance,
        const Parameters& parameters = {});

}
