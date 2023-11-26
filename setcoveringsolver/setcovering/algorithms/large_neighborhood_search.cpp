#include "setcoveringsolver/setcovering/algorithms/large_neighborhood_search.hpp"

#include "setcoveringsolver/setcovering/algorithms/greedy.hpp"

#include "optimizationtools/containers/indexed_set.hpp"
#include "optimizationtools/containers/indexed_binary_heap.hpp"

using namespace setcoveringsolver::setcovering;

void LargeNeighborhoodSearchOutput::print_statistics(
        optimizationtools::Info& info) const
{
    if (info.output().verbosity_level() >= 1) {
        info.output()
            << "Number of iterations:          " << number_of_iterations << std::endl
            ;
    }
    info.output().add_to_json("Algorithm", "NumberOfIterations", number_of_iterations);
}

struct LargeNeighborhoodSearchSet
{
    Counter timestamp = -1;
    Counter last_addition = -1;
    Counter last_removal = -1;
    Counter iterations = 0;
    Cost score = 0;
};

const LargeNeighborhoodSearchOutput setcoveringsolver::setcovering::large_neighborhood_search(
        Instance& original_instance,
        LargeNeighborhoodSearchOptionalParameters parameters)
{
    init_display(original_instance, parameters.info);
    parameters.info.output()
            << "Algorithm" << std::endl
            << "---------" << std::endl
            << "Large neighborhood search" << std::endl
            << std::endl
            << "Parameters" << std::endl
            << "----------" << std::endl
            << "Maximum number of iterations:                      " << parameters.maximum_number_of_iterations << std::endl
            << "Maximum number of iterations without improvement:  " << parameters.maximum_number_of_iterations_without_improvement << std::endl
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

    LargeNeighborhoodSearchOutput output(original_instance, parameters.info);

    GreedyOptionalParameters greedy_parameters;
    greedy_parameters.reduction_parameters.reduce = false;
    Solution solution = greedy(instance, greedy_parameters).solution;
    std::stringstream ss;
    ss << "initial solution";
    output.update_solution(solution, ss, parameters.info);

    // Initialize local search structures.
    std::vector<LargeNeighborhoodSearchSet> sets(instance.number_of_sets());
    std::vector<Penalty> solution_penalties(instance.number_of_elements(), 1);
    for (SetId set_id: solution.sets())
        for (ElementId element_id: instance.set(set_id).elements)
            if (solution.covers(element_id) == 1)
                sets[set_id].score += solution_penalties[element_id];
    optimizationtools::IndexedBinaryHeap<std::pair<double, Counter>> scores_in(instance.number_of_sets());
    optimizationtools::IndexedBinaryHeap<std::pair<double, Counter>> scores_out(instance.number_of_sets());
    for (SetId set_id: solution.sets())
        scores_in.update_key(set_id, {(double)sets[set_id].score / instance.set(set_id).cost, 0});

    optimizationtools::IndexedSet sets_in_to_update(instance.number_of_sets());
    optimizationtools::IndexedSet sets_out_to_update(instance.number_of_sets());
    Counter iterations_without_improvment = 0;
    for (output.number_of_iterations = 0;
            !parameters.info.needs_to_end();
            ++output.number_of_iterations,
            ++iterations_without_improvment) {
        // Check stop criteria.
        if (parameters.maximum_number_of_iterations != -1
                && output.number_of_iterations >= parameters.maximum_number_of_iterations)
            break;
        if (parameters.maximum_number_of_iterations_without_improvement != -1
                && iterations_without_improvment >= parameters.maximum_number_of_iterations_without_improvement)
            break;
        if (output.solution.cost() == parameters.goal)
            break;
        //std::cout
            //<< "cost " << solution.cost()
            //<< " s " << solution.number_of_sets()
            //<< " f " << solution.feasible()
            //<< std::endl;

        // Remove sets.
        SetPos number_of_removed_sets = sqrt(solution.number_of_sets());
        sets_out_to_update.clear();
        for (SetPos s_tmp = 0; s_tmp < number_of_removed_sets && !scores_in.empty(); ++s_tmp) {
            auto p = scores_in.top();
            scores_in.pop();
            SetId set_id = p.first;
            //std::cout << "remove " << s << " score " << p.second << " cost " << instance.set(set_id).cost << " e " << solution.number_of_elements() << std::endl;
            solution.remove(set_id);
            sets[set_id].last_removal = output.number_of_iterations;
            sets_out_to_update.add(set_id);
            // Update scores.
            sets_in_to_update.clear();
            for (ElementId element_id: instance.set(set_id).elements) {
                if (solution.covers(element_id) == 0) {
                    for (SetId set_id_2: instance.element(element_id).sets) {
                        if (set_id_2 == set_id)
                            continue;
                        sets[set_id_2].score += solution_penalties[element_id];
                        sets_out_to_update.add(set_id_2);
                    }
                } else if (solution.covers(element_id) == 1) {
                    for (SetId set_id_2: instance.element(element_id).sets) {
                        if (!solution.contains(set_id_2))
                            continue;
                        sets[set_id_2].score += solution_penalties[element_id];
                        sets_in_to_update.add(set_id_2);
                    }
                }
            }
            for (SetId set_id_2: sets_in_to_update)
                scores_in.update_key(set_id_2, {(double)sets[set_id_2].score / instance.set(set_id_2).cost, sets[set_id_2].last_addition});
        }
        for (SetId set_id_2: sets_out_to_update)
            scores_out.update_key(set_id_2, {- (double)sets[set_id_2].score / instance.set(set_id_2).cost, sets[set_id_2].last_removal});

        // Update penalties: we increment the penalty of each uncovered element.
        sets_out_to_update.clear();
        for (auto it = solution.elements().out_begin(); it != solution.elements().out_end(); ++it) {
            solution_penalties[it->first]++;
            for (SetId set_id: instance.element(it->first).sets) {
                sets[set_id].score++;
                sets_out_to_update.add(set_id);
            }
        }
        for (SetId set_id: sets_out_to_update)
            scores_out.update_key(set_id, {- (double)sets[set_id].score / instance.set(set_id).cost, sets[set_id].last_removal});

        // Add sets.
        sets_in_to_update.clear();
        while (!solution.feasible() && !scores_out.empty()) {
            auto p = scores_out.top();
            scores_out.pop();
            SetId set_id = p.first;
            solution.add(set_id);
            //std::cout << "add " << s << " score " << p.second << " cost " << instance.set(set_id).cost << " e " << solution.number_of_elements() << std::endl;
            assert(p.second.first < 0);
            sets[set_id].last_addition = output.number_of_iterations;
            sets_in_to_update.add(set_id);
            // Update scores.
            sets_out_to_update.clear();
            for (ElementId element_id: instance.set(set_id).elements) {
                if (solution.covers(element_id) == 1) {
                    for (SetId set_id_2: instance.element(element_id).sets) {
                        if (solution.contains(set_id_2))
                            continue;
                        sets[set_id_2].score -= solution_penalties[element_id];
                        sets_out_to_update.add(set_id_2);
                    }
                } else if (solution.covers(element_id) == 2) {
                    for (SetId set_id_2: instance.element(element_id).sets) {
                        if (set_id_2 == set_id || !solution.contains(set_id_2))
                            continue;
                        sets[set_id_2].score -= solution_penalties[element_id];
                        sets_in_to_update.add(set_id_2);
                    }
                }
            }

            // Remove redundant sets.
            for (ElementId element_id: instance.set(set_id).elements) {
                for (SetId set_id_2: instance.element(element_id).sets) {
                    if (solution.contains(set_id_2) && sets[set_id_2].score == 0) {
                        solution.remove(set_id_2);
                        sets[set_id_2].last_removal = output.number_of_iterations;
                        //std::cout << "> remove " << set_id_2 << " score " << sets[set_id_2].score << " cost " << instance.set(set_id_2).cost << " e " << solution.number_of_elements() << " / " << instance.number_of_elements() << std::endl;
                        for (ElementId element_id_2: instance.set(set_id_2).elements) {
                            if (solution.covers(element_id_2) == 1) {
                                for (SetId s3: instance.element(element_id_2).sets) {
                                    if (!solution.contains(s3))
                                        continue;
                                    sets[s3].score += solution_penalties[element_id_2];
                                    sets_in_to_update.add(s3);
                                }
                            }
                        }
                    }
                }
            }

            for (SetId set_id_2: sets_out_to_update)
                scores_out.update_key(set_id_2, {- (double)sets[set_id_2].score / instance.set(set_id_2).cost, sets[set_id_2].last_removal});
        }
        for (SetId set_id_2: sets_in_to_update) {
            if (solution.contains(set_id_2)) {
                scores_in.update_key(set_id_2, {(double)sets[set_id_2].score / instance.set(set_id_2).cost, sets[set_id_2].last_addition});
            } else {
                scores_in.update_key(set_id_2, {-1, -1});
                scores_in.pop();
            }
        }

        // Update best solution.
        //std::cout << "cost " << solution.cost() << std::endl;
        if (output.solution.cost() > solution.cost()){
            std::stringstream ss;
            ss << "iteration " << output.number_of_iterations;
            output.update_solution(solution, ss, parameters.info);
            iterations_without_improvment = 0;
        }
    }

    output.algorithm_end(parameters.info);
    return output;
}

