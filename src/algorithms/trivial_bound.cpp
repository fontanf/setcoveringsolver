#include "setcoveringsolver/algorithms/trivial_bound.hpp"

#include "setcoveringsolver/algorithm_formatter.hpp"

using namespace setcoveringsolver;

const Output setcoveringsolver::trivial_bound(
        const Instance& instance,
        const Parameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("Trivial bound");

    // Reduction.
    if (parameters.reduction_parameters.reduce)
        return solve_reduced_instance(trivial_bound, instance, parameters, algorithm_formatter, output);

    algorithm_formatter.print_header();

    // Sort sets by number_of_elements_covered / cost.
    Cost bound = 0;
    std::vector<SetId> sorted_sets(instance.number_of_sets(), 0);
    std::iota(sorted_sets.begin(), sorted_sets.end(), 0);
    std::sort(
            sorted_sets.begin(),
            sorted_sets.end(),
            [&instance](
                SetId set_id_1,
                SetId set_id_2)
            {
                const Set& set_1 = instance.set(set_id_1);
                const Set& set_2 = instance.set(set_id_2);
                return set_1.cost * set_2.elements.size() < set_2.cost * set_1.elements.size();
            });
    ElementPos number_of_uncovered_elements = instance.number_of_elements();
    for (SetPos set_pos = 0;
            set_pos < instance.number_of_sets();
            ++set_pos) {
        SetId set_id = sorted_sets[set_pos];
        const Set& set = instance.set(set_id);
        if (set.elements.size() <= number_of_uncovered_elements) {
            bound += set.cost;
            number_of_uncovered_elements -= set.elements.size();
            if (number_of_uncovered_elements == 0)
                break;
        } else {
            bound += (set.cost * number_of_uncovered_elements - 1) / set.elements.size() + 1;
            break;
        }
    }

    algorithm_formatter.update_bound(bound, "");
    algorithm_formatter.end();
    return output;
}
