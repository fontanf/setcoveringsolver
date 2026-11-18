#pragma once

#include "setcoveringsolver/algorithm.hpp"

namespace setcoveringsolver
{

struct LocalSearchRowWeightingParameters: Parameters
{
    /** Maximum number of iterations. */
    Counter maximum_number_of_iterations = -1;

    /** Maximum number of iterations without improvement. */
    Counter maximum_number_of_iterations_without_improvement = -1;

    /** Best solution update frequency. */
    Counter best_solution_update_frequency = 1;

    /**
     * When the solution of a component becomes feasible with at most this
     * number of sets which are not mandatory (sets which are the only one
     * covering some element), search exhaustively for a solution of the
     * component with fewer sets, to improve it and prove its optimality.
     * Always done with up to 2 such sets.
     */
    Counter exhaustive_search_maximum_number_of_sets = 3;

    /**
     * Maximum number of nodes of an exhaustive search (no limit with up to
     * 2 sets which are not mandatory; -1: no limit).
     */
    Counter exhaustive_search_maximum_number_of_nodes = 10000;

    virtual int format_width() const override { return 37; }

    virtual void format(std::ostream& os) const override
    {
        Parameters::format(os);
        int width = format_width();
        os
            << std::setw(width) << std::left << "Max. # of iterations: " << maximum_number_of_iterations << std::endl
            << std::setw(width) << std::left << "Max. # of iterations without impr.:  " << maximum_number_of_iterations_without_improvement << std::endl
            << std::setw(width) << std::left << "Exhaustive search max. # of sets: " << exhaustive_search_maximum_number_of_sets << std::endl
            << std::setw(width) << std::left << "Exhaustive search max. # of nodes: " << exhaustive_search_maximum_number_of_nodes << std::endl
            ;
    }

    virtual nlohmann::json to_json() const override
    {
        nlohmann::json json = Parameters::to_json();
        json.merge_patch({
                {"MaximumNumberOfIterations", maximum_number_of_iterations},
                {"MaximumNumberOfIterationsWithoutImprovement", maximum_number_of_iterations_without_improvement},
                {"ExhaustiveSearchMaximumNumberOfSets", exhaustive_search_maximum_number_of_sets},
                {"ExhaustiveSearchMaximumNumberOfNodes", exhaustive_search_maximum_number_of_nodes},
                });
        return json;
    }
};

struct LocalSearchRowWeightingOutput: Output
{
    LocalSearchRowWeightingOutput(
            const Instance& instance):
        Output(instance) { }


    /** Number of iterations. */
    Counter number_of_iterations = 0;


    virtual int format_width() const override { return 31; }

    virtual void format(std::ostream& os) const override
    {
        Output::format(os);
        int width = format_width();
        os
            << std::setw(width) << std::left << "Number of iterations: " << number_of_iterations << std::endl
            ;
    }

    virtual nlohmann::json to_json() const override
    {
        nlohmann::json json = Output::to_json();
        json.merge_patch({
                {"NumberOfIterations", number_of_iterations},
                });
        return json;
    }
};

const LocalSearchRowWeightingOutput local_search_row_weighting(
        const Instance& instance,
        const Solution* initial_solution,
        std::mt19937_64& generator,
        const LocalSearchRowWeightingParameters& parameters = {});

}
