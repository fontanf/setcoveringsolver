#include "setcoveringsolver/setcovering/algorithm_formatter.hpp"

#include "optimizationtools/utils/utils.hpp"

#include <iomanip>

using namespace setcoveringsolver::setcovering;

void AlgorithmFormatter::start(
        const std::string& algorithm_name)
{
    output_.json["Parameters"] = parameters_.to_json();

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
    output_.solution.instance().format(*os_, parameters_.verbosity_level);
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

void AlgorithmFormatter::print_header()
{
    if (parameters_.verbosity_level == 0)
        return;
    *os_
        << std::right
        << std::endl
        << std::setw(12) << "Time (s)"
        << std::setw(12) << "Value"
        << std::setw(12) << "Bound"
        << std::setw(12) << "Gap"
        << std::setw(12) << "Gap (%)"
        << std::setw(24) << "Comment"
        << std::endl
        << std::setw(12) << "--------"
        << std::setw(12) << "-----"
        << std::setw(12) << "-----"
        << std::setw(12) << "---"
        << std::setw(12) << "-------"
        << std::setw(24) << "-------"
        << std::endl;
    print("");
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
        const std::string& s)
{
    if (parameters_.verbosity_level == 0)
        return;
    std::streamsize precision = std::cout.precision();
    *os_
        << std::setw(12) << std::fixed << std::setprecision(3) << output_.time << std::defaultfloat << std::setprecision(precision)
        << std::setw(12) << output_.solution_value()
        << std::setw(12) << output_.bound
        << std::setw(12) << output_.absolute_optimality_gap()
        << std::setw(12) << std::fixed << std::setprecision(2) << output_.relative_optimality_gap() * 100 << std::defaultfloat << std::setprecision(precision)
        << std::setw(24) << s << std::endl;
}

void AlgorithmFormatter::update_solution(
        const Solution& solution_new,
        const std::string& s)
{
    if (optimizationtools::is_solution_strictly_better(
                objective_direction(),
                output_.solution.feasible(),
                output_.solution.objective_value(),
                solution_new.feasible(),
                solution_new.objective_value())) {
        output_.time = parameters_.timer.elapsed_time();
        output_.solution = solution_new;
        print(s);
        output_.json["IntermediaryOutputs"].push_back(output_.to_json());
        parameters_.new_solution_callback(output_, s);
    }
}

void AlgorithmFormatter::update_bound(
        Cost bound_new,
        const std::string& s)
{
    if (optimizationtools::is_bound_strictly_better(
            objective_direction(),
            output_.bound,
            bound_new)) {
        output_.time = parameters_.timer.elapsed_time();
        output_.bound = bound_new;
        print(s);
        output_.json["IntermediaryOutputs"].push_back(output_.to_json());
        parameters_.new_solution_callback(output_, s);
    }
}

void AlgorithmFormatter::end()
{
    output_.time = parameters_.timer.elapsed_time();
    output_.json["Output"] = output_.to_json();

    if (parameters_.verbosity_level == 0)
        return;
    *os_
        << std::endl
        << "Final statistics" << std::endl
        << "----------------" << std::endl;
    output_.format(*os_);
    *os_
        << std::endl
        << "Solution" << std::endl
        << "--------" << std::endl;
    output_.solution.format(*os_, parameters_.verbosity_level);
}
