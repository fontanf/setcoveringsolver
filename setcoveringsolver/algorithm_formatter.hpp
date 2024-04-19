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
    new_parameters.new_solution_callback = [
        &algorithm_formatter,
        &reduction,
        &output](
                const Output& new_output,
                const std::string& s)
        {
            Solution solution = output.solution;
            Cost bound = output.bound;
            output = static_cast<const AlgorithmOutput&>(new_output);
            output.solution = solution;
            output.bound = bound;
            algorithm_formatter.update_solution(
                    reduction.unreduce_solution(new_output.solution),
                    s);
            algorithm_formatter.update_bound(
                    reduction.unreduce_bound(new_output.bound),
                    s);
        };
    algorithm(reduction.instance(), new_parameters);

    algorithm_formatter.end();
    return output;
}

}
