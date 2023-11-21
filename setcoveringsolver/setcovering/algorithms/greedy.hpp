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

const Output greedy(
        const Instance& instance,
        GreedyOptionalParameters parameters = {});

const Output greedy_lin(
        const Instance& instance,
        GreedyOptionalParameters parameters = {});

const Output greedy_dual(
        const Instance& instance,
        GreedyOptionalParameters parameters = {});

}
}

