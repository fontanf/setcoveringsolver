#include "setcoveringsolver/algorithms/milp_ortools.hpp"

#include "setcoveringsolver/algorithm_formatter.hpp"

#include "ortools/math_opt/cpp/math_opt.h"

using namespace setcoveringsolver;

const Output setcoveringsolver::milp_ortools(
        const Instance& instance,
        const MilpOrtoolsParameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("MILP (OR-Tools)");

    // Reduction.
    if (parameters.reduction_parameters.reduce)
        return solve_reduced_instance(milp_ortools, instance, parameters, algorithm_formatter, output);

    algorithm_formatter.print_header();

    operations_research::math_opt::Model model;

    // Variables.
    std::vector<operations_research::math_opt::Variable> x;
    for (SetId set_id = 0; set_id < instance.number_of_sets(); ++set_id)
        x.push_back(model.AddBinaryVariable("x_" + std::to_string(set_id)));

    // Objective: minimize total weight
    operations_research::math_opt::LinearExpression objective;
    for (SetId set_id = 0; set_id < instance.number_of_sets(); ++set_id) {
        const Set& set = instance.set(set_id);
        objective += set.cost * x[set_id];
    }
    model.Minimize(objective);

    // Constraints: each element must be covered.
    for (ElementId element_id = 0;
            element_id < instance.number_of_elements();
            ++element_id) {
        const Element& element = instance.element(element_id);
        operations_research::math_opt::LinearExpression expr;
        for (SetId set_id: element.sets)
            expr += x[set_id];
        model.AddLinearConstraint(expr >= 1.0, "cover_" + std::to_string(element_id));
    }

    // Set time limit.
    operations_research::math_opt::SolveArguments args;
    // Write solver output to file.
    // TODO
    //args.parameters.enable_output = true;

    // Solve
    auto result = Solve(model, operations_research::math_opt::SolverType::kHighs, args);
    if (result->termination.reason
            == operations_research::math_opt::TerminationReason::kOptimal
            || result->termination.reason
            == operations_research::math_opt::TerminationReason::kFeasible) {
        const auto& values = result->variable_values();
        Solution solution(instance);
        for (SetId set_id = 0; set_id < instance.number_of_sets(); ++set_id)
            if (values.at(x[set_id]) > 0.5)
                solution.add(set_id);
        algorithm_formatter.update_solution(solution, "");
    }

    double bound = result->best_objective_bound();
    algorithm_formatter.update_bound(bound, "");

    algorithm_formatter.end();
    return output;
}
