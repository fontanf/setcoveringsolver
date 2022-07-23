#pragma once

#include "setcoveringsolver/solution.hpp"

namespace setcoveringsolver
{

struct LocalSearchRowWeighting2OptionalParameters
{
    /**
     * First parameter of the neighborhood:
     * - '0': swap between a set covering the drawn element and one of its
     *   neighbor
     * - '1': swap between a set covering the drawn element and one of the set
     *   neighbor of the drawn element.
     */
    int neighborhood_1 = 0;

    /**
     * Second parameter of the neighborhood:
     * - '0': no additional swap
     * - '1': swap between a set covering the drawn element and one of the set
     *   which is not a set neighbor of the drawn element. In this case, the
     *   two sets which are swapped are "independent".
     * - '2': same as '1' but is only done when no improving move has been
     *   found in the first part of the neighborhood..
     *
     * 'neighborhood_1 = 0' and 'neighborhood_2 = 0' only consider swaps
     * between sets which are neighbors.
     * See "Weighting-based parallel local search for optimal camera placement
     * and unicost set covering" (Lin et al., 2020).
     *
     * 'neighborhood_1 = 1' and 'neighborhood_2 = 1' explores the whole swap
     * neighborhood.
     * See "Weighting-based Variable Neighborhood Search for Optimal Camera
     * Placement" (Su et al., 2021).
     */
    int neighborhood_2 = 0;

    /**
     * Parameter for the weights update strategy:
     * - '0': at the end of an iteration, increment the weights of the elements
     *   uncovered by the set which has been remove.
     * - '1': at the end of a non-improving interation, increment the weights
     *   of all uncovered elements.
     */
    int weights_update_strategy = 0;

    /** Maximum number of iterations. */
    Counter maximum_number_of_iterations = -1;

    /** Maximum number of iterations without improvement. */
    Counter maximum_number_of_iterations_without_improvement = -1;

    /** Info structure. */
    optimizationtools::Info info = optimizationtools::Info();
};

struct LocalSearchRowWeighting2Output: Output
{
    LocalSearchRowWeighting2Output(
            const Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    LocalSearchRowWeighting2Output& algorithm_end(
            optimizationtools::Info& info);

    /** Number of improvements due to the first part of the neighborhood. */
    Counter neighborhood_1_improvements = 0;

    /** Number of improvements due to the second part of the neighborhood. */
    Counter neighborhood_2_improvements = 0;

    /** Time spent in the first part of the neighborhood. */
    double neighborhood_1_time = 0.0;

    /** Time spent in the second part of the neighborhood. */
    double neighborhood_2_time = 0.0;

    /** Number of weights reductions. */
    Counter number_of_weights_reductions = 0;

    /** Number of iterations. */
    Counter number_of_iterations = 0;
};

LocalSearchRowWeighting2Output localsearch_rowweighting_2(
        Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeighting2OptionalParameters parameters = {});

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct LocalSearchRowWeighting1OptionalParameters
{
    /** Maximum number of iterations. */
    Counter maximum_number_of_iterations = -1;
    /** Maximum number of iterations without improvement. */
    Counter maximum_number_of_iterations_without_improvement = -1;
    /** Info structure. */
    optimizationtools::Info info = optimizationtools::Info();
};

struct LocalSearchRowWeighting1Output: Output
{
    LocalSearchRowWeighting1Output(
            const Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    LocalSearchRowWeighting1Output& algorithm_end(
            optimizationtools::Info& info);

    /** Number of iterations. */
    Counter number_of_iterations = 0;
};

LocalSearchRowWeighting1Output localsearch_rowweighting_1(
        Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeighting1OptionalParameters parameters = {});

}

