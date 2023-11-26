#include "setcoveringsolver/setcovering/algorithms/greedy.hpp"

#include "optimizationtools/containers/indexed_binary_heap.hpp"

#include <set>

using namespace setcoveringsolver::setcovering;

const Output setcoveringsolver::setcovering::greedy(
        const Instance& original_instance,
        GreedyOptionalParameters parameters)
{
    init_display(original_instance, parameters.info);
    parameters.info.output()
            << "Algorithm" << std::endl
            << "---------" << std::endl
            << "Greedy" << std::endl
            << std::endl;

    // Reduction.
    std::unique_ptr<Instance> reduced_instance = nullptr;
    if (parameters.reduction_parameters.reduce) {
        reduced_instance = std::unique_ptr<Instance>(
                new Instance(
                    original_instance.reduce(
                        parameters.reduction_parameters)));
        parameters.info.output()
            << "Reduced instance" << std::endl
            << "----------------" << std::endl
            << InstanceFormatter{*reduced_instance, parameters.info.output().verbosity_level()}
            << std::endl;
    }
    const Instance& instance = (reduced_instance == nullptr)? original_instance: *reduced_instance;

    Output output(original_instance, parameters.info);

    Solution solution(instance);

    auto f = [&instance](SetId set_id) { return std::pair<double, SetId>{-1.0 * (double)instance.set(set_id).elements.size() / instance.set(set_id).cost, set_id}; };
    optimizationtools::IndexedBinaryHeap<std::pair<double, SetId>> heap(instance.number_of_sets(), f);

    while (!solution.feasible()) {
        auto p = heap.top();
        // Number of uncovered elements covered by p.first.
        ElementId number_of_covered_elements = 0;
        for (ElementId element_id: instance.set(p.first).elements)
            if (solution.covers(element_id) == 0)
                number_of_covered_elements++;
        double val = -(double)number_of_covered_elements / instance.set(p.first).cost;
        //std::cout << "s " << solution.number_of_sets()
            //<< " c " << solution.cost()
            //<< " e " << solution.number_of_elements() << " / " << instance.number_of_elements()
            //<< " s " << p.first << " v_old " << p.second.first << " v_new " << val << std::endl;
        if (val <= p.second.first + FFOT_TOL) {
            solution.add(p.first);
            heap.pop();
        } else {
            heap.update_key(p.first, {val, p.first});
        }
    }

    // Remove redundant sets.
    for (auto it_s = solution.sets().begin(); it_s != solution.sets().end();) {
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

    output.update_solution(solution, std::stringstream(), parameters.info);
    return output.algorithm_end(parameters.info);
}

const Output setcoveringsolver::setcovering::greedy_lin(
        const Instance& original_instance,
        GreedyOptionalParameters parameters)
{
    init_display(original_instance, parameters.info);
    parameters.info.output()
            << "Algorithm" << std::endl
            << "---------" << std::endl
            << "Greedy Lin" << std::endl
            << std::endl;

    // Reduction.
    std::unique_ptr<Instance> reduced_instance = nullptr;
    if (parameters.reduction_parameters.reduce) {
        reduced_instance = std::unique_ptr<Instance>(
                new Instance(
                    original_instance.reduce(
                        parameters.reduction_parameters)));
        parameters.info.output()
            << "Reduced instance" << std::endl
            << "----------------" << std::endl
            << InstanceFormatter{*reduced_instance, parameters.info.output().verbosity_level()}
            << std::endl;
    }
    const Instance& instance = (reduced_instance == nullptr)? original_instance: *reduced_instance;

    Output output(original_instance, parameters.info);

    Solution solution(instance);

    auto f = [&instance, &solution](SetId set_id)
    {
        double val = 0;
        for (ElementId element_id: instance.set(set_id).elements)
            if (solution.covers(element_id) == 0)
                val += 1.0 / instance.element(element_id).sets.size();
        return - val / instance.set(set_id).cost;
    };
    optimizationtools::IndexedBinaryHeap<double> heap(instance.number_of_sets(), f);

    while (!solution.feasible()) {
        auto p = heap.top();
        double val = f(p.first);
        if (val <= p.second + FFOT_TOL) {
            solution.add(p.first);
            heap.pop();
        } else {
            heap.update_key(p.first, val);
        }
    }

    // Remove redundant sets.
    for (auto it_s = solution.sets().begin(); it_s != solution.sets().end();) {
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

    output.update_solution(solution, std::stringstream(), parameters.info);
    return output.algorithm_end(parameters.info);
}

const Output setcoveringsolver::setcovering::greedy_dual(
        const Instance& original_instance,
        GreedyOptionalParameters parameters)
{
    init_display(original_instance, parameters.info);
    parameters.info.output()
            << "Algorithm" << std::endl
            << "---------" << std::endl
            << "Dual Greedy" << std::endl
            << std::endl;

    // Reduction.
    std::unique_ptr<Instance> reduced_instance = nullptr;
    if (parameters.reduction_parameters.reduce) {
        reduced_instance = std::unique_ptr<Instance>(
                new Instance(
                    original_instance.reduce(
                        parameters.reduction_parameters)));
        parameters.info.output()
            << "Reduced instance" << std::endl
            << "----------------" << std::endl
            << InstanceFormatter{*reduced_instance, parameters.info.output().verbosity_level()}
            << std::endl;
    }
    const Instance& instance = (reduced_instance == nullptr)? original_instance: *reduced_instance;

    Output output(original_instance, parameters.info);

    Solution solution(instance);

    for (ElementId element_id = 0;
            element_id < instance.number_of_elements();
            ++element_id) {
        if (solution.covers(element_id) != 0)
            continue;

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

    output.update_solution(solution, std::stringstream(), parameters.info);
    return output.algorithm_end(parameters.info);
}

