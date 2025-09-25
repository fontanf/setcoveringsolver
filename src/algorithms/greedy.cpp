#include "setcoveringsolver/algorithms/greedy.hpp"

#include "setcoveringsolver/algorithm_formatter.hpp"

#include "optimizationtools/containers/indexed_4ary_heap.hpp"

using namespace setcoveringsolver;

const Output setcoveringsolver::greedy(
        const Instance& instance,
        const Parameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("Greedy");

    // Reduction.
    if (parameters.reduction_parameters.reduce)
        return solve_reduced_instance(greedy, instance, parameters, algorithm_formatter, output);

    algorithm_formatter.print_header();

    Solution solution(instance);

    auto f = [&instance](SetId set_id) { return std::pair<double, SetId>{-1.0 * (double)instance.set(set_id).elements.size() / instance.set(set_id).cost, set_id}; };
    optimizationtools::Indexed4aryHeap<std::pair<double, SetId>> heap(instance.number_of_sets(), f);

    while (!solution.feasible()) {

        // Check time.
        if (parameters.timer.needs_to_end()) {
            algorithm_formatter.end();
            return output;
        }

        auto p = heap.top();
        SetId set_id = p.first;
        double score_old = p.second.first;
        // Number of uncovered elements covered by p.first.
        ElementId number_of_covered_elements = 0;
        for (ElementId element_id: instance.set(p.first).elements)
            if (solution.covers(element_id) == 0)
                number_of_covered_elements++;
        double score_cur = -(double)number_of_covered_elements / instance.set(p.first).cost;
        //std::cout << "n " << solution.number_of_sets()
        //    << " cost " << solution.cost()
        //    << " e " << solution.number_of_elements() << " / " << instance.number_of_elements()
        //    << " set_id " << set_id
        //    << " score_old " << score_old
        //    << " score_new " << score_cur
        //    << std::endl;
        if (score_cur <= score_old + FFOT_TOL) {
            solution.add(set_id);
            heap.pop();
        } else {
            heap.update_key(set_id, {score_cur, set_id});
        }
    }

    // Remove redundant sets.
    for (auto it_s = solution.sets().begin(); it_s != solution.sets().end();) {

        // Check time.
        if (parameters.timer.needs_to_end()) {
            algorithm_formatter.end();
            return output;
        }

        SetId set_id = *it_s;
        bool remove = true;
        for (ElementId element_id: instance.set(set_id).elements) {
            if (solution.covers(element_id) == 1) {
                remove = false;
                break;
            }
        }
        if (remove) {
            solution.remove(set_id);
        } else {
            it_s++;
        }
    }

    algorithm_formatter.update_solution(solution, "");
    algorithm_formatter.end();
    return output;
}

const Output setcoveringsolver::greedy_lin(
        const Instance& instance,
        const Parameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("Greedy Lin");

    // Reduction.
    if (parameters.reduction_parameters.reduce)
        return solve_reduced_instance(greedy_lin, instance, parameters, algorithm_formatter, output);

    algorithm_formatter.print_header();

    Solution solution(instance);

    auto f = [&instance, &solution](SetId set_id)
    {
        double val = 0;
        for (ElementId element_id: instance.set(set_id).elements)
            if (solution.covers(element_id) == 0)
                val += 1.0 / instance.element(element_id).sets.size();
        return - val / instance.set(set_id).cost;
    };
    optimizationtools::Indexed4aryHeap<double> heap(instance.number_of_sets(), f);

    while (!solution.feasible()) {

        // Check time.
        if (parameters.timer.needs_to_end()) {
            algorithm_formatter.end();
            return output;
        }

        auto p = heap.top();
        SetId set_id = p.first;
        double score = f(set_id);
        if (score <= p.second + FFOT_TOL) {
            solution.add(set_id);
            heap.pop();
        } else {
            heap.update_key(set_id, score);
        }
    }

    // Remove redundant sets.
    for (auto it_s = solution.sets().begin(); it_s != solution.sets().end();) {

        // Check time.
        if (parameters.timer.needs_to_end()) {
            algorithm_formatter.end();
            return output;
        }

        SetId set_id = *it_s;
        bool remove = true;
        for (ElementId element_id: instance.set(set_id).elements) {
            if (solution.covers(element_id) == 1) {
                remove = false;
                break;
            }
        }
        if (remove) {
            solution.remove(set_id);
        } else {
            it_s++;
        }
    }

    algorithm_formatter.update_solution(solution, "");
    algorithm_formatter.end();
    return output;
}

namespace
{

inline double greedy_reverse_score(
        const Solution& solution,
        SetId set_id)
{
    const Instance& instance = solution.instance();
    const Set& set = instance.set(set_id);
    double score = 0;
    for (ElementId element_id: set.elements) {
        if (solution.covers(element_id) == 0) {
            throw std::logic_error(
                    "setcoveringsolver::greedy_reverse_score: "
                    "infeasible solution; "
                    "element_id: " + std::to_string(element_id) + ".");
        }
        if (solution.covers(element_id) == 1)
            return std::numeric_limits<double>::infinity();
        score += 1.0 / solution.covers(element_id);
    }
    score /= set.cost;
    return score;
}

}

const Output setcoveringsolver::greedy_reverse(
        const Instance& instance,
        const Parameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("Reverse greedy");

    // Reduction.
    if (parameters.reduction_parameters.reduce)
        return solve_reduced_instance(greedy_reverse, instance, parameters, algorithm_formatter, output);

    algorithm_formatter.print_header();

    Solution solution(instance);
    solution.fill();

    auto f = [&solution](SetId set_id) { return std::pair<double, SetId>{greedy_reverse_score(solution, set_id), set_id}; };
    optimizationtools::Indexed4aryHeap<std::pair<double, SetId>> heap(instance.number_of_sets(), f);

    for (;;) {

        // Check time.
        if (parameters.timer.needs_to_end()) {
            algorithm_formatter.end();
            return output;
        }

        auto p = heap.top();
        // Number of uncovered elements covered by p.first.
        SetId set_id = p.first;
        double score_old = p.second.first;
        double score_cur = greedy_reverse_score(solution, set_id);
        //std::cout << "s " << solution.number_of_sets()
        //    << " c " << solution.cost()
        //    << " e " << solution.number_of_elements() << " / " << instance.number_of_elements()
        //    << " set_id " << set_id
        //    << " score_old " << score_old
        //    << " score_cur " << score_cur << std::endl;
        if (score_cur <= score_old) {
            if (score_cur == std::numeric_limits<double>::infinity())
                break;
            solution.remove(set_id);
            if (!solution.feasible()) {
                throw std::logic_error(
                        "setcoveringsolver::greedy_reverse: "
                        "infeasible solution.");
            }
            heap.pop();
        } else {
            heap.update_key(set_id, {score_cur, set_id});
        }
    }

    algorithm_formatter.update_solution(solution, "");
    algorithm_formatter.end();
    return output;
}

const Output setcoveringsolver::greedy_dual(
        const Instance& instance,
        const Parameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("Dual greedy");

    // Reduction.
    if (parameters.reduction_parameters.reduce)
        return solve_reduced_instance(greedy_dual, instance, parameters, algorithm_formatter, output);

    algorithm_formatter.print_header();

    Solution solution(instance);

    for (ElementId element_id = 0;
            element_id < instance.number_of_elements();
            ++element_id) {
        if (solution.covers(element_id) != 0)
            continue;

        // Check time.
        if (parameters.timer.needs_to_end()) {
            algorithm_formatter.end();
            return output;
        }

        SetId set_id_best = -1;
        double val_best;
        for (SetId set_id: instance.element(element_id).sets) {
            if (solution.contains(set_id))
                continue;
            ElementId number_of_covered_elements = 0;
            for (ElementId element_id_2: instance.set(set_id).elements)
                if (solution.covers(element_id_2) == 0)
                    number_of_covered_elements++;
            double val = (double)number_of_covered_elements / instance.set(set_id).cost;
            if (set_id_best == -1 || val_best < val) {
                set_id_best = set_id;
                val_best = val;
            }
        }
        solution.add(set_id_best);
    }

    // Remove redundant sets.
    for (auto it_s = solution.sets().begin(); it_s != solution.sets().end();) {

        // Check time.
        if (parameters.timer.needs_to_end()) {
            algorithm_formatter.end();
            return output;
        }

        SetId set_id = *it_s;
        bool remove = true;
        for (ElementId element_id: instance.set(set_id).elements) {
            if (solution.covers(element_id) == 1) {
                remove = false;
                break;
            }
        }
        if (remove) {
            solution.remove(set_id);
        } else {
            it_s++;
        }
    }

    algorithm_formatter.update_solution(solution, "");
    algorithm_formatter.end();
    return output;
}

const Output setcoveringsolver::greedy_gwmin(
        const Instance& instance,
        const Parameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("Greedy");

    // Reduction.
    if (parameters.reduction_parameters.reduce)
        return solve_reduced_instance(greedy, instance, parameters, algorithm_formatter, output);

    algorithm_formatter.print_header();

    Solution solution(instance);
    solution.fill();

    std::vector<double> sets_values(instance.number_of_sets(), 0);
    for (SetId set_id = 0;
            set_id < instance.number_of_sets();
            ++set_id) {
        sets_values[set_id] = (double)instance.set(set_id).cost
            / (instance.set(set_id).elements.size() + 1);
    }

    std::vector<SetId> sorted_sets(instance.number_of_sets(), 0);
    std::iota(sorted_sets.begin(), sorted_sets.end(), 0);
    std::sort(sorted_sets.begin(), sorted_sets.end(),
            [&sets_values](SetId set_id_1, SetId set_id_2) -> bool
        {
            return sets_values[set_id_1] > sets_values[set_id_2];
        });

    for (SetId set_id: sorted_sets) {
        bool ok = true;
        for (ElementId element_id: instance.set(set_id).elements) {
            if (solution.covers(element_id) == 1) {
                ok = false;
                break;
            }
        }
        if (ok)
            solution.remove(set_id);
    }
    algorithm_formatter.update_solution(solution, "");

    algorithm_formatter.end();
    return output;
}

const Output setcoveringsolver::greedy_or_greedy_reverse(
        const Instance& instance,
        const Parameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("Greedy or reverse greedy");

    // Reduction.
    if (parameters.reduction_parameters.reduce)
        return solve_reduced_instance(greedy_or_greedy_reverse, instance, parameters, algorithm_formatter, output);

    algorithm_formatter.print_header();

    // Estimate the number of sets in the solution.
    Parameters greedy_dual_parameters;
    greedy_dual_parameters.verbosity_level = 0;
    greedy_dual_parameters.timer = parameters.timer;
    auto greedy_dual_output = greedy_dual(instance, greedy_dual_parameters);
    algorithm_formatter.update_solution(greedy_dual_output.solution, "dual greedy");

    Parameters greedy_gwmin_parameters;
    greedy_gwmin_parameters.verbosity_level = 0;
    greedy_gwmin_parameters.timer = parameters.timer;
    auto greedy_gwmin_output = greedy_gwmin(instance, greedy_gwmin_parameters);
    algorithm_formatter.update_solution(greedy_gwmin_output.solution, "greedy gwmin");

    // Check time.
    if (parameters.timer.needs_to_end()) {
        algorithm_formatter.end();
        return output;
    }

    if (greedy_dual_output.solution.number_of_sets() < instance.number_of_sets() / 2) {
        Parameters greedy_parameters;
        greedy_parameters.verbosity_level = 0;
        greedy_parameters.timer = parameters.timer;
        auto greedy_output = greedy(instance, greedy_parameters);
        algorithm_formatter.update_solution(greedy_output.solution, "greedy");

        // Check time.
        if (parameters.timer.needs_to_end()) {
            algorithm_formatter.end();
            return output;
        }

        Parameters greedy_lin_parameters;
        greedy_lin_parameters.verbosity_level = 0;
        greedy_lin_parameters.timer = parameters.timer;
        auto greedy_lin_output = greedy_lin(instance, greedy_parameters);
        algorithm_formatter.update_solution(greedy_lin_output.solution, "greedy lin");
    } else {
        Parameters greedy_reverse_parameters;
        greedy_reverse_parameters.verbosity_level = 0;
        greedy_reverse_parameters.timer = parameters.timer;
        auto greedy_reverse_output = greedy_reverse(instance, greedy_reverse_parameters);
        algorithm_formatter.update_solution(greedy_reverse_output.solution, "reverse greedy");
    }

    algorithm_formatter.end();
    return output;
}
