#pragma once

#include "setcoveringsolver/setcovering/solution.hpp"

namespace setcoveringsolver
{
namespace setcovering
{

struct GreedyOptionalParameters
{
    /** Reduction parameters. */
    ReductionParameters reduction_parameters;

    /** Info structure. */
    optimizationtools::Info info = optimizationtools::Info();
};

Output greedy(
        const Instance& instance,
        GreedyOptionalParameters parameters = {});

Output greedy_lin(
        const Instance& instance,
        GreedyOptionalParameters parameters = {});

Output greedy_dual(
        const Instance& instance,
        GreedyOptionalParameters parameters = {});

}
}

