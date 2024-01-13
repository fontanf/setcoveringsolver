#include "setcoveringsolver/setcovering/algorithms/local_search_row_weighting.hpp"

#include "setcoveringsolver/setcovering/algorithm_formatter.hpp"
#include "setcoveringsolver/setcovering/algorithms/greedy.hpp"

#include "optimizationtools/containers/indexed_set.hpp"

using namespace setcoveringsolver::setcovering;

struct LocalSearchRowWeightingComponent
{
    /** Last set added to the current solution. */
    SetId set_id_last_added = -1;

    /** Last set removed from the current solution. */
    SetId set_id_last_removed = -1;

    /** Number of iterations. */
    Counter iterations = 0;

    /** Number of iterations without improvment. */
    Counter iterations_without_improvment = 0;

    /** Iteration at which optimizing this component starts (included). */
    Counter itmode_start;

    /** Iteration at which optimizing this component ends (excluded). */
    Counter itmode_end;

    /**
     * Boolean that indicates if the component of the current solution optimal.
     *
     * This is set to true when all the sets of this components which are in
     * the current solution are mandatory sets.
     */
    bool optimal = false;
};

struct LocalSearchRowWeightingSet
{
    Counter timestamp = -1;
    Counter last_addition = -1;
    Counter last_removal = -1;
    Counter iterations = 0;
    Cost score = 0;
};

const LocalSearchRowWeighting2Output setcoveringsolver::setcovering::local_search_row_weighting_2(
        const Instance& instance,
        std::mt19937_64& generator,
        const LocalSearchRowWeighting2Parameters& parameters)
{
    LocalSearchRowWeighting2Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("Row weighting local search 2");

    // Reduction.
    if (parameters.reduction_parameters.reduce) {
        return solve_reduced_instance(
                [&generator](
                    const Instance& instance,
                    const LocalSearchRowWeighting2Parameters& parameters)
                {
                    return local_search_row_weighting_2(
                            instance,
                            generator,
                            parameters);
                },
                instance,
                parameters,
                algorithm_formatter,
                output);
    }

    algorithm_formatter.print_header();

    // Instance pre-processing.
    const auto& set_neighbors = instance.set_neighbors();
    const std::vector<std::vector<SetId>>* element_set_neighbors = nullptr;
    if (parameters.neighborhood_1 == 1 || parameters.neighborhood_2 == 1)
        element_set_neighbors = &instance.element_set_neighbors();

    // Compute initial greedy solution.
    Parameters greedy_parameters;
    greedy_parameters.verbosity_level = 0;
    greedy_parameters.reduction_parameters.reduce = false;
    Solution solution = greedy(instance, greedy_parameters).solution;
    //Solution solution = greedy_lin(instance).solution;
    algorithm_formatter.update_solution(solution, "initial solution");

    Solution solution_best(solution);

    // Initialize local search structures.
    std::vector<LocalSearchRowWeightingSet> sets(instance.number_of_sets());
    for (SetId set_id: solution.sets())
        sets[set_id].last_addition = 0;
    Penalty solution_penalty = 0;
    std::vector<Penalty> solution_penalties(instance.number_of_elements(), 1);
    for (ElementId element_id = 0;
            element_id < instance.number_of_elements();
            ++element_id) {
        if (solution.covers(element_id) == 1)
            for (SetId set_id: instance.element(element_id).sets)
                if (solution.contains(set_id))
                    sets[set_id].score += solution_penalties[element_id];
    }
    bool reduce = false;

    std::vector<LocalSearchRowWeightingComponent> components(instance.number_of_components());
    components[0].itmode_start = 0;
    for (ComponentId component_id = 0;
            component_id < instance.number_of_components();
            ++component_id) {
        components[component_id].itmode_end = components[component_id].itmode_start
            + instance.component(component_id).elements.size();
        if (component_id + 1 < instance.number_of_components())
            components[component_id + 1].itmode_start = components[component_id].itmode_end;
    }

    optimizationtools::IndexedSet neighbor_sets(instance.number_of_sets());

    ComponentId component_id = 0;
    Counter number_of_iterations_without_improvement = 0;
    for (output.number_of_iterations = 0;
            !parameters.timer.needs_to_end();
            ++output.number_of_iterations,
            ++number_of_iterations_without_improvement) {
        // Check stop criteria.
        if (parameters.maximum_number_of_iterations != -1
                && output.number_of_iterations >= parameters.maximum_number_of_iterations)
            break;
        if (parameters.maximum_number_of_iterations_without_improvement != -1
                && number_of_iterations_without_improvement >= parameters.maximum_number_of_iterations_without_improvement)
            break;

        // Compute component
        //std::cout << "it " << output.number_of_iterations
        //    << " % " << output.number_of_iterations % (components.back().itmode_end)
        //    << " c " << c
        //    << " start " << components[component_id].itmode_start
        //    << " end " << components[component_id].itmode_end
        //    << std::endl;
        Counter itmod = output.number_of_iterations % (components.back().itmode_end);
        while (itmod < components[component_id].itmode_start
                || itmod >= components[component_id].itmode_end) {
            component_id = (component_id + 1) % instance.number_of_components();
            //std::cout << "it " << output.number_of_iterations
            //    << " % " << output.number_of_iterations % (components.back().itmode_end)
            //    << " c " << c
            //    << " start " << components[component_id].itmode_start
            //    << " end " << components[component_id].itmode_end
            //    << std::endl;
        }
        LocalSearchRowWeightingComponent& component = components[component_id];

        while (solution.feasible(component_id)) {
            // New best solution
            if (output.solution.cost(component_id) > solution.cost(component_id)) {
                // Update solution_best.
                for (SetId set_id: instance.component(component_id).sets) {
                    if (solution_best.contains(set_id)
                            && !solution.contains(set_id)) {
                        solution_best.remove(set_id);
                    } else if (!solution_best.contains(set_id)
                            && solution.contains(set_id)) {
                        solution_best.add(set_id);
                    }
                }
                std::stringstream ss;
                ss << "it " << output.number_of_iterations << " comp " << component_id;
                algorithm_formatter.update_solution(solution, ss.str());
            }
            // Update statistics
            number_of_iterations_without_improvement = 0;
            if (component.iterations_without_improvment > 0)
                component.iterations_without_improvment = 0;

            // Find the best shift move.
            SetId set_id_best = -1;
            Cost p_best = -1;
            // For each set s of the current solution which belongs to the
            // currently considered component and is not mandatory.
            for (SetId set_id: solution.sets()) {
                if (instance.set(set_id).component != component_id)
                    continue;
                Penalty p = -sets[set_id].score;
                // Update best move.
                if (set_id_best == -1 // First move considered.
                        || p_best < p // Strictly better.
                        // Equivalent, but s has not been considered for a
                        // longer time.
                        || (p_best == p
                            && sets[set_id_best].timestamp > sets[set_id].timestamp)) {
                    set_id_best = set_id;
                    p_best = p;
                }
            }
            if (set_id_best == -1) {
                // Happens when all sets of the component which are in the
                // solution are mandatory.
                component.optimal = true;
                //std::cout << "c " << c << " optimal" << std::endl;
                bool all_component_optimal = true;
                components[0].itmode_start = 0;
                for (ComponentId component_id = 0;
                        component_id < instance.number_of_components();
                        ++component_id) {
                    //std::cout << "comp " << c << " opt " << component.optimal << std::endl;
                    components[component_id].itmode_end = components[component_id].itmode_start;
                    if (components[component_id].optimal) {
                    } else {
                        components[component_id].itmode_end += instance.component(component_id).elements.size();
                        all_component_optimal = false;
                    }
                    if (component_id + 1 < instance.number_of_components())
                        components[component_id + 1].itmode_start = components[component_id].itmode_end;
                }
                // If all components are optimal, stop here.
                if (all_component_optimal) {
                    algorithm_formatter.end();
                    return output;
                }
                break;
            }
            // Apply best move
            solution.remove(set_id_best);
            // Update scores.
            for (ElementId element_id: instance.set(set_id_best).elements) {
                if (solution.covers(element_id) == 0) {
                    solution_penalty += solution_penalties[element_id];
                    for (SetId set_id: instance.element(element_id).sets)
                        if (set_id != set_id_best)
                            sets[set_id].score += solution_penalties[element_id];
                } else if (solution.covers(element_id) == 1) {
                    for (SetId set_id: instance.element(element_id).sets)
                        if (solution.contains(set_id))
                            sets[set_id].score += solution_penalties[element_id];
                }
            }
            // Update sets
            sets[set_id_best].timestamp = output.number_of_iterations;
            sets[set_id_best].iterations += (component.iterations - sets[set_id_best].last_addition);
            sets[set_id_best].last_removal = component.iterations;
            // Update tabu
            component.set_id_last_removed = set_id_best;
            //std::cout << "it " << output.iterations
            //<< " set_id_best " << set_id_best
            //<< " p " << solution.penalty()
            //<< " e " << instance.number_of_elements() - solution.number_of_elements() << "/" << instance.number_of_elements()
            //<< " s " << solution.number_of_sets()
            //<< std::endl;
        }
        if (components[component_id].optimal)
            continue;

        if (solution.cost(component_id) != output.solution.cost(component_id) - 1) {
            throw std::runtime_error("component " + std::to_string(component_id)
                    + " size " + std::to_string(instance.component(component_id).sets.size()));
        }

        bool neighborhood_2_improvement = false;

        // Draw randomly an uncovered element e.
        std::vector<ElementId> e_candidates;
        for (auto it = solution.elements().out_begin(); it != solution.elements().out_end(); ++it)
            if (instance.element(it->first).component == component_id)
                e_candidates.push_back(it->first);
        std::uniform_int_distribution<ElementId> d_e(
                0, e_candidates.size() - 1);
        ElementId element_id = e_candidates[d_e(generator)];
        //std::cout << "it " << output.iterations
            //<< " e " << e
            //<< " s " << instance.element(element_id).sets.size()
            //<< std::endl;

        std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();

        // Find the best swap move.
        SetId set_id_1_best = -1;
        SetId set_id_2_best = -1;
        Cost p_best = 0;

        if (parameters.neighborhood_1 == 1 || parameters.neighborhood_2 == 1) {
            neighbor_sets.clear();
            for (SetId set_id: (*element_set_neighbors)[element_id])
                neighbor_sets.add(set_id);
        }

        // For each set set_id_1 covering element e which is not part of the solution
        // and which is not the last set removed.
        for (SetId set_id_1: instance.element(element_id).sets) {
            if (set_id_1 == component.set_id_last_removed)
                continue;
            Penalty p0 = 0;
            for (ElementId element_id_2: instance.set(set_id_1).elements)
                if (solution.covers(element_id_2) == 0)
                    p0 -= solution_penalties[element_id_2];
            if (set_id_1_best == -1 || p0 <= p_best) {
                solution.add(set_id_1);
                // For each neighbor set_id_2 of set_id_1 which is neither part of the
                // solution, nor the last set added, nor mandatory.
                auto it_begin = (parameters.neighborhood_1 == 0)?
                    set_neighbors[set_id_1].begin():
                    neighbor_sets.begin();
                auto it_end = (parameters.neighborhood_1 == 0)?
                    set_neighbors[set_id_1].end():
                    neighbor_sets.end();
                for (auto it = it_begin; it != it_end; ++it) {
                    SetId set_id_2 = *it;
                    if (set_id_1 == set_id_2
                            || set_id_2 == component.set_id_last_added
                            || !solution.contains(set_id_2))
                        continue;
                    Penalty p = p0;
                    for (ElementId element_id_2: instance.set(set_id_2).elements)
                        if (solution.covers(element_id_2) == 1)
                            p += solution_penalties[element_id_2];
                    // If the new solution is better, we update the best move.
                    if (set_id_1_best == -1
                            || p_best > p // Strictly better.
                            // Equivalent, but set_id_1 and set_id_2 have not been
                            // considered for a longer time.
                            || (p_best == p
                                && sets[set_id_1_best].timestamp + sets[set_id_2_best].timestamp
                                > sets[set_id_1].timestamp + sets[set_id_2].timestamp)) {
                        set_id_1_best = set_id_1;
                        set_id_2_best = set_id_2;
                        p_best = p;
                    }
                }
                solution.remove(set_id_1);
            }
        }

        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
        output.neighborhood_1_time += std::chrono::duration_cast<std::chrono::duration<double>>(t1 - t0).count();

        if (parameters.neighborhood_2 == 1
                || (parameters.neighborhood_2 == 2 && (set_id_1_best == -1 || p_best >= 0))) {
            SetId set_id_3_best = -1;
            SetId set_id_4_best = -1;
            Cost p3_best = -1;
            Cost p4_best = -1;
            for (SetId set_id_3: instance.element(element_id).sets) {
                if (set_id_3 == component.set_id_last_removed)
                    continue;
                // If the new solution is better, we update the best move.
                Penalty p = -sets[set_id_3].score;
                //Penalty p2 = 0;
                //for (ElementId element_id_2: instance.set(set_id_3).elements)
                //    if (solution.covers(element_id_2) == 0)
                //        p2 -= solution_penalties[element_id_2];
                //if (p2 != p) {
                //    std::cout << "set_id_3 p2 " << p2 << " p " << p << std::endl;
                //}
                if (set_id_3_best == -1 // First move considered.
                        || p3_best > p // Strictly better.
                        // Equivalent, but set_id_1 and set_id_2 have not been
                        // considered for a longer time.
                        || (p3_best == p
                            && sets[set_id_3_best].timestamp > sets[set_id_3].timestamp)) {
                    set_id_3_best = set_id_3;
                    p3_best = p;
                }
            }
            if (set_id_3_best != -1) {
                for (SetId set_id_4: solution.sets()) {
                    if (set_id_4 == component.set_id_last_added
                            || neighbor_sets.contains(set_id_4)
                            || instance.set(set_id_4).component != component_id)
                        continue;
                    // If the new solution is better, we update the best move.
                    Penalty p = sets[set_id_4].score;
                    //Penalty p2 = 0;
                    //for (ElementId element_id_2: instance.set(set_id_4).elements)
                    //    if (solution.covers(element_id_2) == 1)
                    //        p2 += solution_penalties[element_id_2];
                    //if (p2 != p) {
                    //    std::cout << "set_id_4 p2 " << p2 << " p " << p << std::endl;
                    //}
                    if (set_id_4_best == -1 // First move considered.
                            || p4_best > p // Strictly better.
                            // Equivalent, but set_id_1 and set_id_2 have not been
                            // considered for a longer time.
                            || (p4_best == p
                                && sets[set_id_4_best].timestamp > sets[set_id_4].timestamp)) {
                        set_id_4_best = set_id_4;
                        p4_best = p;
                    }
                }
            }
            if (set_id_3_best != -1 && set_id_4_best != -1) {
                //std::cout
                //    << "p3_best " << p3_best
                //    << " p4_best " << p4_best
                //    << " p3_best + p4_best " << p3_best + p4_best
                //    << std::endl;
                if (set_id_1_best == -1
                        || p_best > p3_best + p4_best
                        || (p_best == p3_best + p4_best
                            && sets[set_id_1_best].timestamp + sets[set_id_2_best].timestamp
                            > sets[set_id_3_best].timestamp + sets[set_id_4_best].timestamp)) {
                    //std::cout << "toto" << std::endl;
                    p_best = p3_best + p4_best;
                    set_id_1_best = set_id_3_best;
                    set_id_2_best = set_id_4_best;
                    neighborhood_2_improvement = true;
                }
            }

            std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
            output.neighborhood_2_time += std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count();
        }

        if (!neighborhood_2_improvement) {
            output.neighborhood_1_improvements++;
        } else {
            output.neighborhood_2_improvements++;
        }

        if (set_id_1_best != -1) {
            if (instance.set(set_id_1_best).component != component_id) {
                throw std::runtime_error("1");
            }
            if (instance.set(set_id_2_best).component != component_id) {
                throw std::runtime_error("2");
            }
            //std::cout << "set_id_1_best " << set_id_1_best
            //    << " set_id_2_best " << set_id_2_best
            //    << " solution_penalty " << solution_penalty
            //    << " ue " << solution.number_of_uncovered_elements()
            //    << " p_best " << p_best
            //    << std::endl;
            Penalty solution_penalty_old = solution_penalty;
            // Apply move
            solution.add(set_id_1_best);
            // Update scores.
            for (ElementId element_id: instance.set(set_id_1_best).elements) {
                if (solution.covers(element_id) == 1) {
                    solution_penalty -= solution_penalties[element_id];
                    for (SetId set_id: instance.element(element_id).sets)
                        if (!solution.contains(set_id))
                            sets[set_id].score -= solution_penalties[element_id];
                } else if (solution.covers(element_id) == 2) {
                    for (SetId set_id: instance.element(element_id).sets)
                        if (set_id != set_id_1_best && solution.contains(set_id))
                            sets[set_id].score -= solution_penalties[element_id];
                }
            }
            solution.remove(set_id_2_best);
            // Update scores.
            for (ElementId element_id: instance.set(set_id_2_best).elements) {
                if (solution.covers(element_id) == 0) {
                    solution_penalty += solution_penalties[element_id];
                    for (SetId set_id: instance.element(element_id).sets)
                        if (set_id != set_id_2_best)
                            sets[set_id].score += solution_penalties[element_id];
                } else if (solution.covers(element_id) == 1) {
                    for (SetId set_id: instance.element(element_id).sets)
                        if (solution.contains(set_id))
                            sets[set_id].score += solution_penalties[element_id];
                }
            }
            if (solution_penalty_old + p_best != solution_penalty) {
                throw std::runtime_error("\n"
                        "* set_id_1_best: " + std::to_string(set_id_1_best) + "\n"
                        "* set_id_2_best: " + std::to_string(set_id_2_best) + "\n"
                        "* solution_penalty_old: " + std::to_string(solution_penalty_old) + "\n"
                        "* p_best: " + std::to_string(p_best) + "\n"
                        "* solution_penalty: " + std::to_string(solution_penalty) + "\n"
                        );
            }
            // Update sets
            sets[set_id_1_best].timestamp = output.number_of_iterations;
            sets[set_id_2_best].timestamp = output.number_of_iterations;
            sets[set_id_1_best].last_addition = component.iterations;
            sets[set_id_2_best].last_removal  = component.iterations;
            sets[set_id_2_best].iterations += (component.iterations - sets[set_id_2_best].last_addition);

            if (parameters.weights_update_strategy == 0) {
                for (ElementId element_id: instance.set(set_id_2_best).elements) {
                    if (solution.covers(element_id) == 0) {
                        solution_penalty++;
                        solution_penalties[element_id]++;
                        for (SetId set_id: instance.element(element_id).sets)
                            sets[set_id].score++;
                        if (solution_penalties[element_id] > 2e16)
                            reduce = true;
                    }
                }
            }

        }

        // Update penalties.
        if (parameters.weights_update_strategy == 1 && p_best >= 0) {
            //std::cout << "Update penalties" << std::endl;
            for (auto it = solution.elements().out_begin();
                    it != solution.elements().out_end();
                    ++it) {
                ElementId element_id = it->first;
                solution_penalty++;
                solution_penalties[element_id]++;
                for (SetId set_id: instance.element(element_id).sets)
                    sets[set_id].score++;
                if (solution_penalties[element_id] > 2e16)
                    reduce = true;
            }
        }

        if (reduce) {
            std::cout << "reduce" << std::endl;
            for (ElementId element_id = 0;
                    element_id < instance.number_of_elements();
                    ++element_id) {
                solution_penalties[element_id] = (solution_penalties[element_id] - 1) / 2 + 1;
            }
            solution_penalty = 0;
            for (ElementId element_id = 0;
                    element_id < instance.number_of_elements();
                    ++element_id) {
                if (solution.covers(element_id) == 1) {
                    for (SetId set_id: instance.element(element_id).sets)
                        if (solution.contains(set_id))
                            sets[set_id].score += solution_penalties[element_id];
                } else if (solution.covers(element_id) == 0) {
                    solution_penalty += solution_penalties[element_id];
                    for (SetId set_id: instance.element(element_id).sets)
                        sets[set_id].score += solution_penalties[element_id];
                }
            }
            output.number_of_weights_reductions++;
            reduce = false;
        }

        // Update tabu
        component.set_id_last_added = set_id_1_best;
        component.set_id_last_removed = set_id_2_best;
        //std::cout << "it " << component.iterations
            //<< " set_id_1_best " << set_id_1_best
            //<< " set_id_2_best " << set_id_2_best
            //<< " p " << solution.penalty()
            //<< " e " << instance.number_of_elements() - solution.number_of_elements() << "/" << instance.number_of_elements()
            //<< " s " << solution.number_of_sets()
            //<< std::endl;

        // Update component.iterations and component.iterations_without_improvment.
        component.iterations++;
        component.iterations_without_improvment++;
    }

    algorithm_formatter.end();
    return output;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct LocalSearchRowWeighting1Set
{
    Counter timestamp = -1;
    Counter last_addition = -1;
    Counter last_removal = -1;
    Counter iterations = 0;
    Cost score = 0;
};

const LocalSearchRowWeighting1Output setcoveringsolver::setcovering::local_search_row_weighting_1(
        const Instance& instance,
        std::mt19937_64& generator,
        const LocalSearchRowWeighting1Parameters& parameters)
{
    LocalSearchRowWeighting1Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("Row weighting local search 1");

    // Reduction.
    if (parameters.reduction_parameters.reduce) {
        return solve_reduced_instance(
                [&generator](
                    const Instance& instance,
                    const LocalSearchRowWeighting1Parameters& parameters)
                {
                    return local_search_row_weighting_1(
                            instance,
                            generator,
                            parameters);
                },
                instance,
                parameters,
                algorithm_formatter,
                output);
    }

    algorithm_formatter.print_header();

    // Compute initial greedy solution.
    Parameters greedy_parameters;
    greedy_parameters.verbosity_level = 0;
    greedy_parameters.reduction_parameters.reduce = false;
    Solution solution = greedy(instance, greedy_parameters).solution;
    algorithm_formatter.update_solution(solution, "initial solution");

    // Initialize local search structures.
    std::vector<LocalSearchRowWeighting1Set> sets(instance.number_of_sets());
    std::vector<Penalty> solution_penalties(instance.number_of_elements(), 1);
    for (ElementId element_id = 0;
            element_id < instance.number_of_elements();
            ++element_id) {
        if (solution.covers(element_id) == 1)
            for (SetId set_id: instance.element(element_id).sets)
                if (solution.contains(set_id))
                    sets[set_id].score += solution_penalties[element_id];
    }
    SetId set_id_last_removed = -1;
    SetId set_id_last_added = -1;

    Counter number_of_iterations_without_improvement = 0;
    for (output.number_of_iterations = 0;
            !parameters.timer.needs_to_end();
            ++output.number_of_iterations,
            ++number_of_iterations_without_improvement) {
        // Check stop criteria.
        if (parameters.maximum_number_of_iterations != -1
                && output.number_of_iterations >= parameters.maximum_number_of_iterations)
            break;
        if (parameters.maximum_number_of_iterations_without_improvement != -1
                && number_of_iterations_without_improvement >= parameters.maximum_number_of_iterations_without_improvement)
            break;

        while (solution.feasible()) {

            // Update best solution
            if (output.solution.cost() > solution.cost()) {
                std::stringstream ss;
                ss << "iteration " << output.number_of_iterations;
                algorithm_formatter.update_solution(solution, ss.str());
            }

            // Update statistics
            number_of_iterations_without_improvement = 0;

            // Find the best shift move.
            SetId set_id_best = -1;
            Cost score_best = -1;
            for (SetId set_id: solution.sets()) {
                if (set_id_best == -1
                        || score_best > sets[set_id].score
                        || (score_best == sets[set_id].score
                            && sets[set_id_best].timestamp > sets[set_id].timestamp)) {
                    set_id_best = set_id;
                    score_best = sets[set_id].score;
                }
            }
            // It may happen that all sets in the solution are mandatory.
            if (set_id_best == -1) {
                algorithm_formatter.end();
                return output;
            }
            // Apply best move
            solution.remove(set_id_best);
            // Update scores.
            for (ElementId element_id: instance.set(set_id_best).elements) {
                if (solution.covers(element_id) == 0) {
                    for (SetId set_id: instance.element(element_id).sets)
                        if (set_id != set_id_best)
                            sets[set_id].score += solution_penalties[element_id];
                } else if (solution.covers(element_id) == 1) {
                    for (SetId set_id: instance.element(element_id).sets)
                        if (solution.contains(set_id))
                            sets[set_id].score += solution_penalties[element_id];
                }
            }
            // Update sets
            sets[set_id_best].timestamp = output.number_of_iterations;
            sets[set_id_best].iterations += (output.number_of_iterations - sets[set_id_best].last_addition);
            sets[set_id_best].last_removal = output.number_of_iterations;
            // Update tabu
            set_id_last_removed = -1;
            set_id_last_added = -1;
            // Update penalties.
            for (ElementId element_id: instance.set(set_id_best).elements) {
                if (solution.covers(element_id) == 0) {
                    solution_penalties[element_id]++;
                    for (SetId set_id: instance.element(element_id).sets)
                        sets[set_id].score++;
                }
            }
            //std::cout << "it " << output.iterations
                //<< " set_id_best " << set_id_best
                //<< " p " << solution.penalty()
                //<< " e " << instance.number_of_elements() - solution.number_of_elements() << "/" << instance.number_of_elements()
                //<< " s " << solution.number_of_sets()
                //<< std::endl;
        }

        // Find the cheapest set to remove.
        SetId set_id_1_best = -1;
        Cost score1_best = -1;
        for (SetId set_id: solution.sets()) {
            if (set_id == set_id_last_added)
                continue;
            if (set_id_1_best == -1
                    || score1_best > sets[set_id].score
                    || (score1_best == sets[set_id].score
                        && sets[set_id_1_best].timestamp > sets[set_id].timestamp)) {
                set_id_1_best = set_id;
                score1_best = sets[set_id].score;
            }
        }
        // Apply move
        solution.remove(set_id_1_best);
        // Update scores.
        for (ElementId element_id: instance.set(set_id_1_best).elements) {
            if (solution.covers(element_id) == 0) {
                for (SetId set_id: instance.element(element_id).sets)
                    if (set_id != set_id_1_best)
                        sets[set_id].score += solution_penalties[element_id];
            } else if (solution.covers(element_id) == 1) {
                for (SetId set_id: instance.element(element_id).sets)
                    if (solution.contains(set_id))
                        sets[set_id].score += solution_penalties[element_id];
            }
        }
        // Update sets
        sets[set_id_1_best].timestamp = output.number_of_iterations;
        sets[set_id_1_best].last_removal = output.number_of_iterations;
        sets[set_id_1_best].iterations += (output.number_of_iterations - sets[set_id_1_best].last_addition);
        // Update tabu
        set_id_last_removed = set_id_1_best;
        // Update penalties.
        for (ElementId element_id: instance.set(set_id_1_best).elements) {
            if (solution.covers(element_id) == 0) {
                solution_penalties[element_id]++;
                for (SetId set_id: instance.element(element_id).sets)
                    sets[set_id].score++;
            }
        }
        //std::cout << "it " << output.number_of_iterations
        //    << " set_id_1_best " << set_id_1_best
        //    << " score " << score1_best
        //    << " e " << instance.number_of_elements() - solution.number_of_elements() << "/" << instance.number_of_elements()
        //    << " s " << solution.number_of_sets()
        //    << std::endl;

        // Draw randomly an uncovered element e.
        std::uniform_int_distribution<ElementId> d_e(
                0, solution.number_of_uncovered_elements() - 1);
        ElementId e_cur = (solution.elements().out_begin() + d_e(generator))->first;
        //std::cout << "it " << output.iterations
            //<< " e " << e
            //<< " s " << instance.element(element_id).sets.size()
            //<< std::endl;

        // Find the best set to add.
        SetId set_id_2_best = -1;
        Cost scorelement_id_2_best = -1;
        for (SetId set_id: instance.element(e_cur).sets) {
            if (set_id == set_id_last_removed)
                continue;
            if (set_id_2_best == -1
                    || scorelement_id_2_best < sets[set_id].score
                    || (scorelement_id_2_best == sets[set_id].score
                        && sets[set_id_2_best].timestamp > sets[set_id].timestamp)) {
                set_id_2_best = set_id;
                scorelement_id_2_best = sets[set_id].score;
            }
        }
        if (set_id_2_best == -1)
            set_id_2_best = set_id_1_best;
        // Apply move
        solution.add(set_id_2_best);
        // Update scores.
        for (ElementId element_id: instance.set(set_id_2_best).elements) {
            if (solution.covers(element_id) == 1) {
                for (SetId set_id: instance.element(element_id).sets)
                    if (!solution.contains(set_id))
                        sets[set_id].score -= solution_penalties[element_id];
            } else if (solution.covers(element_id) == 2) {
                for (SetId set_id: instance.element(element_id).sets)
                    if (set_id != set_id_2_best && solution.contains(set_id))
                        sets[set_id].score -= solution_penalties[element_id];
            }
        }
        // Update sets
        sets[set_id_2_best].timestamp = output.number_of_iterations;
        sets[set_id_2_best].last_addition = output.number_of_iterations;
        // Update tabu
        set_id_last_added = set_id_2_best;
        //std::cout << "it " << output.number_of_iterations
        //    << " set_id_2_best " << set_id_2_best
        //    << " score " << scorelement_id_2_best
        //    << " e " << instance.number_of_elements() - solution.number_of_elements() << "/" << instance.number_of_elements()
        //    << " s " << solution.number_of_sets()
        //    << std::endl;
    }

    algorithm_formatter.end();
    return output;
}

