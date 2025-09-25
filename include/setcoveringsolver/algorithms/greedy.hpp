#pragma once

#include "setcoveringsolver/algorithm.hpp"

namespace setcoveringsolver
{

const Output greedy(
        const Instance& instance,
        const Parameters& parameters = {});

const Output greedy_lin(
        const Instance& instance,
        const Parameters& parameters = {});

const Output greedy_reverse(
        const Instance& instance,
        const Parameters& parameters = {});

const Output greedy_dual(
        const Instance& instance,
        const Parameters& parameters = {});

const Output greedy_gwmin(
        const Instance& instance,
        const Parameters& parameters = {});

const Output greedy_or_greedy_reverse(
        const Instance& instance,
        const Parameters& parameters = {});

}
