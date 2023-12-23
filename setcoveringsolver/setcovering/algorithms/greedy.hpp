#pragma once

#include "setcoveringsolver/setcovering/algorithm.hpp"

namespace setcoveringsolver
{
namespace setcovering
{

const Output greedy(
        const Instance& instance,
        const Parameters& parameters = {});

const Output greedy_lin(
        const Instance& instance,
        const Parameters& parameters = {});

const Output greedy_dual(
        const Instance& instance,
        const Parameters& parameters = {});

}
}

