#pragma once

#include "setcoveringsolver/setcovering/solution.hpp"

namespace setcoveringsolver
{
namespace setcovering
{

struct LocalSearchRowWeighting1Parameters: Parameters
{
    /** Maximum number of iterations. */
    Counter maximum_number_of_iterations = -1;

    /** Maximum number of iterations without improvement. */
    Counter maximum_number_of_iterations_without_improvement = -1;


    virtual int format_width() const override { return 37; }

    virtual void format(std::ostream& os) const override
    {
        Parameters::format(os);
        int width = format_width();
        os
            << std::setw(width) << std::left << "Max. # of iterations: " << maximum_number_of_iterations << std::endl
            << std::setw(width) << std::left << "Max. # of iterations without impr.:  " << maximum_number_of_iterations_without_improvement << std::endl
            ;
    }

    virtual nlohmann::json to_json() const override
    {
        nlohmann::json json = Parameters::to_json();
        json.merge_patch({
                {"MaximumNumberOfIterations", maximum_number_of_iterations},
                {"MaximumNumberOfIterationsWithoutImprovement", maximum_number_of_iterations_without_improvement}});
        return json;
    }
};

struct LocalSearchRowWeighting1Output: Output
{
    LocalSearchRowWeighting1Output(
            const Instance& instance):
        Output(instance) { }


    /** Number of iterations. */
    Counter number_of_iterations = 0;


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
                {"NumberOfIterations", number_of_iterations}});
        return json;
    }
};

const LocalSearchRowWeighting1Output local_search_row_weighting_1(
        const Instance& instance,
        std::mt19937_64& generator,
        const LocalSearchRowWeighting1Parameters& parameters = {});

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct LocalSearchRowWeighting2Parameters: Parameters
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
     *   found in the first part of the neighborhood.
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
     * - '1': at the end of a non-improving iteration, increment the weights
     *   of all uncovered elements.
     */
    int weights_update_strategy = 0;

    /** Maximum number of iterations. */
    Counter maximum_number_of_iterations = -1;

    /** Maximum number of iterations without improvement. */
    Counter maximum_number_of_iterations_without_improvement = -1;


    virtual int format_width() const override { return 37; }

    virtual void format(std::ostream& os) const override
    {
        Parameters::format(os);
        int width = format_width();
        os
            << std::setw(width) << std::left << "Max. # of iterations: " << maximum_number_of_iterations << std::endl
            << std::setw(width) << std::left << "Max. # of iterations without impr.:  " << maximum_number_of_iterations_without_improvement << std::endl
            ;
    }

    virtual nlohmann::json to_json() const override
    {
        nlohmann::json json = Parameters::to_json();
        json.merge_patch({
                {"MaximumNumberOfIterations", maximum_number_of_iterations},
                {"MaximumNumberOfIterationsWithoutImprovement", maximum_number_of_iterations_without_improvement}});
        return json;
    }
};

struct LocalSearchRowWeighting2Output: Output
{
    LocalSearchRowWeighting2Output(
            const Instance& instance):
        Output(instance) { }


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


    virtual int format_width() const override { return 31; }

    virtual void format(std::ostream& os) const override
    {
        Output::format(os);
        int width = format_width();
        os
            << std::setw(width) << std::left << "Number of iterations: " << number_of_iterations << std::endl
            << std::setw(width) << std::left << "Neighborhood 1 improvements: " << neighborhood_1_improvements << std::endl
            << std::setw(width) << std::left << "Neighborhood 2 improvements: " << neighborhood_2_improvements << std::endl
            << std::setw(width) << std::left << "Neighborhood 1 time: " << neighborhood_1_time << std::endl
            << std::setw(width) << std::left << "Neighborhood 2 time: " << neighborhood_2_time << std::endl
            << std::setw(width) << std::left << "Number of weights reductions: " << neighborhood_2_time << std::endl
            ;
    }

    virtual nlohmann::json to_json() const override
    {
        nlohmann::json json = Output::to_json();
        json.merge_patch({
                {"NumberOfIterations", number_of_iterations}});
        return json;
    }
};

const LocalSearchRowWeighting2Output local_search_row_weighting_2(
        const Instance& instance,
        std::mt19937_64& generator,
        const LocalSearchRowWeighting2Parameters& parameters = {});

}
}

