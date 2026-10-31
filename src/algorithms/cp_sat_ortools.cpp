#include "setcoveringsolver/algorithms/cp_sat_ortools.hpp"

#include "setcoveringsolver/algorithm_formatter.hpp"

#include "ortools/sat/cp_model.h"
#include "ortools/sat/model.h"
#include "ortools/util/time_limit.h"

#include <atomic>

using namespace setcoveringsolver;

const Output setcoveringsolver::cp_sat_ortools(
        const Instance& instance,
        const Solution* initial_solution,
        const CpSatOrtoolsParameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output, initial_solution);
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

    operations_research::sat::Model model;
    model.Add(operations_research::sat::NewSatParameters(cp_parameters));

    // Stop the search when the solution is known to be optimal.
    std::atomic<bool> stop = false;
    model.GetOrCreate<operations_research::TimeLimit>()->RegisterExternalBooleanAsLimit(&stop);

    // Retrieve intermediate solutions.
    auto retrieve_solution = [&instance, &x](
            const operations_research::sat::CpSolverResponse& resp)
    {
        Solution solution(instance);
        for (SetId set_id = 0; set_id < instance.number_of_sets(); ++set_id)
            if (SolutionIntegerValue(resp, x[set_id]) > 0)
                solution.add(set_id);
        return solution;
    };
    model.Add(operations_research::sat::NewFeasibleSolutionObserver(
                [&output, &algorithm_formatter, &retrieve_solution, &stop](
                    const operations_research::sat::CpSolverResponse& resp)
                {
                    algorithm_formatter.update_solution(retrieve_solution(resp), "");
                    if (output.optimal())
                        stop = true;
                }));

    // Solve
    operations_research::sat::CpSolverResponse resp = operations_research::sat::SolveCpModel(cp.Build(), &model);
    if (resp.status() == operations_research::sat::CpSolverStatus::OPTIMAL
            || resp.status() == operations_research::sat::CpSolverStatus::FEASIBLE) {
        Solution solution = retrieve_solution(resp);
        algorithm_formatter.update_solution(solution, "");
        if (resp.status() == operations_research::sat::CpSolverStatus::OPTIMAL)
            algorithm_formatter.update_bound(solution.cost(), "");
    }

    algorithm_formatter.end();
    return output;
}
