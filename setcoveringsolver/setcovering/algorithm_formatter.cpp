#include "setcoveringsolver/setcovering/algorithm_formatter.hpp"

#include "optimizationtools/utils/utils.hpp"

#include <iomanip>

using namespace setcoveringsolver::setcovering;

void AlgorithmFormatter::start(
        Output& output,
        const std::string& algorithm_name)
{
    output.json["Parameters"] = parameters_.to_json();

    if (parameters_.verbosity_level == 0)
        return;
    *os_
        << "=====================================" << std::endl
        << "          SetCoveringSolver          " << std::endl
        << "=====================================" << std::endl
        << std::endl
        << "Problem" << std::endl
        << "-------" << std::endl
        << "Set covering problem" << std::endl
        << std::endl
        << "Instance" << std::endl
        << "--------" << std::endl;
    output.solution.instance().format(*os_, parameters_.verbosity_level);
    *os_
        << std::endl
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << algorithm_name << std::endl
        << std::endl
        << "Parameters" << std::endl
        << "----------" << std::endl;
    parameters_.format(*os_);
}

void AlgorithmFormatter::print_header(
        Output& output)
{
    *os_
        << std::right
        << std::endl
        << std::setw(12) << "T (s)"
        << std::setw(12) << "UB"
        << std::setw(12) << "LB"
        << std::setw(12) << "GAP"
        << std::setw(12) << "GAP (%)"
        << std::setw(24) << "Comment"
        << std::endl
        << std::setw(12) << "-----"
        << std::setw(12) << "--"
        << std::setw(12) << "--"
        << std::setw(12) << "---"
        << std::setw(12) << "-------"
        << std::setw(24) << "-------"
        << std::endl;
    print(output, std::stringstream(""));
}

void AlgorithmFormatter::print_reduced_instance(
        const Instance& reduced_instance)
{
    if (parameters_.verbosity_level == 0)
        return;
    *os_
        << std::endl
        << "Reduced instance" << std::endl
        << "----------------" << std::endl;
    reduced_instance.format(*os_, parameters_.verbosity_level);
}

void AlgorithmFormatter::print(
        const Output& output,
        const std::stringstream& s)
{
    if (parameters_.verbosity_level == 0)
        return;
    std::streamsize precision = std::cout.precision();
    *os_
        << std::setw(12) << std::fixed << std::setprecision(3) << output.time << std::defaultfloat << std::setprecision(precision)
        << std::setw(12) << output.solution_value()
        << std::setw(12) << output.bound
        << std::setw(12) << output.absolute_optimality_gap()
        << std::setw(12) << std::fixed << std::setprecision(2) << output.relative_optimality_gap() * 100 << std::defaultfloat << std::setprecision(precision)
        << std::setw(24) << s.str() << std::endl;
}

void AlgorithmFormatter::update_solution(
        Output& output,
        const Solution& solution_new,
        const std::stringstream& s)
{
    if (optimizationtools::is_solution_strictly_better(
                objective_direction(),
                output.solution.feasible(),
                output.solution.cost(),
                solution_new.feasible(),
                solution_new.cost())) {
        output.time = parameters_.timer.elapsed_time();
        output.solution.update(solution_new);
        print(output, s);
        output.json["IntermediaryOutputs"].push_back(output.to_json());
        parameters_.new_solution_callback(output);
    }
}

void AlgorithmFormatter::update_bound(
        Output& output,
        Cost bound_new,
        const std::stringstream& s)
{
    if (optimizationtools::is_bound_strictly_better(
            objective_direction(),
            output.bound,
            bound_new)) {
        output.time = parameters_.timer.elapsed_time();
        output.bound = bound_new;
        print(output, s);
        output.json["IntermediaryOutputs"].push_back(output.to_json());
        parameters_.new_solution_callback(output);
    }
}

void AlgorithmFormatter::end(
        Output& output)
{
    output.time = parameters_.timer.elapsed_time();
    output.json["Output"] = output.to_json();

    if (parameters_.verbosity_level == 0)
        return;
    *os_
        << std::endl
        << "Final statistics" << std::endl
        << "----------------" << std::endl;
    output.format(*os_);
    *os_
        << std::endl
        << "Solution" << std::endl
        << "--------" << std::endl;
    output.solution.format(*os_, parameters_.verbosity_level);
}
