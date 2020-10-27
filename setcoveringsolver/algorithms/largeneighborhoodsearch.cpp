#include "setcoveringsolver/algorithms/largeneighborhoodsearch.hpp"

#include "setcoveringsolver/algorithms/greedy.hpp"

#include "optimizationtools/indexed_set.hpp"
#include "optimizationtools/indexed_binary_heap.hpp"

using namespace setcoveringsolver;

LargeNeighborhoodSearchOutput& LargeNeighborhoodSearchOutput::algorithm_end(Info& info)
{
    PUT(info, "Algorithm", "Iterations", iterations);
    Output::algorithm_end(info);
    VER(info, "Iterations: " << iterations << std::endl);
    return *this;
}

LargeNeighborhoodSearchOutput setcoveringsolver::largeneighborhoodsearch(
        const Instance& instance,
        std::mt19937_64& generator,
        LargeNeighborhoodSearchOptionalParameters parameters)
{
    VER(parameters.info, "*** largeneighborhoodsearch ***" << std::endl);

    LargeNeighborhoodSearchOutput output(instance, parameters.info);
    Solution solution = greedy(instance).solution;
    std::stringstream ss;
    ss << "initial solution";
    output.update_solution(solution, ss, parameters.info);

    std::vector<Counter> last_iteration(instance.set_number(), 0);
    std::uniform_int_distribution<ElementId> distribution_elements(0, instance.element_number() - 1);
    std::uniform_real_distribution<double> d01(0, 1);
    std::vector<SetId> sets_added;
    std::vector<SetId> sets_removed;
    optimizationtools::IndexedSet sets_candidates(instance.set_number());
    optimizationtools::IndexedBinaryHeap<double> heap(instance.set_number());
    Counter iterations_without_improvment = 0;
    for (output.iterations = 1; parameters.info.check_time(); ++output.iterations, ++iterations_without_improvment) {
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

