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


    virtual nlohmann::json to_json() const override;

    virtual int format_width() const override { return 37; }

    virtual void format(std::ostream& os) const override;
};

struct LargeNeighborhoodSearchOutput: Output
{
    LargeNeighborhoodSearchOutput(
            const Instance& instance):
        Output(instance) { }


    /** Number of iterations. */
    Counter number_of_iterations = 0;


    virtual nlohmann::json to_json() const override;

    virtual int format_width() const override { return 30; }

    virtual void format(std::ostream& os) const override;
};

const LargeNeighborhoodSearchOutput large_neighborhood_search(
        const Instance& instance,
        const LargeNeighborhoodSearchParameters& parameters = {});

}
}

