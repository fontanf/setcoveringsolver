#pragma once

#include "setcoveringsolver/setcovering/solution.hpp"

namespace setcoveringsolver
{
namespace setcovering
{

struct LargeNeighborhoodSearchParameters: Parameters
{
    /** Maximum number of iterations. */
    Counter maximum_number_of_iterations = -1;

    /** Maximum number of iterations without improvement. */
    Counter maximum_number_of_iterations_without_improvement = -1;

    /** Goal. */
    Cost goal;


    virtual nlohmann::json to_json() const override
    {
        nlohmann::json json = Parameters::to_json();
        json.merge_patch({
                {"MaximumNumberOfIterations", maximum_number_of_iterations},
                {"MaximumNumberOfIterationsWithoutImprovement", maximum_number_of_iterations_without_improvement}});
        return json;
    }

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
};

struct LargeNeighborhoodSearchOutput: Output
{
    LargeNeighborhoodSearchOutput(
            const Instance& instance):
        Output(instance) { }


    /** Number of iterations. */
    Counter number_of_iterations = 0;


    virtual nlohmann::json to_json() const override
    {
        nlohmann::json json = Output::to_json();
        json.merge_patch({
                {"NumberOfIterations", number_of_iterations}});
        return json;
    }

    virtual int format_width() const override { return 30; }

    virtual void format(std::ostream& os) const override
    {
        Output::format(os);
        int width = format_width();
        os
            << std::setw(width) << std::left << "Number of iterations: " << number_of_iterations << std::endl
            ;
    }
};

const LargeNeighborhoodSearchOutput large_neighborhood_search(
        const Instance& instance,
        const LargeNeighborhoodSearchParameters& parameters = {});

}
}

