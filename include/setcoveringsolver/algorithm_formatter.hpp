#pragma once

#include "setcoveringsolver/algorithm.hpp"

namespace setcoveringsolver
{

class AlgorithmFormatter
{

public:

    /** Constructor. */
    AlgorithmFormatter(
            const Parameters& parameters,
            Output& output):
        parameters_(parameters),
        output_(output),
        os_(parameters.create_os()) { }

    /** Print the header. */
    void start(
            const std::string& algorithm_name);

    void print_reduced_instance(
            const Instance& reduced_instance);

    /** Print the header. */
    void print_header();

    /** Print current state. */
    void print(
            const std::string& s);

    /** Update the solution. */
    void update_solution(
            const Solution& solution,
            const std::string& s);

    /** Update the bound. */
    void update_bound(
            Cost bound,
            const std::string& s);

    /** Method to call at the end of the algorithm. */
    void end();

private:

    /** Parameters. */
    const Parameters& parameters_;

    /** Output. */
    Output& output_;

    /** Output stream. */
    std::unique_ptr<optimizationtools::ComposeStream> os_;

};

template <typename Algorithm, typename AlgorithmParameters, typename AlgorithmOutput>
inline const AlgorithmOutput solve_reduced_instance(
        const Algorithm& algorithm,
        const Instance& instance,
        const AlgorithmParameters& parameters,
        AlgorithmFormatter& algorithm_formatter,
        AlgorithmOutput& output)
{
    Reduction reduction(instance, parameters.reduction_parameters);
    algorithm_formatter.print_reduced_instance(reduction.instance());
    algorithm_formatter.print_header();

    algorithm_formatter.update_solution(
            reduction.unreduce_solution(Solution(reduction.instance())),
            "");
    algorithm_formatter.update_bound(
            reduction.unreduce_bound(0),
            "");

    AlgorithmParameters new_parameters = parameters;
    new_parameters.reduction_parameters.reduce = false;
    new_parameters.verbosity_level = 0;
    Solution solution_tmp(instance);
    Solution unreduced_solution(instance);
    new_parameters.new_solution_callback = [
        &algorithm_formatter,
        &reduction,
        &output,
        &solution_tmp,
        &unreduced_solution](
                const Output& new_output,
                const std::string& s)
        {
            solution_tmp = output.solution;
            Cost bound = output.bound;
            output = static_cast<const AlgorithmOutput&>(new_output);
            output.solution = solution_tmp;
            output.bound = bound;

            reduction.unreduce_solution(unreduced_solution, new_output.solution);
            Cost unreduced_bound = reduction.unreduce_bound(new_output.bound);
            algorithm_formatter.update_solution(unreduced_solution, s);
            algorithm_formatter.update_bound(unreduced_bound, s);
        };
    auto new_output = algorithm(reduction.instance(), new_parameters);

    solution_tmp = output.solution;
    Cost bound = output.bound;
    output = static_cast<const AlgorithmOutput&>(new_output);
    output.solution = solution_tmp;
    output.bound = bound;

    std::string s = "";
    reduction.unreduce_solution(unreduced_solution, new_output.solution);
    Cost unreduced_bound = reduction.unreduce_bound(new_output.bound);
    algorithm_formatter.update_solution(unreduced_solution, s);
    algorithm_formatter.update_bound(unreduced_bound, s);

    algorithm_formatter.end();
    return output;
}

}
