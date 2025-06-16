#include "setcoveringsolver/algorithms/cp_sat_ortools.hpp"

#include "setcoveringsolver/algorithm_formatter.hpp"

#include "ortools/sat/cp_model.h"

using namespace setcoveringsolver;

const Output setcoveringsolver::cp_sat_ortools(
        const Instance& instance,
        const CpSatOrtoolsParameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("CP-SAT (OR-Tools)");

    // Reduction.
    if (parameters.reduction_parameters.reduce)
        return solve_reduced_instance(cp_sat_ortools, instance, parameters, algorithm_formatter, output);

    algorithm_formatter.print_header();

    operations_research::sat::CpModelBuilder cp;

    // Variables.
    std::vector<operations_research::sat::BoolVar> x;
    x.reserve(instance.number_of_sets());
    for (SetId set_id = 0; set_id < instance.number_of_sets(); ++set_id)
        x.push_back(cp.NewBoolVar());

    // Objective: minimize total weight
    operations_research::sat::LinearExpr objective;
    for (SetId set_id = 0; set_id < instance.number_of_sets(); ++set_id) {
        const Set& set = instance.set(set_id);
        objective += set.cost * x[set_id];
    }
    cp.Minimize(objective);

    // Constraints: each element must be covered.
    for (ElementId element_id = 0;
            element_id < instance.number_of_elements();
            ++element_id) {
        const Element& element = instance.element(element_id);
        operations_research::sat::LinearExpr expr;
        for (SetId set_id: element.sets)
            expr += x[set_id];
        cp.AddGreaterOrEqual(expr, 1);
    }

    operations_research::sat::SatParameters cp_parameters;

    // Write solver output to file.
    // TODO
    //cp_parameters.set_log_search_progress(true);

    // Set time limit.
    cp_parameters.set_max_time_in_seconds(parameters.timer.remaining_time());

    // Solve
    operations_research::sat::CpSolverResponse resp = operations_research::sat::SolveWithParameters(cp.Build(), cp_parameters);
    if (resp.status() == operations_research::sat::CpSolverStatus::OPTIMAL
            || resp.status() == operations_research::sat::CpSolverStatus::FEASIBLE) {
        Solution solution(instance);
        for (SetId set_id = 0; set_id < instance.number_of_sets(); ++set_id)
            if (SolutionIntegerValue(resp, x[set_id]) > 0)
                solution.add(set_id);
        algorithm_formatter.update_solution(solution, "");
        if (resp.status() == operations_research::sat::CpSolverStatus::OPTIMAL)
            algorithm_formatter.update_bound(solution.cost(), "");
    }

    algorithm_formatter.end();
    return output;
}
