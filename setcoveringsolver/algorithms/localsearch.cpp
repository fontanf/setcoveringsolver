#include "setcoveringsolver/algorithms/localsearch.hpp"

#include "setcoveringsolver/algorithms/greedy.hpp"

#include "optimizationtools/indexed_set.hpp"
#include "optimizationtools/indexed_binary_heap.hpp"
#include "optimizationtools/utils.hpp"

#include <thread>

using namespace setcoveringsolver;

LocalSearchOutput& LocalSearchOutput::algorithm_end(Info& info)
{
    //PUT(info, "Algorithm", "Iterations", iterations);
    Output::algorithm_end(info);
    //VER(info, "Iterations: " << iterations << std::endl);
    return *this;
}

struct LocalSearchComponent
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

struct LocalSearchSet
{
    Counter timestamp = -1;
    Counter last_addition = -1;
    Counter last_removal = -1;
    Counter iterations = 0;
};

void localsearch_worker(
        const Instance& instance,
        Seed seed,
        LocalSearchOptionalParameters parameters,
        LocalSearchOutput& output,
        Counter thread_id)
{
    std::mt19937_64 generator(seed);
    parameters.info.output->mutex_sol.lock();
    Solution solution = output.solution;
    parameters.info.output->mutex_sol.unlock();

    // Initialize local search structures.
    std::vector<LocalSearchSet> sets(instance.set_number());
    for (SetId s: solution.sets())
        sets[s].last_addition = 0;
    std::vector<LocalSearchComponent> components(instance.component_number());
    for (ComponentId c = 0; c < instance.component_number(); ++c)
        components[c].iteration_max = ((c == 0)? 0: components[c - 1].iteration_max)
            + instance.component(c).elements.size();

    ComponentId c = 0;
    for (Counter iterations = 1; parameters.info.check_time(); ++iterations) {
        // Compute component
        if (iterations % (components.back().iteration_max + 1) >= components[c].iteration_max) {
            c = (c + 1) % instance.component_number();
            //std::cout << "c " << c << " " << components[c].iteration_max
                //<< " e " << instance.component(c).elements.size()
                //<< " s " << instance.component(c).sets.size()
                //<< std::endl;
        }
        LocalSearchComponent& component = components[c];

        while (solution.feasible(c)) {
            // New best solution
            if (output.solution.cost(c) > solution.cost(c)) {
                // Update best solution
                std::stringstream ss;
                ss << "thread " << thread_id
                    << ", it " << iterations
                    << ", comp " << c
                    << " (" << component.iterations_without_improvment << ")";
                output.update_solution(solution, c, ss, parameters.info);
            }
            // Update statistics
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
                solution.remove(s);
                // Update best move.
                if (s_best == -1 // First move considered.
                        || p_best > solution.penalty() // Strictly better.
                        // Equivalent, but s has not been considered for a
                        // longer time.
                        || (p_best == solution.penalty()
                            && sets[s_best].timestamp > sets[s].timestamp)) {
                    s_best = s;
                    p_best = solution.penalty();
                }
                solution.add(s);
            }
            // Apply best move
            solution.remove(s_best);
            // Update sets
            sets[s_best].timestamp = iterations;
            sets[s_best].iterations += (component.iterations - sets[s_best].last_addition);
            sets[s_best].last_removal = component.iterations;
            // Update tabu
            component.s_last_removed = s_best;
            //std::cout << "it " << output.iterations
                //<< " s_best " << s_best
                //<< " p " << solution.penalty()
                //<< " e " << instance.element_number() - solution.element_number() << "/" << instance.element_number()
                //<< " s " << solution.set_number()
                //<< std::endl;
        }

        // Draw randomly an uncovered element e.
        std::uniform_int_distribution<ElementId> d_e(
                0, solution.uncovered_element_number() - 1);
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
            solution.add(s1);
            if (p_best == -1 || solution.penalty() <= p_best) {
                // For each neighbor s2 of s1 which is neither part of the
                // solution, nor the last set added, nor mandatory.
                for (SetId s2: instance.set(s1).neighbors) {
                    if (s2 == component.s_last_added
                            || instance.set(s2).mandatory
                            || !solution.contains(s2))
                        continue;
                    solution.remove(s2);
                    // If the new solution is better, we update the best move.
                    if (s1_best == -1 // First move considered.
                            || p_best > solution.penalty() // Strictly better.
                            // Equivalent, but s1 and s2 have not been
                            // considered for a longer time.
                            || (p_best == solution.penalty()
                                && sets[s1_best].timestamp + sets[s2_best].timestamp
                                > sets[s1].timestamp + sets[s2].timestamp)) {
                        s1_best = s1;
                        s2_best = s2;
                        p_best = solution.penalty();
                    }
                    solution.add(s2);
                }
            }
            solution.remove(s1);
        }
        if (s1_best != -1) {
            // Apply move
            solution.add(s1_best);
            solution.remove(s2_best);
            // Update sets
            sets[s1_best].timestamp = iterations;
            sets[s2_best].timestamp = iterations;
            sets[s1_best].last_addition = component.iterations;
            sets[s2_best].last_removal  = component.iterations;
            sets[s2_best].iterations += (component.iterations - sets[s2_best].last_addition);
        }
        // Update tabu
        component.s_last_added   = s1_best;
        component.s_last_removed = s2_best;
        //std::cout << "it " << component.iterations
            //<< " s1_best " << s1_best
            //<< " s2_best " << s2_best
            //<< " p " << solution.penalty()
            //<< " e " << instance.element_number() - solution.element_number() << "/" << instance.element_number()
            //<< " s " << solution.set_number()
            //<< std::endl;

        // Update penalties: we increment the penalty of each uncovered element.
        // "reduce" becomes true if we divide by 2 all penalties to avoid
        // integer overflow (this very rarely occur in practice).
        bool reduce = false;
        for (auto it = solution.elements().out_begin(); it != solution.elements().out_end(); ++it) {
            solution.increment_penalty(it->first);
            if (solution.penalty(it->first) > std::numeric_limits<Cost>::max() / instance.unfixed_element_number())
                reduce = true;
        }
        if (reduce) {
            //std::cout << "reduce" << std::endl;
            for (ElementId e = 0; e < instance.element_number(); ++e)
                solution.set_penalty(e, (solution.penalty(e) - 1) / 2 + 1);
        }

        // Update component.iterations and component.iterations_without_improvment.
        component.iterations++;
        component.iterations_without_improvment++;
    }
}

LocalSearchOutput setcoveringsolver::localsearch(
        Instance& instance,
        std::mt19937_64& generator,
        LocalSearchOptionalParameters parameters)
{
    VER(parameters.info, "*** localsearch ***" << std::endl);

    // Instance pre-processing.
    instance.fix_identical(parameters.info);
    instance.compute_set_neighbors(parameters.thread_number, parameters.info);
    instance.compute_components(parameters.info);

    // Compute initial greedy solution.
    LocalSearchOutput output(instance, parameters.info);
    Solution solution = greedy(instance).solution;
    std::stringstream ss;
    ss << "initial solution";
    output.update_solution(solution, ss, parameters.info);

    auto seeds = optimizationtools::bob_floyd(parameters.thread_number, std::numeric_limits<Seed>::max(), generator);
    std::vector<std::thread> threads;
    for (Counter thread_id = 0; thread_id < parameters.thread_number; ++thread_id)
        threads.push_back(std::thread(localsearch_worker, std::ref(instance), seeds[thread_id], parameters, std::ref(output), thread_id));

    for (Counter thread_id = 0; thread_id < parameters.thread_number; ++thread_id)
        threads[thread_id].join();

    return output.algorithm_end(parameters.info);
}

/******************************************************************************/

LocalSearch2Output& LocalSearch2Output::algorithm_end(Info& info)
{
    //PUT(info, "Algorithm", "Iterations", iterations);
    Output::algorithm_end(info);
    //VER(info, "Iterations: " << iterations << std::endl);
    return *this;
}

struct LocalSearch2Set
{
    Counter timestamp = -1;
    Counter last_addition = -1;
    Counter last_removal = -1;
    Counter iterations = 0;
    Cost    score = 0;
};

void localsearch_2_worker(
        const Instance& instance,
        Seed seed,
        LocalSearch2OptionalParameters parameters,
        LocalSearch2Output& output,
        Counter thread_id)
{
    std::mt19937_64 generator(seed);
    parameters.info.output->mutex_sol.lock();
    Solution solution = output.solution;
    parameters.info.output->mutex_sol.unlock();

    // Initialize local search structures.
    std::vector<LocalSearch2Set> sets(instance.set_number());
    for (ElementId e = 0; e < instance.element_number(); ++e)
        if (solution.covers(e) == 1)
            for (SetId s: instance.element(e).sets)
                if (solution.contains(s))
                    sets[s].score += solution.penalty(e);
    SetId s_last_removed = -1;
    SetId s_last_added = -1;

    Counter iterations_without_improvment = 0;
    for (Counter iterations = 1; parameters.info.check_time(); ++iterations, iterations_without_improvment++) {

        while (solution.feasible()) {

            // Update best solution
            if (output.solution.cost() > solution.cost()) {
                std::stringstream ss;
                ss << "thread " << thread_id
                    << ", it " << iterations
                    << " (" << iterations_without_improvment << ")";
                output.update_solution(solution, ss, parameters.info);
            }

            // Update statistics
            iterations_without_improvment = 0;

            // Find the best shift move.
            SetId    s_best = -1;
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
            // Apply best move
            Cost tmp = solution.penalty();
            solution.remove(s_best);
            assert(solution.penalty() == tmp + score_best);
            // Update scores.
            for (ElementId e: instance.set(s_best).elements) {
                if (solution.covers(e) == 0) {
                    for (SetId s: instance.element(e).sets)
                        if (s != s_best)
                            sets[s].score += solution.penalty(e);
                } else if (solution.covers(e) == 1) {
                    for (SetId s: instance.element(e).sets)
                        if (solution.contains(s))
                            sets[s].score += solution.penalty(e);
                }
            }
            // Update sets
            sets[s_best].timestamp = iterations;
            sets[s_best].iterations += (iterations - sets[s_best].last_addition);
            sets[s_best].last_removal = iterations;
            // Update tabu
            s_last_removed = -1;
            s_last_added   = -1;
            //std::cout << "it " << output.iterations
                //<< " s_best " << s_best
                //<< " p " << solution.penalty()
                //<< " e " << instance.element_number() - solution.element_number() << "/" << instance.element_number()
                //<< " s " << solution.set_number()
                //<< std::endl;
        }

        // Find the cheapest set to remove.
        SetId    s1_best = -1;
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
        Cost tmp1 = solution.penalty();
        solution.remove(s1_best);
        assert(solution.penalty() == tmp1 + score1_best);
        // Update scores.
        for (ElementId e: instance.set(s1_best).elements) {
            if (solution.covers(e) == 0) {
                for (SetId s: instance.element(e).sets)
                    if (s != s1_best)
                        sets[s].score += solution.penalty(e);
            } else if (solution.covers(e) == 1) {
                for (SetId s: instance.element(e).sets)
                    if (solution.contains(s))
                        sets[s].score += solution.penalty(e);
            }
        }
        // Update sets
        sets[s1_best].timestamp = iterations;
        sets[s1_best].last_removal = iterations;
        sets[s1_best].iterations += (iterations - sets[s1_best].last_addition);
        // Update tabu
        s_last_removed = s1_best;
        //std::cout << "it " << iterations
            //<< " s1_best " << s1_best
            //<< " score " << score1_best
            //<< " p " << solution.penalty()
            //<< " e " << instance.element_number() - solution.element_number() << "/" << instance.element_number()
            //<< " s " << solution.set_number()
            //<< std::endl;

        // Draw randomly an uncovered element e.
        std::uniform_int_distribution<ElementId> d_e(
                0, solution.uncovered_element_number() - 1);
        ElementId e = (solution.elements().out_begin() + d_e(generator))->first;
        //std::cout << "it " << output.iterations
            //<< " e " << e
            //<< " s " << instance.element(e).sets.size()
            //<< std::endl;

        // Find the best set to add.
        SetId    s2_best = -1;
        Cost score2_best = -1;
        for (SetId s: instance.element(e).sets) {
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
        Cost tmp2 = solution.penalty();
        solution.add(s2_best);
        assert(solution.penalty() == tmp2 - score2_best);
        // Update scores.
        for (ElementId e: instance.set(s2_best).elements) {
            if (solution.covers(e) == 1) {
                for (SetId s: instance.element(e).sets)
                    if (!solution.contains(s))
                        sets[s].score -= solution.penalty(e);
            } else if (solution.covers(e) == 2) {
                for (SetId s: instance.element(e).sets)
                    if (s != s2_best && solution.contains(s))
                        sets[s].score -= solution.penalty(e);
            }
        }
        // Update sets
        sets[s2_best].timestamp = iterations;
        sets[s2_best].last_addition = iterations;
        // Update tabu
        s_last_added = s2_best;
        //std::cout << "it " << iterations
            //<< " s2_best " << s2_best
            //<< " score " << score2_best
            //<< " p " << solution.penalty()
            //<< " e " << instance.element_number() - solution.element_number() << "/" << instance.element_number()
            //<< " s " << solution.set_number()
            //<< std::endl;

        // Update penalties: we increment the penalty of each uncovered element.
        for (auto it = solution.elements().out_begin(); it != solution.elements().out_end(); ++it) {
            solution.increment_penalty(it->first);
            for (SetId s: instance.element(it->first).sets)
                sets[s].score++;
        }
    }
}

LocalSearch2Output setcoveringsolver::localsearch_2(
        Instance& instance,
        std::mt19937_64& generator,
        LocalSearch2OptionalParameters parameters)
{
    VER(parameters.info, "*** localsearch_2 ***" << std::endl);

    // Instance pre-processing.
    instance.fix_identical(parameters.info);
    instance.compute_set_neighbors(parameters.thread_number, parameters.info);
    instance.compute_components(parameters.info);

    // Compute initial greedy solution.
    LocalSearch2Output output(instance, parameters.info);
    Solution solution = greedy(instance).solution;
    std::stringstream ss;
    ss << "initial solution";
    output.update_solution(solution, ss, parameters.info);

    auto seeds = optimizationtools::bob_floyd(parameters.thread_number, std::numeric_limits<Seed>::max(), generator);
    std::vector<std::thread> threads;
    for (Counter thread_id = 0; thread_id < parameters.thread_number; ++thread_id)
        threads.push_back(std::thread(localsearch_2_worker, std::ref(instance), seeds[thread_id], parameters, std::ref(output), thread_id));

    for (Counter thread_id = 0; thread_id < parameters.thread_number; ++thread_id)
        threads[thread_id].join();

    return output.algorithm_end(parameters.info);
}

