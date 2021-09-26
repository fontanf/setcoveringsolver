#include "setcoveringsolver/algorithms/largeneighborhoodsearch.hpp"

#include "setcoveringsolver/algorithms/greedy.hpp"

#include "optimizationtools/indexed_set.hpp"
#include "optimizationtools/indexed_binary_heap.hpp"

using namespace setcoveringsolver;

LargeNeighborhoodSearchOutput& LargeNeighborhoodSearchOutput::algorithm_end(
        optimizationtools::Info& info)
{
    PUT(info, "Algorithm", "Iterations", iterations);
    Output::algorithm_end(info);
    VER(info, "Iterations: " << iterations << std::endl);
    return *this;
}

LargeNeighborhoodSearchOutput setcoveringsolver::largeneighborhoodsearch(
        Instance& instance,
        std::mt19937_64& generator,
        LargeNeighborhoodSearchOptionalParameters parameters)
{
    VER(parameters.info, "*** largeneighborhoodsearch ***" << std::endl);

    instance.fix_identical(parameters.info);

    LargeNeighborhoodSearchOutput output(instance, parameters.info);
    Solution solution = greedy(instance).solution;
    std::stringstream ss;
    ss << "initial solution";
    output.update_solution(solution, ss, parameters.info);

    std::vector<Counter> last_iteration(instance.number_of_sets(), 0);
    std::uniform_int_distribution<ElementId> distribution_elements(0, instance.number_of_elements() - 1);
    std::uniform_real_distribution<double> d01(0, 1);
    std::vector<SetId> sets_added;
    std::vector<SetId> sets_removed;
    optimizationtools::IndexedSet sets_candidates(instance.number_of_sets());
    optimizationtools::IndexedBinaryHeap<double> heap(instance.number_of_sets());
    Counter iterations_without_improvment = 0;
    for (output.iterations = 1; !parameters.info.needs_to_end(); ++output.iterations, ++iterations_without_improvment) {
        // Check stop criteria.
        if (parameters.iteration_limit != -1
                && output.iterations > parameters.iteration_limit)
            break;
        if (parameters.iteration_without_improvment_limit != -1
                && iterations_without_improvment > parameters.iteration_without_improvment_limit)
            break;

        sets_added.clear();
        sets_removed.clear();
        sets_candidates.clear();

        // Remove all sets covering element e.
        ElementId e = distribution_elements(generator);
        for (SetId s: instance.element(e).sets) {
            if (solution.contains(s)) {
                solution.remove(s);
                sets_removed.push_back(s);
            }
        }

        // Update heap
        for (SetId s: instance.element(e).sets)
            sets_candidates.add(s);
        auto f = [&solution, &last_iteration, &output](SetId s)
        {
            double val = 0;
            for (ElementId e: solution.instance().set(s).elements)
                if (!solution.covers(e))
                    val++;
            val += (double)(output.iterations - last_iteration[s]) / output.iterations;
            val /= solution.instance().set(s).cost;
            return - val;
        };
        heap.reset(sets_candidates, f);

        // Fill solution
        while (!solution.feasible() && !heap.empty()) {
            auto p = heap.top();
            double val = 0;
            for (ElementId e: instance.set(p.first).elements)
                if (!solution.covers(e))
                    val++;
            val += (double)(output.iterations - last_iteration[p.first]) / output.iterations;
            val /= instance.set(p.first).cost;
            val = - val;
            if (val <= p.second + TOL) {
                if (d01(generator) < 0.9) {
                    solution.add(p.first);
                    sets_added.push_back(p.first);
                    last_iteration[p.first] = output.iterations;
                }
                heap.pop();
            } else {
                heap.update_key(p.first, val);
            }
        }
        if (!solution.feasible() || output.solution.cost() < solution.cost()) {
            for (SetId s: sets_added)
                solution.remove(s);
            for (SetId s: sets_removed)
                solution.add(s);
        } else if (output.solution.cost() > solution.cost()){
            std::stringstream ss;
            ss << "iteration " << output.iterations;
            output.update_solution(solution, ss, parameters.info);
            iterations_without_improvment = 0;
        }
    }

    return output.algorithm_end(parameters.info);
}

/******************************************************************************/

LargeNeighborhoodSearch2Output& LargeNeighborhoodSearch2Output::algorithm_end(
        optimizationtools::Info& info)
{
    PUT(info, "Algorithm", "Iterations", iterations);
    Output::algorithm_end(info);
    VER(info, "Iterations: " << iterations << std::endl);
    return *this;
}

struct LargeNeighborhoodSearch2Set
{
    Counter timestamp     = -1;
    Counter last_addition = -1;
    Counter last_removal  = -1;
    Counter iterations    = 0;
    Cost    score         = 0;
};

LargeNeighborhoodSearch2Output setcoveringsolver::largeneighborhoodsearch_2(
        Instance& instance,
        LargeNeighborhoodSearch2OptionalParameters parameters)
{
    VER(parameters.info, "*** largeneighborhoodsearch_2 ***" << std::endl);

    instance.fix_identical(parameters.info);
    //instance.fix_dominated(parameters.info);

    LargeNeighborhoodSearch2Output output(instance, parameters.info);
    Solution solution = greedy(instance).solution;
    std::stringstream ss;
    ss << "initial solution";
    output.update_solution(solution, ss, parameters.info);

    // Initialize local search structures.
    std::vector<LargeNeighborhoodSearch2Set> sets(instance.number_of_sets());
    std::vector<Penalty> solution_penalties(instance.number_of_elements(), 1);
    for (SetId s: solution.sets())
        for (ElementId e: instance.set(s).elements)
            if (solution.covers(e) == 1)
                sets[s].score += solution_penalties[e];
    optimizationtools::IndexedBinaryHeap<std::pair<double, Counter>> scores_in(instance.number_of_sets());
    optimizationtools::IndexedBinaryHeap<std::pair<double, Counter>> scores_out(instance.number_of_sets());
    for (SetId s: solution.sets())
        scores_in.update_key(s, {(double)sets[s].score / instance.set(s).cost, 0});

    optimizationtools::IndexedSet sets_in_to_update(instance.number_of_sets());
    optimizationtools::IndexedSet sets_out_to_update(instance.number_of_sets());
    Counter iterations_without_improvment = 0;
    for (output.iterations = 1; !parameters.info.needs_to_end(); ++output.iterations, ++iterations_without_improvment) {
        // Check stop criteria.
        if (parameters.iteration_limit != -1
                && output.iterations > parameters.iteration_limit)
            break;
        if (parameters.iteration_without_improvment_limit != -1
                && iterations_without_improvment > parameters.iteration_without_improvment_limit)
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
            SetId s = p.first;
            //std::cout << "remove " << s << " score " << p.second << " cost " << instance.set(s).cost << " e " << solution.number_of_elements() << std::endl;
            solution.remove(s);
            sets[s].last_removal = output.iterations;
            sets_out_to_update.add(s);
            // Update scores.
            sets_in_to_update.clear();
            for (ElementId e: instance.set(s).elements) {
                if (solution.covers(e) == 0) {
                    for (SetId s2: instance.element(e).sets) {
                        if (s2 == s)
                            continue;
                        sets[s2].score += solution_penalties[e];
                        sets_out_to_update.add(s2);
                    }
                } else if (solution.covers(e) == 1) {
                    for (SetId s2: instance.element(e).sets) {
                        if (!solution.contains(s2))
                            continue;
                        sets[s2].score += solution_penalties[e];
                        sets_in_to_update.add(s2);
                    }
                }
            }
            for (SetId s2: sets_in_to_update)
                scores_in.update_key(s2, {(double)sets[s2].score / instance.set(s2).cost, sets[s2].last_addition});
        }
        for (SetId s2: sets_out_to_update)
            scores_out.update_key(s2, {- (double)sets[s2].score / instance.set(s2).cost, sets[s2].last_removal});

        // Update penalties: we increment the penalty of each uncovered element.
        sets_out_to_update.clear();
        for (auto it = solution.elements().out_begin(); it != solution.elements().out_end(); ++it) {
            solution_penalties[it->first]++;
            for (SetId s: instance.element(it->first).sets) {
                sets[s].score++;
                sets_out_to_update.add(s);
            }
        }
        for (SetId s: sets_out_to_update)
            scores_out.update_key(s, {- (double)sets[s].score / instance.set(s).cost, sets[s].last_removal});

        // Add sets.
        sets_in_to_update.clear();
        while (!solution.feasible() && !scores_out.empty()) {
            auto p = scores_out.top();
            scores_out.pop();
            SetId s = p.first;
            solution.add(s);
            //std::cout << "add " << s << " score " << p.second << " cost " << instance.set(s).cost << " e " << solution.number_of_elements() << std::endl;
            assert(p.second.first < 0);
            sets[s].last_addition = output.iterations;
            sets_in_to_update.add(s);
            // Update scores.
            sets_out_to_update.clear();
            for (ElementId e: instance.set(s).elements) {
                if (solution.covers(e) == 1) {
                    for (SetId s2: instance.element(e).sets) {
                        if (solution.contains(s2))
                            continue;
                        sets[s2].score -= solution_penalties[e];
                        sets_out_to_update.add(s2);
                    }
                } else if (solution.covers(e) == 2) {
                    for (SetId s2: instance.element(e).sets) {
                        if (s2 == s || !solution.contains(s2))
                            continue;
                        sets[s2].score -= solution_penalties[e];
                        sets_in_to_update.add(s2);
                    }
                }
            }

            // Remove redundant sets.
            for (ElementId e: instance.set(s).elements) {
                for (SetId s2: instance.element(e).sets) {
                    if (solution.contains(s2) && sets[s2].score == 0) {
                        solution.remove(s2);
                        sets[s2].last_removal = output.iterations;
                        //std::cout << "> remove " << s2 << " score " << sets[s2].score << " cost " << instance.set(s2).cost << " e " << solution.number_of_elements() << " / " << instance.number_of_elements() << std::endl;
                        for (ElementId e2: instance.set(s2).elements) {
                            if (solution.covers(e2) == 1) {
                                for (SetId s3: instance.element(e2).sets) {
                                    if (!solution.contains(s3))
                                        continue;
                                    sets[s3].score += solution_penalties[e2];
                                    sets_in_to_update.add(s3);
                                }
                            }
                        }
                    }
                }
            }

            for (SetId s2: sets_out_to_update)
                scores_out.update_key(s2, {- (double)sets[s2].score / instance.set(s2).cost, sets[s2].last_removal});
        }
        for (SetId s2: sets_in_to_update) {
            if (solution.contains(s2)) {
                scores_in.update_key(s2, {(double)sets[s2].score / instance.set(s2).cost, sets[s2].last_addition});
            } else {
                scores_in.update_key(s2, {-1, -1});
                scores_in.pop();
            }
        }

        // Update best solution.
        //std::cout << "cost " << solution.cost() << std::endl;
        if (output.solution.cost() > solution.cost()){
            std::stringstream ss;
            ss << "iteration " << output.iterations;
            output.update_solution(solution, ss, parameters.info);
            iterations_without_improvment = 0;
        }
    }

    return output.algorithm_end(parameters.info);
}

