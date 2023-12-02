#pragma once

#include "setcoveringsolver/setcovering/solution.hpp"

namespace setcoveringsolver
{
namespace setcovering
{

class AlgorithmFormatter
{

public:

    /** Constructor. */
    AlgorithmFormatter(
            const Parameters& parameters):
        parameters_(parameters),
        os_(parameters.create_os()) { }

    /** Print the header. */
    void start(
            Output& output,
            const std::string& algorithm_name);

    void print_reduced_instance(
            const Instance& reduced_instance);

    /** Print the header. */
    void print_header(
            Output& output);

    /** Print current state. */
    void print(
            const Output& output,
            const std::stringstream& s);

    /** Update the solution. */
    void update_solution(
            Output& output,
            const Solution& solution_new,
            const std::stringstream& s);

    /** Update the bound. */
    void update_bound(
            Output& output,
            Cost bound_new,
            const std::stringstream& s);

    /** Method to call at the end of the algorithm. */
    void end(
            Output& output);

private:

    const Parameters& parameters_;

    std::unique_ptr<optimizationtools::ComposeStream> os_;

};

}
}
