#include "setcoveringsolver/algorithms/localsearch_rowweighting.hpp"

#include "setcoveringsolver/algorithms/greedy.hpp"

#include "optimizationtools/containers/indexed_set.hpp"
#include "optimizationtools/containers/indexed_binary_heap.hpp"
#include "optimizationtools/utils/utils.hpp"

#include <thread>

using namespace setcoveringsolver;

LocalSearchRowWeightingOutput& LocalSearchRowWeightingOutput::algorithm_end(
        optimizationtools::Info& info)
{
    FFOT_PUT(info, "Algorithm", "NumberOfIterations", number_of_iterations);
    Output::algorithm_end(info);
    FFOT_VER(info, "Number of iterations:  " << number_of_iterations << std::endl);
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
    /** When to start optimizing next component. */
    Counter iteration_max;
};

struct LocalSearchRowWeightingSet
{
    Counter timestamp = -1;
    Counter last_addition = -1;
    Counter last_removal = -1;
    Counter iterations = 0;
};

LocalSearchRowWeightingOutput setcoveringsolver::localsearch_rowweighting(
        Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeightingOptionalParameters parameters)
{
    init_display(instance, parameters.info);
    FFOT_VER(parameters.info,
               "Algorithm" << std::endl
            << "---------" << std::endl
            << "Row Weighting Local Search" << std::endl
            << std::endl
            << "Parameters" << std::endl
            << "----------" << std::endl
            << "Maximum number of iterations:                      " << parameters.maximum_number_of_iterations << std::endl
            << "Maximum number of iterations without improvement:  " << parameters.maximum_number_of_iterations_without_improvement << std::endl
            << std::endl);

    // Instance pre-processing.
    instance.fix_identical(parameters.info);
    instance.compute_set_neighbors(6, parameters.info);
    instance.compute_components();

    LocalSearchRowWeightingOutput output(instance, parameters.info);

    // Compute initial greedy solution.
    Solution solution = greedy(instance).solution;
    std::stringstream ss;
    ss << "initial solution";
    output.update_solution(solution, ss, parameters.info);

    // Initialize local search structures.
    std::vector<LocalSearchRowWeightingSet> sets(instance.number_of_sets());
    for (SetId s: solution.sets())
        sets[s].last_addition = 0;
    std::vector<LocalSearchRowWeightingComponent> components(instance.number_of_components());
    for (ComponentId c = 0; c < instance.number_of_components(); ++c)
        components[c].iteration_max = ((c == 0)? 0: components[c - 1].iteration_max)
            + instance.component(c).elements.size();
    std::vector<Penalty> solution_penalties(instance.number_of_elements(), 1);

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
        if (output.number_of_iterations % (components.back().iteration_max + 1) >= components[c].iteration_max) {
            c = (c + 1) % instance.number_of_components();
            //std::cout << "c " << c << " " << components[c].iteration_max
                //<< " e " << instance.component(c).elements.size()
                //<< " s " << instance.component(c).sets.size()
                //<< std::endl;
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
            Cost  p_best = -1;
            // For each set s of the current solution which belongs to the
            // currently considered component and is not mandatory.
            for (SetId s: solution.sets()) {
                if (instance.set(s).component != c || instance.set(s).mandatory)
                    continue;
                Penalty p = 0;
                for (ElementId e: instance.set(s).elements)
                    if (solution.covers(e) == 1)
                        p += solution_penalties[e];
                // Update best move.
                if (s_best == -1 // First move considered.
                        || p_best > p // Strictly better.
                        // Equivalent, but s has not been considered for a
                        // longer time.
                        || (p_best == p
                            && sets[s_best].timestamp > sets[s].timestamp)) {
                    s_best = s;
                    p_best = p;
                }
            }
            // Apply best move
            solution.remove(s_best);
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

        // Draw randomly an uncovered element e.
        std::uniform_int_distribution<ElementId> d_e(
                0, solution.number_of_uncovered_elements() - 1);
        ElementId e = (solution.elements().out_begin() + d_e(generator))->first;
        //std::cout << "it " << output.iterations
            //<< " e " << e
            //<< " s " << instance.element(e).sets.size()
            //<< std::endl;

        // Find the best swap move.
        SetId s1_best = -1;
        SetId s2_best = -1;
        Cost   p_best = -1;
        // For each set s1 covering element e which is not part of the solution
        // and which is not the last set removed.
        for (SetId s1: instance.element(e).sets) {
            if (s1 == component.s_last_removed)
                continue;
            Penalty p0 = 0;
            for (ElementId e2: instance.set(s1).elements)
                if (solution.covers(e2) == 0)
                    p0 -= solution_penalties[e2];
            solution.add(s1);
            if (p_best == -1 || p0 <= p_best) {
                // For each neighbor s2 of s1 which is neither part of the
                // solution, nor the last set added, nor mandatory.
                for (SetId s2: instance.set(s1).neighbors) {
                    if (s2 == component.s_last_added
                            || instance.set(s2).mandatory
                            || !solution.contains(s2))
                        continue;
                    Penalty p = p0;
                    for (ElementId e2: instance.set(s2).elements)
                        if (solution.covers(e2) == 1)
                            p += solution_penalties[e2];
                    // If the new solution is better, we update the best move.
                    if (s1_best == -1 // First move considered.
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
            }
            solution.remove(s1);
        }
        if (s1_best != -1) {
            // Apply move
            solution.add(s1_best);
            solution.remove(s2_best);
            // Update sets
            sets[s1_best].timestamp = output.number_of_iterations;
            sets[s2_best].timestamp = output.number_of_iterations;
            sets[s1_best].last_addition = component.iterations;
            sets[s2_best].last_removal  = component.iterations;
            sets[s2_best].iterations += (component.iterations - sets[s2_best].last_addition);

            // Update penalties.
            bool reduce = false;
            for (ElementId e: instance.set(s2_best).elements) {
                if (solution.covers(e) == 0) {
                    solution_penalties[e]++;
                    if (solution_penalties[e] > std::numeric_limits<Cost>::max() / 2)
                        reduce = true;
                }
            }
            if (reduce) {
                //std::cout << "reduce" << std::endl;
                for (ElementId e = 0; e < instance.number_of_elements(); ++e)
                    solution_penalties[e] = (solution_penalties[e] - 1) / 2 + 1;
            }
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

LocalSearchRowWeighting2Output& LocalSearchRowWeighting2Output::algorithm_end(
        optimizationtools::Info& info)
{
    FFOT_PUT(info, "Algorithm", "NumberOfIterations", number_of_iterations);
    Output::algorithm_end(info);
    FFOT_VER(info, "Number of iterations:  " << number_of_iterations << std::endl);
    return *this;
}

struct LocalSearchRowWeighting2Set
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
    FFOT_VER(parameters.info,
               "Algorithm" << std::endl
            << "---------" << std::endl
            << "Row Weighting Local Search 2" << std::endl
            << std::endl
            << "Parameters" << std::endl
            << "----------" << std::endl
            << "Maximum number of iterations:                      " << parameters.maximum_number_of_iterations << std::endl
            << "Maximum number of iterations without improvement:  " << parameters.maximum_number_of_iterations_without_improvement << std::endl
            << std::endl);

    // Instance pre-processing.
    instance.fix_identical(parameters.info);

    LocalSearchRowWeighting2Output output(instance, parameters.info);

    // Compute initial greedy solution.
    Solution solution = greedy(instance).solution;
    std::stringstream ss;
    ss << "initial solution";
    output.update_solution(solution, ss, parameters.info);

    // Initialize local search structures.
    std::vector<LocalSearchRowWeighting2Set> sets(instance.number_of_sets());
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

