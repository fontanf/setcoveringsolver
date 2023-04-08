#include "setcoveringsolver/algorithms/localsearch_rowweighting.hpp"

#include "setcoveringsolver/algorithms/greedy.hpp"

#include "optimizationtools/containers/indexed_set.hpp"
#include "optimizationtools/containers/indexed_binary_heap.hpp"
#include "optimizationtools/utils/utils.hpp"

#include <thread>

using namespace setcoveringsolver;

LocalSearchRowWeighting2Output& LocalSearchRowWeighting2Output::algorithm_end(
        optimizationtools::Info& info)
{
    info.add_to_json("Algorithm", "NumberOfIterations", number_of_iterations);
    Output::algorithm_end(info);
    info.os() << "Number of iterations:          " << number_of_iterations << std::endl;
    info.os() << "Neighborhood 1 improvements:   " << neighborhood_1_improvements << std::endl;
    info.os() << "Neighborhood 2 improvements:   " << neighborhood_2_improvements << std::endl;
    info.os() << "Neighborhood 1 time:           " << neighborhood_1_time << std::endl;
    info.os() << "Neighborhood 2 time:           " << neighborhood_2_time << std::endl;
    info.os() << "Number of weights reductions:  " << neighborhood_2_time << std::endl;
    return *this;
}

struct LocalSearchRowWeightingComponent
{
    /** Last set added to the current solution. */
    SetId s_last_added = -1;

    /** Last set removed from the current solution. */
    SetId s_last_removed = -1;

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

LocalSearchRowWeighting2Output setcoveringsolver::localsearch_rowweighting_2(
        Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeighting2OptionalParameters parameters)
{
    init_display(instance, parameters.info);
    parameters.info.os()
            << "Algorithm" << std::endl
            << "---------" << std::endl
            << "Row Weighting Local Search 2" << std::endl
            << std::endl
            << "Parameters" << std::endl
            << "----------" << std::endl
            << "Neighborhood 1:                                    " << parameters.neighborhood_1 << std::endl
            << "Neighborhood 2:                                    " << parameters.neighborhood_2 << std::endl
            << "Weights update strategy:                           " << parameters.weights_update_strategy << std::endl
            << "Maximum number of iterations:                      " << parameters.maximum_number_of_iterations << std::endl
            << "Maximum number of iterations without improvement:  " << parameters.maximum_number_of_iterations_without_improvement << std::endl
            << std::endl;

    // Instance pre-processing.
    instance.fix_identical(parameters.info);
    instance.compute_set_neighbors(6, parameters.info);
    if (parameters.neighborhood_1 == 1 || parameters.neighborhood_2 == 1)
        instance.compute_element_neighbor_sets(parameters.info);
    instance.compute_components();

    LocalSearchRowWeighting2Output output(instance, parameters.info);

    // Compute initial greedy solution.
    Solution solution = greedy(instance).solution;
    //Solution solution = greedy_lin(instance).solution;
    std::stringstream ss;
    ss << "initial solution";
    output.update_solution(solution, ss, parameters.info);

    // Initialize local search structures.
    std::vector<LocalSearchRowWeightingSet> sets(instance.number_of_sets());
    for (SetId s: solution.sets())
        sets[s].last_addition = 0;
    Penalty solution_penalty = 0;
    std::vector<Penalty> solution_penalties(instance.number_of_elements(), 1);
    for (ElementId e = 0; e < instance.number_of_elements(); ++e)
        if (solution.covers(e) == 1)
            for (SetId s: instance.element(e).sets)
                if (solution.contains(s))
                    sets[s].score += solution_penalties[e];
    bool reduce = false;

    std::vector<LocalSearchRowWeightingComponent> components(instance.number_of_components());
    components[0].itmode_start = 0;
    for (ComponentId c = 0; c < instance.number_of_components(); ++c) {
        components[c].itmode_end = components[c].itmode_start
            + instance.component(c).elements.size();
        if (c + 1 < instance.number_of_components())
            components[c + 1].itmode_start = components[c].itmode_end;
    }

    optimizationtools::IndexedSet neighbor_sets(instance.number_of_sets());

    ComponentId c = 0;
    Counter number_of_iterations_without_improvement = 0;
    for (output.number_of_iterations = 0;
            !parameters.info.needs_to_end();
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
        //    << " start " << components[c].itmode_start
        //    << " end " << components[c].itmode_end
        //    << std::endl;
        Counter itmod = output.number_of_iterations % (components.back().itmode_end);
        while (itmod < components[c].itmode_start
                || itmod >= components[c].itmode_end) {
            c = (c + 1) % instance.number_of_components();
            //std::cout << "it " << output.number_of_iterations
            //    << " % " << output.number_of_iterations % (components.back().itmode_end)
            //    << " c " << c
            //    << " start " << components[c].itmode_start
            //    << " end " << components[c].itmode_end
            //    << std::endl;
        }
        LocalSearchRowWeightingComponent& component = components[c];

        while (solution.feasible(c)) {
            // New best solution
            if (output.solution.cost(c) > solution.cost(c)) {
                // Update best solution
                std::stringstream ss;
                ss << "it " << output.number_of_iterations << " comp " << c;
                output.update_solution(solution, c, ss, parameters.info);
            }
            // Update statistics
            number_of_iterations_without_improvement = 0;
            if (component.iterations_without_improvment > 0)
                component.iterations_without_improvment = 0;

            // Find the best shift move.
            SetId s_best = -1;
            Cost p_best = -1;
            // For each set s of the current solution which belongs to the
            // currently considered component and is not mandatory.
            for (SetId s: solution.sets()) {
                if (instance.set(s).component != c || instance.set(s).mandatory)
                    continue;
                Penalty p = -sets[s].score;
                // Update best move.
                if (s_best == -1 // First move considered.
                        || p_best < p // Strictly better.
                        // Equivalent, but s has not been considered for a
                        // longer time.
                        || (p_best == p
                            && sets[s_best].timestamp > sets[s].timestamp)) {
                    s_best = s;
                    p_best = p;
                }
            }
            if (s_best == -1) {
                // Happens when all sets of the component which are in the
                // solution are mandatory.
                component.optimal = true;
                //std::cout << "c " << c << " optimal" << std::endl;
                bool all_component_optimal = true;
                components[0].itmode_start = 0;
                for (ComponentId c = 0; c < instance.number_of_components(); ++c) {
                    //std::cout << "comp " << c << " opt " << component.optimal << std::endl;
                    components[c].itmode_end = components[c].itmode_start;
                    if (components[c].optimal) {
                    } else {
                        components[c].itmode_end += instance.component(c).elements.size();
                        all_component_optimal = false;
                    }
                    if (c + 1 < instance.number_of_components())
                        components[c + 1].itmode_start = components[c].itmode_end;
                }
                // If all components are optimal, stop here.
                if (all_component_optimal)
                    return output.algorithm_end(parameters.info);
                break;
            }
            // Apply best move
            solution.remove(s_best);
            // Update scores.
            for (ElementId e: instance.set(s_best).elements) {
                if (solution.covers(e) == 0) {
                    solution_penalty += solution_penalties[e];
                    for (SetId s: instance.element(e).sets)
                        if (s != s_best)
                            sets[s].score += solution_penalties[e];
                } else if (solution.covers(e) == 1) {
                    for (SetId s: instance.element(e).sets)
                        if (solution.contains(s))
                            sets[s].score += solution_penalties[e];
                }
            }
            // Update sets
            sets[s_best].timestamp = output.number_of_iterations;
            sets[s_best].iterations += (component.iterations - sets[s_best].last_addition);
            sets[s_best].last_removal = component.iterations;
            // Update tabu
            component.s_last_removed = s_best;
            //std::cout << "it " << output.iterations
            //<< " s_best " << s_best
            //<< " p " << solution.penalty()
            //<< " e " << instance.number_of_elements() - solution.number_of_elements() << "/" << instance.number_of_elements()
            //<< " s " << solution.number_of_sets()
            //<< std::endl;
        }
        if (components[c].optimal)
            continue;

        if (solution.cost(c) != output.solution.cost(c) - 1) {
            throw std::runtime_error("component " + std::to_string(c)
                    + " size " + std::to_string(instance.component(c).sets.size()));
        }

        bool neighborhood_2_improvement = false;

        // Draw randomly an uncovered element e.
        std::vector<ElementId> e_candidates;
        for (auto it = solution.elements().out_begin(); it != solution.elements().out_end(); ++it)
            if (instance.element(it->first).component == c)
                e_candidates.push_back(it->first);
        std::uniform_int_distribution<ElementId> d_e(
                0, e_candidates.size() - 1);
        ElementId e = e_candidates[d_e(generator)];
        //std::cout << "it " << output.iterations
            //<< " e " << e
            //<< " s " << instance.element(e).sets.size()
            //<< std::endl;

        std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();

        // Find the best swap move.
        SetId s1_best = -1;
        SetId s2_best = -1;
        Cost p_best = 0;

        if (parameters.neighborhood_1 == 1 || parameters.neighborhood_2 == 1) {
            neighbor_sets.clear();
            for (SetId s: instance.element(e).neighbor_sets)
                neighbor_sets.add(s);
        }

        // For each set s1 covering element e which is not part of the solution
        // and which is not the last set removed.
        for (SetId s1: instance.element(e).sets) {
            if (s1 == component.s_last_removed)
                continue;
            Penalty p0 = 0;
            for (ElementId e2: instance.set(s1).elements)
                if (solution.covers(e2) == 0)
                    p0 -= solution_penalties[e2];
            if (s1_best == -1 || p0 <= p_best) {
                solution.add(s1);
                // For each neighbor s2 of s1 which is neither part of the
                // solution, nor the last set added, nor mandatory.
                auto it_begin = (parameters.neighborhood_1 == 0)?
                    instance.set(s1).neighbors.begin():
                    neighbor_sets.begin();
                auto it_end = (parameters.neighborhood_1 == 0)?
                    instance.set(s1).neighbors.end():
                    neighbor_sets.end();
                for (auto it = it_begin; it != it_end; ++it) {
                    SetId s2 = *it;
                    if (s1 == s2
                            || s2 == component.s_last_added
                            || instance.set(s2).mandatory
                            || !solution.contains(s2))
                        continue;
                    Penalty p = p0;
                    for (ElementId e2: instance.set(s2).elements)
                        if (solution.covers(e2) == 1)
                            p += solution_penalties[e2];
                    // If the new solution is better, we update the best move.
                    if (s1_best == -1
                            || p_best > p // Strictly better.
                            // Equivalent, but s1 and s2 have not been
                            // considered for a longer time.
                            || (p_best == p
                                && sets[s1_best].timestamp + sets[s2_best].timestamp
                                > sets[s1].timestamp + sets[s2].timestamp)) {
                        s1_best = s1;
                        s2_best = s2;
                        p_best = p;
                    }
                }
                solution.remove(s1);
            }
        }

        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
        output.neighborhood_1_time += std::chrono::duration_cast<std::chrono::duration<double>>(t1 - t0).count();

        if (parameters.neighborhood_2 == 1
                || (parameters.neighborhood_2 == 2 && (s1_best == -1 || p_best >= 0))) {
            SetId s3_best = -1;
            SetId s4_best = -1;
            Cost p3_best = -1;
            Cost p4_best = -1;
            for (SetId s3: instance.element(e).sets) {
                if (s3 == component.s_last_removed)
                    continue;
                // If the new solution is better, we update the best move.
                Penalty p = -sets[s3].score;
                //Penalty p2 = 0;
                //for (ElementId e2: instance.set(s3).elements)
                //    if (solution.covers(e2) == 0)
                //        p2 -= solution_penalties[e2];
                //if (p2 != p) {
                //    std::cout << "s3 p2 " << p2 << " p " << p << std::endl;
                //}
                if (s3_best == -1 // First move considered.
                        || p3_best > p // Strictly better.
                        // Equivalent, but s1 and s2 have not been
                        // considered for a longer time.
                        || (p3_best == p
                            && sets[s3_best].timestamp > sets[s3].timestamp)) {
                    s3_best = s3;
                    p3_best = p;
                }
            }
            if (s3_best != -1) {
                for (SetId s4: solution.sets()) {
                    if (s4 == component.s_last_added
                            || instance.set(s4).mandatory
                            || neighbor_sets.contains(s4)
                            || instance.set(s4).component != c)
                        continue;
                    // If the new solution is better, we update the best move.
                    Penalty p = sets[s4].score;
                    //Penalty p2 = 0;
                    //for (ElementId e2: instance.set(s4).elements)
                    //    if (solution.covers(e2) == 1)
                    //        p2 += solution_penalties[e2];
                    //if (p2 != p) {
                    //    std::cout << "s4 p2 " << p2 << " p " << p << std::endl;
                    //}
                    if (s4_best == -1 // First move considered.
                            || p4_best > p // Strictly better.
                            // Equivalent, but s1 and s2 have not been
                            // considered for a longer time.
                            || (p4_best == p
                                && sets[s4_best].timestamp > sets[s4].timestamp)) {
                        s4_best = s4;
                        p4_best = p;
                    }
                }
            }
            if (s3_best != -1 && s4_best != -1) {
                //std::cout
                //    << "p3_best " << p3_best
                //    << " p4_best " << p4_best
                //    << " p3_best + p4_best " << p3_best + p4_best
                //    << std::endl;
                if (s1_best == -1
                        || p_best > p3_best + p4_best
                        || (p_best == p3_best + p4_best
                            && sets[s1_best].timestamp + sets[s2_best].timestamp
                            > sets[s3_best].timestamp + sets[s4_best].timestamp)) {
                    //std::cout << "toto" << std::endl;
                    p_best = p3_best + p4_best;
                    s1_best = s3_best;
                    s2_best = s4_best;
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

        if (s1_best != -1) {
            if (instance.set(s1_best).component != c) {
                throw std::runtime_error("1");
            }
            if (instance.set(s2_best).component != c) {
                throw std::runtime_error("2");
            }
            //std::cout << "s1_best " << s1_best
            //    << " s2_best " << s2_best
            //    << " solution_penalty " << solution_penalty
            //    << " ue " << solution.number_of_uncovered_elements()
            //    << " p_best " << p_best
            //    << std::endl;
            Penalty solution_penalty_old = solution_penalty;
            // Apply move
            solution.add(s1_best);
            // Update scores.
            for (ElementId e: instance.set(s1_best).elements) {
                if (solution.covers(e) == 1) {
                    solution_penalty -= solution_penalties[e];
                    for (SetId s: instance.element(e).sets)
                        if (!solution.contains(s))
                            sets[s].score -= solution_penalties[e];
                } else if (solution.covers(e) == 2) {
                    for (SetId s: instance.element(e).sets)
                        if (s != s1_best && solution.contains(s))
                            sets[s].score -= solution_penalties[e];
                }
            }
            solution.remove(s2_best);
            // Update scores.
            for (ElementId e: instance.set(s2_best).elements) {
                if (solution.covers(e) == 0) {
                    solution_penalty += solution_penalties[e];
                    for (SetId s: instance.element(e).sets)
                        if (s != s2_best)
                            sets[s].score += solution_penalties[e];
                } else if (solution.covers(e) == 1) {
                    for (SetId s: instance.element(e).sets)
                        if (solution.contains(s))
                            sets[s].score += solution_penalties[e];
                }
            }
            if (solution_penalty_old + p_best != solution_penalty) {
                throw std::runtime_error("\n"
                        "* s1_best: " + std::to_string(s1_best) + "\n"
                        "* s2_best: " + std::to_string(s2_best) + "\n"
                        "* solution_penalty_old: " + std::to_string(solution_penalty_old) + "\n"
                        "* p_best: " + std::to_string(p_best) + "\n"
                        "* solution_penalty: " + std::to_string(solution_penalty) + "\n"
                        );
            }
            // Update sets
            sets[s1_best].timestamp = output.number_of_iterations;
            sets[s2_best].timestamp = output.number_of_iterations;
            sets[s1_best].last_addition = component.iterations;
            sets[s2_best].last_removal  = component.iterations;
            sets[s2_best].iterations += (component.iterations - sets[s2_best].last_addition);

            if (parameters.weights_update_strategy == 0) {
                for (ElementId e: instance.set(s2_best).elements) {
                    if (solution.covers(e) == 0) {
                        solution_penalty++;
                        solution_penalties[e]++;
                        for (SetId s: instance.element(e).sets)
                            sets[s].score++;
                        if (solution_penalties[e] > 2e16)
                            reduce = true;
                    }
                }
            }

        }

        // Update penalties.
        if (parameters.weights_update_strategy == 1 && p_best >= 0) {
            //std::cout << "Update penalties" << std::endl;
            for (auto it = solution.elements().out_begin();
                    it != solution.elements().out_end(); ++it) {
                ElementId e = it->first;
                solution_penalty++;
                solution_penalties[e]++;
                for (SetId s: instance.element(e).sets)
                    sets[s].score++;
                if (solution_penalties[e] > 2e16)
                    reduce = true;
            }
        }

        if (reduce) {
            std::cout << "reduce" << std::endl;
            for (ElementId e = 0; e < instance.number_of_elements(); ++e)
                solution_penalties[e] = (solution_penalties[e] - 1) / 2 + 1;
            solution_penalty = 0;
            for (ElementId e = 0; e < instance.number_of_elements(); ++e) {
                if (solution.covers(e) == 1) {
                    for (SetId s: instance.element(e).sets)
                        if (solution.contains(s))
                            sets[s].score += solution_penalties[e];
                } else if (solution.covers(e) == 0) {
                    solution_penalty += solution_penalties[e];
                    for (SetId s: instance.element(e).sets)
                        sets[s].score += solution_penalties[e];
                }
            }
            output.number_of_weights_reductions++;
            reduce = false;
        }

        // Update tabu
        component.s_last_added = s1_best;
        component.s_last_removed = s2_best;
        //std::cout << "it " << component.iterations
            //<< " s1_best " << s1_best
            //<< " s2_best " << s2_best
            //<< " p " << solution.penalty()
            //<< " e " << instance.number_of_elements() - solution.number_of_elements() << "/" << instance.number_of_elements()
            //<< " s " << solution.number_of_sets()
            //<< std::endl;

        // Update component.iterations and component.iterations_without_improvment.
        component.iterations++;
        component.iterations_without_improvment++;
    }

    return output.algorithm_end(parameters.info);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

LocalSearchRowWeighting1Output& LocalSearchRowWeighting1Output::algorithm_end(
        optimizationtools::Info& info)
{
    info.add_to_json("Algorithm", "NumberOfIterations", number_of_iterations);
    Output::algorithm_end(info);
    info.os() << "Number of iterations:          " << number_of_iterations << std::endl;
    return *this;
}

struct LocalSearchRowWeighting1Set
{
    Counter timestamp = -1;
    Counter last_addition = -1;
    Counter last_removal = -1;
    Counter iterations = 0;
    Cost score = 0;
};

LocalSearchRowWeighting1Output setcoveringsolver::localsearch_rowweighting_1(
        Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeighting1OptionalParameters parameters)
{
    init_display(instance, parameters.info);
    parameters.info.os()
            << "Algorithm" << std::endl
            << "---------" << std::endl
            << "Row Weighting Local Search 1" << std::endl
            << std::endl
            << "Parameters" << std::endl
            << "----------" << std::endl
            << "Maximum number of iterations:                      " << parameters.maximum_number_of_iterations << std::endl
            << "Maximum number of iterations without improvement:  " << parameters.maximum_number_of_iterations_without_improvement << std::endl
            << std::endl;

    // Instance pre-processing.
    instance.fix_identical(parameters.info);

    LocalSearchRowWeighting1Output output(instance, parameters.info);

    // Compute initial greedy solution.
    Solution solution = greedy(instance).solution;
    std::stringstream ss;
    ss << "initial solution";
    output.update_solution(solution, ss, parameters.info);

    // Initialize local search structures.
    std::vector<LocalSearchRowWeighting1Set> sets(instance.number_of_sets());
    std::vector<Penalty> solution_penalties(instance.number_of_elements(), 1);
    for (ElementId e = 0; e < instance.number_of_elements(); ++e)
        if (solution.covers(e) == 1)
            for (SetId s: instance.element(e).sets)
                if (solution.contains(s))
                    sets[s].score += solution_penalties[e];
    SetId s_last_removed = -1;
    SetId s_last_added = -1;

    Counter number_of_iterations_without_improvement = 0;
    for (output.number_of_iterations = 0;
            !parameters.info.needs_to_end();
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
                output.update_solution(solution, ss, parameters.info);
            }

            // Update statistics
            number_of_iterations_without_improvement = 0;

            // Find the best shift move.
            SetId s_best = -1;
            Cost score_best = -1;
            for (SetId s: solution.sets()) {
                if (instance.set(s).mandatory)
                    continue;
                if (s_best == -1
                        || score_best > sets[s].score
                        || (score_best == sets[s].score
                            && sets[s_best].timestamp > sets[s].timestamp)) {
                    s_best = s;
                    score_best = sets[s].score;
                }
            }
            // It may happen that all sets in the solution are mandatory.
            if (s_best == -1)
                return output.algorithm_end(parameters.info);
            // Apply best move
            solution.remove(s_best);
            // Update scores.
            for (ElementId e: instance.set(s_best).elements) {
                if (solution.covers(e) == 0) {
                    for (SetId s: instance.element(e).sets)
                        if (s != s_best)
                            sets[s].score += solution_penalties[e];
                } else if (solution.covers(e) == 1) {
                    for (SetId s: instance.element(e).sets)
                        if (solution.contains(s))
                            sets[s].score += solution_penalties[e];
                }
            }
            // Update sets
            sets[s_best].timestamp = output.number_of_iterations;
            sets[s_best].iterations += (output.number_of_iterations - sets[s_best].last_addition);
            sets[s_best].last_removal = output.number_of_iterations;
            // Update tabu
            s_last_removed = -1;
            s_last_added = -1;
            // Update penalties.
            for (ElementId e: instance.set(s_best).elements) {
                if (solution.covers(e) == 0) {
                    solution_penalties[e]++;
                    for (SetId s: instance.element(e).sets)
                        sets[s].score++;
                }
            }
            //std::cout << "it " << output.iterations
                //<< " s_best " << s_best
                //<< " p " << solution.penalty()
                //<< " e " << instance.number_of_elements() - solution.number_of_elements() << "/" << instance.number_of_elements()
                //<< " s " << solution.number_of_sets()
                //<< std::endl;
        }

        // Find the cheapest set to remove.
        SetId s1_best = -1;
        Cost score1_best = -1;
        for (SetId s: solution.sets()) {
            if (s == s_last_added
                    || instance.set(s).mandatory)
                continue;
            if (s1_best == -1
                    || score1_best > sets[s].score
                    || (score1_best == sets[s].score
                        && sets[s1_best].timestamp > sets[s].timestamp)) {
                s1_best = s;
                score1_best = sets[s].score;
            }
        }
        // Apply move
        solution.remove(s1_best);
        // Update scores.
        for (ElementId e: instance.set(s1_best).elements) {
            if (solution.covers(e) == 0) {
                for (SetId s: instance.element(e).sets)
                    if (s != s1_best)
                        sets[s].score += solution_penalties[e];
            } else if (solution.covers(e) == 1) {
                for (SetId s: instance.element(e).sets)
                    if (solution.contains(s))
                        sets[s].score += solution_penalties[e];
            }
        }
        // Update sets
        sets[s1_best].timestamp = output.number_of_iterations;
        sets[s1_best].last_removal = output.number_of_iterations;
        sets[s1_best].iterations += (output.number_of_iterations - sets[s1_best].last_addition);
        // Update tabu
        s_last_removed = s1_best;
        // Update penalties.
        for (ElementId e: instance.set(s1_best).elements) {
            if (solution.covers(e) == 0) {
                solution_penalties[e]++;
                for (SetId s: instance.element(e).sets)
                    sets[s].score++;
            }
        }
        //std::cout << "it " << output.number_of_iterations
        //    << " s1_best " << s1_best
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
            //<< " s " << instance.element(e).sets.size()
            //<< std::endl;

        // Find the best set to add.
        SetId s2_best = -1;
        Cost score2_best = -1;
        for (SetId s: instance.element(e_cur).sets) {
            if (s == s_last_removed)
                continue;
            if (s2_best == -1
                    || score2_best < sets[s].score
                    || (score2_best == sets[s].score
                        && sets[s2_best].timestamp > sets[s].timestamp)) {
                s2_best = s;
                score2_best = sets[s].score;
            }
        }
        if (s2_best == -1)
            s2_best = s1_best;
        // Apply move
        solution.add(s2_best);
        // Update scores.
        for (ElementId e: instance.set(s2_best).elements) {
            if (solution.covers(e) == 1) {
                for (SetId s: instance.element(e).sets)
                    if (!solution.contains(s))
                        sets[s].score -= solution_penalties[e];
            } else if (solution.covers(e) == 2) {
                for (SetId s: instance.element(e).sets)
                    if (s != s2_best && solution.contains(s))
                        sets[s].score -= solution_penalties[e];
            }
        }
        // Update sets
        sets[s2_best].timestamp = output.number_of_iterations;
        sets[s2_best].last_addition = output.number_of_iterations;
        // Update tabu
        s_last_added = s2_best;
        //std::cout << "it " << output.number_of_iterations
        //    << " s2_best " << s2_best
        //    << " score " << score2_best
        //    << " e " << instance.number_of_elements() - solution.number_of_elements() << "/" << instance.number_of_elements()
        //    << " s " << solution.number_of_sets()
        //    << std::endl;
    }

    return output.algorithm_end(parameters.info);
}

