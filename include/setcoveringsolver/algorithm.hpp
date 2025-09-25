#pragma once

#include "setcoveringsolver/reduction.hpp"

#include "optimizationtools/utils/output.hpp"
#include "optimizationtools/utils/utils.hpp"

#include <iomanip>

namespace setcoveringsolver
{

inline optimizationtools::ObjectiveDirection objective_direction()
{
    return optimizationtools::ObjectiveDirection::Minimize;
}

/**
 * Output structure for a set covering problem.
 */
struct Output: optimizationtools::Output
{
    /** Constructor. */
    Output(const Instance& instance): solution(instance)
    {
        solution.fill();
    }


    /** Solution. */
    Solution solution;

    /** Bound. */
    Cost bound = 0;

    /** Elapsed time. */
    double time = 0.0;


    std::string solution_value() const
    {
        return optimizationtools::solution_value(
            objective_direction(),
            solution.feasible(),
            solution.objective_value());
    }

    double absolute_optimality_gap() const
    {
        return optimizationtools::absolute_optimality_gap(
                objective_direction(),
                solution.feasible(),
                solution.objective_value(),
                bound);
    }

    double relative_optimality_gap() const
    {
       return optimizationtools::relative_optimality_gap(
            objective_direction(),
            solution.feasible(),
            solution.objective_value(),
            bound);
    }

    virtual nlohmann::json to_json() const
    {
        return nlohmann::json {
            {"Solution", solution.to_json()},
            {"Value", solution_value()},
            {"Bound", bound},
            {"AbsoluteOptimalityGap", absolute_optimality_gap()},
            {"RelativeOptimalityGap", relative_optimality_gap()},
            {"Time", time}
        };
    }

    virtual int format_width() const { return 30; }

    virtual void format(std::ostream& os) const
    {
        int width = format_width();
        os
            << std::setw(width) << std::left << "Value: " << solution_value() << std::endl
            << std::setw(width) << std::left << "Bound: " << bound << std::endl
            << std::setw(width) << std::left << "Absolute optimality gap: " << absolute_optimality_gap() << std::endl
            << std::setw(width) << std::left << "Relative optimality gap (%): " << relative_optimality_gap() * 100 << std::endl
            << std::setw(width) << std::left << "Time (s): " << time << std::endl
            ;
    }
};

using NewSolutionCallback = std::function<void(const Output&, const std::string&)>;

struct Parameters: optimizationtools::Parameters
{
    /** Callback function called when a new best solution is found. */
    NewSolutionCallback new_solution_callback = [](const Output&, const std::string&) { };

    /** Enable new solution callback. */
    bool enable_new_solution_callback = true;

    /** Reduction parameters. */
    ReductionParameters reduction_parameters;

    /** Goal. */
    Cost goal;


    virtual nlohmann::json to_json() const override
    {
        nlohmann::json json = optimizationtools::Parameters::to_json();
        json.merge_patch(
            {"Reduction",
                {"Enable", reduction_parameters.reduce},
                {"TimeLimit", reduction_parameters.timer.time_limit()},
                {"MaximumNumberOfRounds", reduction_parameters.maximum_number_of_rounds},
                {"SetFolding", reduction_parameters.set_folding},
                {"Twin", reduction_parameters.twin},
                {"VertexCoverDomination", reduction_parameters.vertex_cover_domination},
                {"UnconfinedSets", reduction_parameters.unconfined_sets},
                {"DominatedSetsRemoval", reduction_parameters.dominated_sets_removal},
                {"DominatedElementsRemoval", reduction_parameters.dominated_elements_removal}});
        return json;
    }

    virtual int format_width() const override { return 30; }

    virtual void format(std::ostream& os) const override
    {
        optimizationtools::Parameters::format(os);
        int width = format_width();
        os
            << "Reduction" << std::endl
            << std::setw(width) << std::left << "    Enable: " << reduction_parameters.reduce << std::endl
            << std::setw(width) << std::left << "    Time limit: " << reduction_parameters.timer.time_limit() << std::endl
            << std::setw(width) << std::left << "    Max. # of rounds: " << reduction_parameters.maximum_number_of_rounds << std::endl
            << std::setw(width) << std::left << "    Set folding: " << reduction_parameters.set_folding << std::endl
            << std::setw(width) << std::left << "    Twin: " << reduction_parameters.twin << std::endl
            << std::setw(width) << std::left << "    Unconfined sets: " << reduction_parameters.unconfined_sets << std::endl
            << std::setw(width) << std::left << "    Vertex cover domination: " << reduction_parameters.vertex_cover_domination << std::endl
            << std::setw(width) << std::left << "    Dominated sets removal: " << reduction_parameters.dominated_sets_removal << std::endl
            << std::setw(width) << std::left << "    Dominated elts removal: " << reduction_parameters.dominated_elements_removal << std::endl
            ;
    }
};

}
