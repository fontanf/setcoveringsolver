#include "setcoveringsolver/algorithms/local_search_row_weighting.hpp"

#include "setcoveringsolver/algorithm_formatter.hpp"
#include "setcoveringsolver/algorithms/greedy.hpp"
#include "setcoveringsolver/algorithms/trivial_bound.hpp"

#include "optimizationtools/containers/indexed_set.hpp"
#include "optimizationtools/containers/doubly_indexed_map.hpp"
#include "optimizationtools/containers/indexed_4ary_heap.hpp"

using namespace setcoveringsolver;

namespace
{

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

    optimizationtools::Indexed4aryHeap<std::pair<Penalty, double>> scores_in;

    Penalty penalty = 0;
};

struct LocalSearchRowWeightingSet
{
    /**
     * The set is the only one covering some element: it is always in the
     * solution and never in 'scores_in', so it is never removed.
     */
    bool mandatory = false;

    Counter timestamp = -1;
    Counter last_addition = -1;
    Counter last_removal = -1;
    Counter iterations = 0;
    Cost score = 0;
};

void remove_set(
        const Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeightingOutput& output,
        const std::vector<SetPos>& sets_id_to_component_pos,
        ComponentId component_id,
        Solution& solution,
        optimizationtools::DoublyIndexedMap& component_uncovered_elements,
        Penalty& solution_penalty,
        std::vector<Penalty>& solution_penalties,
        std::vector<LocalSearchRowWeightingSet>& sets,
        std::vector<LocalSearchRowWeightingComponent>& components,
        optimizationtools::IndexedSet& scores_in_to_update,
        std::uniform_real_distribution<double>& d_score)
{
    LocalSearchRowWeightingComponent& component = components[component_id];

    // Find the best shift move.
    SetPos set_pos = component.scores_in.top().first;
    SetId set_id_best = instance.component(component_id).sets[set_pos];
    Cost p_best = -sets[set_id_best].score;
    const Set& set_best = instance.set(set_id_best);

    // Apply best move
    solution.remove(set_id_best);

    // Update scores.
    components[set_best.component].scores_in.update_key(
            sets_id_to_component_pos[set_id_best], {-1, -1});
    components[set_best.component].scores_in.pop();
    scores_in_to_update.clear();
    for (ElementId element_id: set_best.elements) {
        if (solution.covers(element_id) == 0) {
            component_uncovered_elements.set(element_id, component_id);
            solution_penalty += solution_penalties[element_id];
            component.penalty += solution_penalties[element_id];
            for (SetId set_id: instance.element(element_id).sets)
                if (set_id != set_id_best)
                    sets[set_id].score += solution_penalties[element_id];
        } else if (solution.covers(element_id) == 1) {
            for (SetId set_id: instance.element(element_id).sets) {
                if (solution.contains(set_id)) {
                    sets[set_id].score += solution_penalties[element_id];
                    scores_in_to_update.add(set_id);
                }
            }
        }
    }
    for (SetId set_id: scores_in_to_update) {
        if (!solution.contains(set_id)) {
            throw std::logic_error(
                    "setcoveringsolver::local_search_row_weighting: "
                    "wrong set in scores_in; "
                    "set_id: " + std::to_string(set_id) + ".");
        }
        if (sets[set_id].mandatory)
            continue;
        const Set& set = instance.set(set_id);
        components[set.component].scores_in.update_key(
                sets_id_to_component_pos[set_id],
                {sets[set_id].score, d_score(generator)});
    }

    // Update sets
    sets[set_id_best].timestamp = output.number_of_iterations;
    sets[set_id_best].iterations += (component.iterations - sets[set_id_best].last_addition);
    sets[set_id_best].last_removal = component.iterations;
    // Update tabu
    component.set_id_last_removed = set_id_best;

    // Update penalties.
    for (ElementId element_id: instance.set(set_id_best).elements) {
        if (solution.covers(element_id) == 0) {
            for (SetId set_id: instance.element(element_id).sets) {
                solution_penalty -= solution_penalties[element_id];
                component.penalty -= solution_penalties[element_id];
                sets[set_id].score -= solution_penalties[element_id];
            }

            solution_penalties[element_id] += (std::max)(
                    (Penalty)1,
                    (Penalty)(1e4 / instance.element(element_id).sets.size()));

            for (SetId set_id: instance.element(element_id).sets) {
                solution_penalty += solution_penalties[element_id];
                component.penalty += solution_penalties[element_id];
                sets[set_id].score += solution_penalties[element_id];
            }
        }
    }
}

void explore_remove_add_neighborhood(
        const Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeightingOutput& output,
        const std::vector<SetPos>& sets_id_to_component_pos,
        ComponentId component_id,
        Solution& solution,
        optimizationtools::DoublyIndexedMap& component_uncovered_elements,
        Penalty& solution_penalty,
        std::vector<Penalty>& solution_penalties,
        std::vector<LocalSearchRowWeightingSet>& sets,
        std::vector<LocalSearchRowWeightingComponent>& components,
        optimizationtools::IndexedSet& scores_in_to_update,
        std::uniform_real_distribution<double>& d_score)
{
    LocalSearchRowWeightingComponent& component = components[component_id];

    // Find the cheapest set to remove.
    SetId set_id_1_best = -1;
    Cost p_best = 0;
    for (SetPos pos = 0; pos < 7; ++pos) {
        if (pos >= component.scores_in.size())
            break;
        SetPos set_pos = component.scores_in.top(pos).first;
        SetId set_id_1 = instance.component(component_id).sets[set_pos];
        Penalty p = sets[set_id_1].score;

        // Check tabu.
        if (component.penalty + p > 0
                && set_id_1 == component.set_id_last_added) {
            continue;
        }

        // Update best move.
        if (set_id_1_best == -1
                || p_best > p
                || (p_best == p
                    && sets[set_id_1_best].timestamp > sets[set_id_1].timestamp)) {
            set_id_1_best = set_id_1;
            p_best = p;
        }
    }
    const Set& set_1_best = instance.set(set_id_1_best);

    // Apply best move
    solution.remove(set_id_1_best);

    // Update scores.
    components[set_1_best.component].scores_in.update_key(
            sets_id_to_component_pos[set_id_1_best], {-1, -1});
    components[set_1_best.component].scores_in.pop();
    scores_in_to_update.clear();
    for (ElementId element_id: set_1_best.elements) {
        if (solution.covers(element_id) == 0) {
            component_uncovered_elements.set(element_id, component_id);
            solution_penalty += solution_penalties[element_id];
            component.penalty += solution_penalties[element_id];
            for (SetId set_id: instance.element(element_id).sets)
                if (set_id != set_id_1_best)
                    sets[set_id].score += solution_penalties[element_id];
        } else if (solution.covers(element_id) == 1) {
            for (SetId set_id: instance.element(element_id).sets) {
                if (solution.contains(set_id)) {
                    sets[set_id].score += solution_penalties[element_id];
                    scores_in_to_update.add(set_id);
                }
            }
        }
    }
    for (SetId set_id: scores_in_to_update) {
        if (!solution.contains(set_id)) {
            throw std::logic_error(
                    "setcoveringsolver::local_search_row_weighting: "
                    "wrong set in scores_in; "
                    "set_id: " + std::to_string(set_id) + ".");
        }
        if (sets[set_id].mandatory)
            continue;
        const Set& set = instance.set(set_id);
        components[set.component].scores_in.update_key(
                sets_id_to_component_pos[set_id],
                {sets[set_id].score, d_score(generator)});
    }

    // Update sets
    sets[set_id_1_best].timestamp = output.number_of_iterations;
    sets[set_id_1_best].iterations += (component.iterations - sets[set_id_1_best].last_addition);
    sets[set_id_1_best].last_removal = component.iterations;
    // Update tabu
    component.set_id_last_removed = set_id_1_best;

    // Update penalties.
    for (ElementId element_id: instance.set(set_id_1_best).elements) {
        if (solution.covers(element_id) == 0) {
            for (SetId set_id: instance.element(element_id).sets) {
                solution_penalty -= solution_penalties[element_id];
                component.penalty -= solution_penalties[element_id];
                sets[set_id].score -= solution_penalties[element_id];
            }

            solution_penalties[element_id] += (std::max)(
                    (Penalty)1,
                    (Penalty)(1e4 / instance.element(element_id).sets.size()));

            for (SetId set_id: instance.element(element_id).sets) {
                solution_penalty += solution_penalties[element_id];
                component.penalty += solution_penalties[element_id];
                sets[set_id].score += solution_penalties[element_id];
            }
        }
    }


    // Draw randomly an uncovered element e.
    std::uniform_int_distribution<ElementId> d_e(
            0, component_uncovered_elements.number_of_elements(component_id) - 1);
    ElementId element_id = *(component_uncovered_elements.begin(component_id) + d_e(generator));
    //std::cout << "it " << output.iterations
    //<< " e " << e
    //<< " s " << instance.element(element_id).sets.size()
    //<< std::endl;


    // Find the best set to add.
    SetId set_id_2_best = -1;
    p_best = 0;
    for (SetId set_id_2: instance.element(element_id).sets) {
        if (set_id_2 == component.set_id_last_removed)
            continue;
        Penalty p = -sets[set_id_2].score;
        const Set& set_2 = instance.set(set_id_2);

        // Update best move.
        if (set_id_2_best == -1
                || p_best > p
                || (p_best == p
                    && sets[set_id_2_best].timestamp > sets[set_id_2].timestamp)) {
            set_id_2_best = set_id_2;
            p_best = p;
        }
    }

    // Apply best swap.
    if (set_id_2_best != -1) {
        const Set& set_2_best = instance.set(set_id_2_best);
        if (set_2_best.component != component_id) {
            throw std::runtime_error(
                    "setcoveringsolver::local_search_row_weighting: "
                    "wrong component; "
                    "set_id_2_best: " + std::to_string(set_id_2_best) + "; "
                    "component_id: " + std::to_string(component_id) + "; "
                    "set_2_best.component: " + std::to_string(set_2_best.component) + ".");
        }
        //std::cout << "set_id_2_best " << set_id_2_best
        //    << " set_id_2_best " << set_id_2_best
        //    << " solution_penalty " << solution_penalty
        //    << " ue " << solution.number_of_uncovered_elements()
        //    << " p_best " << p_best
        //    << std::endl;
        Penalty solution_penalty_old = solution_penalty;

        // Add set.
        solution.add(set_id_2_best);

        // Update scores.
        scores_in_to_update.clear();
        scores_in_to_update.add(set_id_2_best);
        for (ElementId element_id: instance.set(set_id_2_best).elements) {
            if (solution.covers(element_id) == 1) {
                solution_penalty -= solution_penalties[element_id];
                component.penalty -= solution_penalties[element_id];
                component_uncovered_elements.set(element_id, instance.number_of_components());
                for (SetId set_id: instance.element(element_id).sets)
                    if (!solution.contains(set_id))
                        sets[set_id].score -= solution_penalties[element_id];
            } else if (solution.covers(element_id) == 2) {
                for (SetId set_id: instance.element(element_id).sets) {
                    if (set_id != set_id_2_best && solution.contains(set_id)) {
                        sets[set_id].score -= solution_penalties[element_id];
                        scores_in_to_update.add(set_id);
                    }
                }
            }
        }
        for (SetId set_id: scores_in_to_update) {
            if (!solution.contains(set_id)) {
                throw std::logic_error(
                        "setcoveringsolver::local_search_row_weighting: "
                        "wrong set in scores_in 2; "
                        "set_id: " + std::to_string(set_id) + ".");
            }
            if (sets[set_id].mandatory)
                continue;
            const Set& set = instance.set(set_id);
            components[set.component].scores_in.update_key(
                    sets_id_to_component_pos[set_id],
                    {sets[set_id].score, d_score(generator)});
        }
        if (solution_penalty_old + p_best != solution_penalty) {
            throw std::runtime_error(
                    "setcoveringsolver::local_search_row_weighting: "
                    "wrong penalty; "
                    "set_id_2_best: " + std::to_string(set_id_2_best) + "; "
                    "solution_penalty_old: " + std::to_string(solution_penalty_old) + "; "
                    "p_best: " + std::to_string(p_best) + "; "
                    "solution_penalty: " + std::to_string(solution_penalty) + "."
                    );
        }

        // Update sets
        sets[set_id_2_best].timestamp = output.number_of_iterations;
        sets[set_id_2_best].last_addition = component.iterations;

        // Update penalties.
        for (ElementId element_id: instance.set(set_id_2_best).elements) {
            if (solution.covers(element_id) == 0) {
                for (SetId set_id: instance.element(element_id).sets) {
                    solution_penalty -= solution_penalties[element_id];
                    component.penalty -= solution_penalties[element_id];
                    sets[set_id].score -= solution_penalties[element_id];
                }

                solution_penalties[element_id] += (std::max)(
                        (Penalty)1,
                        (Penalty)(1e4 / instance.element(element_id).sets.size()));

                for (SetId set_id: instance.element(element_id).sets) {
                    solution_penalty += solution_penalties[element_id];
                    component.penalty += solution_penalties[element_id];
                    sets[set_id].score += solution_penalties[element_id];
                }
            }
        }
    }
}

void explore_swap_neighborhood(
        const Instance& instance,
        std::mt19937_64& generator,
        LocalSearchRowWeightingOutput& output,
        const std::vector<SetPos>& sets_id_to_component_pos,
        ComponentId component_id,
        Solution& solution,
        optimizationtools::DoublyIndexedMap& component_uncovered_elements,
        Penalty& solution_penalty,
        std::vector<Penalty>& solution_penalties,
        std::vector<LocalSearchRowWeightingSet>& sets,
        std::vector<LocalSearchRowWeightingComponent>& components,
        optimizationtools::IndexedSet& scores_in_to_update,
        std::uniform_real_distribution<double>& d_score)
{
    LocalSearchRowWeightingComponent& component = components[component_id];

    // Draw randomly an uncovered element e.
    std::uniform_int_distribution<ElementId> d_e(
            0, component_uncovered_elements.number_of_elements(component_id) - 1);
    ElementId element_id = *(component_uncovered_elements.begin(component_id) + d_e(generator));
    //std::cout << "it " << output.iterations
    //<< " e " << e
    //<< " s " << instance.element(element_id).sets.size()
    //<< std::endl;

    // Find the best swap move.
    SetId set_id_1_best = -1;
    SetId set_id_2_best = -1;
    Cost p_best = 0;

    // For each set set_id_1 covering element e which is not part of the solution
    // and which is not the last set removed.
    for (SetId set_id_1: instance.element(element_id).sets) {
        if (set_id_1 == component.set_id_last_removed)
            continue;
        Penalty p0 = -sets[set_id_1].score;
        if (set_id_1_best != -1 && p0 > p_best)
            continue;
        const Set& set_1 = instance.set(set_id_1);

        solution.add(set_id_1);

        // Update scores.
        scores_in_to_update.clear();
        scores_in_to_update.add(set_id_1);
        for (ElementId element_id: set_1.elements) {
            if (solution.covers(element_id) == 1) {
                for (SetId set_id: instance.element(element_id).sets)
                    if (!solution.contains(set_id))
                        sets[set_id].score -= solution_penalties[element_id];
            } else if (solution.covers(element_id) == 2) {
                for (SetId set_id: instance.element(element_id).sets) {
                    if (set_id != set_id_1 && solution.contains(set_id)) {
                        sets[set_id].score -= solution_penalties[element_id];
                        scores_in_to_update.add(set_id);
                    }
                }
            }
        }
        for (SetId set_id: scores_in_to_update) {
            if (!solution.contains(set_id)) {
                throw std::logic_error(
                        "setcoveringsolver::local_search_row_weighting: "
                        "wrong set in scores_in 2; "
                        "set_id: " + std::to_string(set_id) + ".");
            }
            if (sets[set_id].mandatory)
                continue;
            const Set& set = instance.set(set_id);
            components[set.component].scores_in.update_key(
                    sets_id_to_component_pos[set_id],
                    {sets[set_id].score, d_score(generator)});
        }

        for (SetPos pos = 0; pos < 7; ++pos) {
            if (pos >= component.scores_in.size())
                break;
            SetPos set_pos = component.scores_in.top(pos).first;
            SetId set_id_2 = instance.component(component_id).sets[set_pos];
            if (set_id_2 == set_id_1)
                continue;
            Penalty p = p0 + sets[set_id_2].score;

            // Check tabu.
            if (component.penalty + p > 0
                    && set_id_2 == component.set_id_last_added) {
                continue;
            }

            // Update best move.
            if (set_id_1_best == -1
                    || p_best > p
                    || (p_best == p
                        && sets[set_id_1_best].timestamp + sets[set_id_2_best].timestamp
                        > sets[set_id_1].timestamp + sets[set_id_2].timestamp)) {
                set_id_1_best = set_id_1;
                set_id_2_best = set_id_2;
                p_best = p;
            }
        }

        solution.remove(set_id_1);

        // Update scores.
        scores_in_to_update.clear();
        components[set_1.component].scores_in.update_key(
                sets_id_to_component_pos[set_id_1], {-1, -1});
        components[set_1.component].scores_in.pop();
        scores_in_to_update.remove(set_id_1);
        for (ElementId element_id: set_1.elements) {
            if (solution.covers(element_id) == 0) {
                for (SetId set_id: instance.element(element_id).sets)
                    if (set_id != set_id_1)
                        sets[set_id].score += solution_penalties[element_id];
            } else if (solution.covers(element_id) == 1) {
                for (SetId set_id: instance.element(element_id).sets) {
                    if (solution.contains(set_id)) {
                        sets[set_id].score += solution_penalties[element_id];
                        scores_in_to_update.add(set_id);
                    }
                }
            }
        }
        for (SetId set_id: scores_in_to_update) {
            if (!solution.contains(set_id)) {
                throw std::logic_error(
                        "setcoveringsolver::local_search_row_weighting: "
                        "wrong set in scores_in 2; "
                        "set_id: " + std::to_string(set_id) + ".");
            }
            if (sets[set_id].mandatory)
                continue;
            const Set& set = instance.set(set_id);
            components[set.component].scores_in.update_key(
                    sets_id_to_component_pos[set_id],
                    {sets[set_id].score, d_score(generator)});
        }
    }

    // Apply best swap.
    if (set_id_1_best != -1) {
        const Set& set_1_best = instance.set(set_id_1_best);
        const Set& set_2_best = instance.set(set_id_2_best);
        if (set_1_best.component != component_id) {
            throw std::runtime_error(
                    "setcoveringsolver::local_search_row_weighting: "
                    "wrong component; "
                    "set_id_1_best: " + std::to_string(set_id_1_best) + "; "
                    "component_id: " + std::to_string(component_id) + "; "
                    "set_1_best.component: " + std::to_string(set_1_best.component) + ".");
        }
        if (set_2_best.component != component_id) {
            throw std::runtime_error(
                    "setcoveringsolver::local_search_row_weighting: "
                    "wrong component; "
                    "set_id_2_best: " + std::to_string(set_id_2_best) + "; "
                    "component_id: " + std::to_string(component_id) + "; "
                    "set_2_best.component: " + std::to_string(set_2_best.component) + ".");
        }
        //std::cout << "set_id_1_best " << set_id_1_best
        //    << " set_id_2_best " << set_id_2_best
        //    << " solution_penalty " << solution_penalty
        //    << " ue " << solution.number_of_uncovered_elements()
        //    << " p_best " << p_best
        //    << std::endl;
        Penalty solution_penalty_old = solution_penalty;

        // Add set.
        solution.add(set_id_1_best);

        // Update scores.
        scores_in_to_update.clear();
        scores_in_to_update.add(set_id_1_best);
        for (ElementId element_id: instance.set(set_id_1_best).elements) {
            if (solution.covers(element_id) == 1) {
                solution_penalty -= solution_penalties[element_id];
                component.penalty -= solution_penalties[element_id];
                component_uncovered_elements.set(element_id, instance.number_of_components());
                for (SetId set_id: instance.element(element_id).sets)
                    if (!solution.contains(set_id))
                        sets[set_id].score -= solution_penalties[element_id];
            } else if (solution.covers(element_id) == 2) {
                for (SetId set_id: instance.element(element_id).sets) {
                    if (set_id != set_id_1_best && solution.contains(set_id)) {
                        sets[set_id].score -= solution_penalties[element_id];
                        scores_in_to_update.add(set_id);
                    }
                }
            }
        }

        // Remove set.
        solution.remove(set_id_2_best);

        // Update scores.
        components[set_2_best.component].scores_in.update_key(
                sets_id_to_component_pos[set_id_2_best], {-1, -1});
        components[set_2_best.component].scores_in.pop();
        scores_in_to_update.remove(set_id_2_best);
        for (ElementId element_id: instance.set(set_id_2_best).elements) {
            if (solution.covers(element_id) == 0) {
                solution_penalty += solution_penalties[element_id];
                component.penalty += solution_penalties[element_id];
                component_uncovered_elements.set(element_id, component_id);
                for (SetId set_id: instance.element(element_id).sets)
                    if (set_id != set_id_2_best)
                        sets[set_id].score += solution_penalties[element_id];
            } else if (solution.covers(element_id) == 1) {
                for (SetId set_id: instance.element(element_id).sets) {
                    if (solution.contains(set_id)) {
                        sets[set_id].score += solution_penalties[element_id];
                        scores_in_to_update.add(set_id);
                    }
                }
            }
        }
        for (SetId set_id: scores_in_to_update) {
            if (!solution.contains(set_id)) {
                throw std::logic_error(
                        "setcoveringsolver::local_search_row_weighting: "
                        "wrong set in scores_in 2; "
                        "set_id: " + std::to_string(set_id) + ".");
            }
            if (sets[set_id].mandatory)
                continue;
            const Set& set = instance.set(set_id);
            components[set.component].scores_in.update_key(
                    sets_id_to_component_pos[set_id],
                    {sets[set_id].score, d_score(generator)});
        }
        if (solution_penalty_old + p_best != solution_penalty) {
            throw std::runtime_error(
                    "setcoveringsolver::local_search_row_weighting: "
                    "wrong penalty; "
                    "set_id_1_best: " + std::to_string(set_id_1_best) + "; "
                    "set_id_2_best: " + std::to_string(set_id_2_best) + "; "
                    "solution_penalty_old: " + std::to_string(solution_penalty_old) + "; "
                    "p_best: " + std::to_string(p_best) + "; "
                    "solution_penalty: " + std::to_string(solution_penalty) + "."
                    );
        }

        // Update sets
        sets[set_id_1_best].timestamp = output.number_of_iterations;
        sets[set_id_2_best].timestamp = output.number_of_iterations;
        sets[set_id_1_best].last_addition = component.iterations;
        sets[set_id_2_best].last_removal  = component.iterations;
        sets[set_id_2_best].iterations += (component.iterations - sets[set_id_2_best].last_addition);

        // Update penalties.
        for (ElementId element_id: instance.set(set_id_2_best).elements) {
            if (solution.covers(element_id) == 0) {
                for (SetId set_id: instance.element(element_id).sets) {
                    solution_penalty -= solution_penalties[element_id];
                    component.penalty -= solution_penalties[element_id];
                    sets[set_id].score -= solution_penalties[element_id];
                }

                solution_penalties[element_id] += (std::max)(
                        (Penalty)1,
                        (Penalty)(1e4 / instance.element(element_id).sets.size()));

                for (SetId set_id: instance.element(element_id).sets) {
                    solution_penalty += solution_penalties[element_id];
                    component.penalty += solution_penalties[element_id];
                    sets[set_id].score += solution_penalties[element_id];
                }
            }
        }
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
}

/**
 * Exhaustive search for a solution of a component with fewer sets which are
 * not mandatory: it only has to cover the elements of the component which
 * no mandatory set covers.
 */
class ComponentExhaustiveSearch
{

public:

    ComponentExhaustiveSearch(
            const Instance& instance,
            const std::vector<LocalSearchRowWeightingSet>& sets):
        instance_(instance),
        sets_(sets),
        covers_(instance.number_of_elements(), 0) { }

    /**
     * Search for the smallest set of sets which are not mandatory covering
     * the elements of component 'component_id' which no mandatory set
     * covers, with fewer than 'number_of_sets' sets. Return 'false' if the
     * search was stopped after 'maximum_number_of_nodes' nodes (-1: no
     * limit); otherwise, 'sets()' is such a set of sets if one was found
     * ('found()'), and there is none with fewer sets.
     */
    bool run(
            ComponentId component_id,
            SetId number_of_sets,
            Counter maximum_number_of_nodes)
    {
        found_ = false;
        best_sets_.clear();
        number_of_nodes_ = 0;
        maximum_number_of_nodes_ = maximum_number_of_nodes;

        // Elements to cover.
        elements_.clear();
        for (ElementId element_id: instance_.component(component_id).elements) {
            bool covered = false;
            for (SetId set_id: instance_.element(element_id).sets) {
                if (sets_[set_id].mandatory) {
                    covered = true;
                    break;
                }
            }
            if (!covered)
                elements_.push_back(element_id);
        }

        // Search with fewer and fewer sets.
        bool complete = true;
        for (SetId depth = number_of_sets - 1; depth >= 0; ) {
            current_sets_.clear();
            bool aborted = false;
            if (!search(depth, aborted)) {
                complete = !aborted;
                break;
            }
            found_ = true;
            best_sets_ = current_sets_;
            for (SetId set_id: current_sets_)
                for (ElementId element_id: instance_.set(set_id).elements)
                    covers_[element_id]--;
            depth = (SetId)best_sets_.size() - 1;
        }
        for (SetId set_id: current_sets_)
            for (ElementId element_id: instance_.set(set_id).elements)
                covers_[element_id] = 0;
        return complete;
    }

    /** Return 'true' iff a set of sets with fewer sets was found. */
    bool found() const { return found_; }

    /** Get the best set of sets found. */
    const std::vector<SetId>& sets() const { return best_sets_; }

private:

    /**
     * Cover the remaining elements with at most 'depth' more sets, branching
     * on the sets covering the uncovered element covered by the fewest sets.
     */
    bool search(SetId depth, bool& aborted)
    {
        ElementId element_id_best = -1;
        for (ElementId element_id: elements_) {
            if (covers_[element_id] > 0)
                continue;
            if (element_id_best == -1
                    || instance_.element(element_id).sets.size()
                    < instance_.element(element_id_best).sets.size()) {
                element_id_best = element_id;
            }
        }
        if (element_id_best == -1)
            return true;
        if (depth == 0)
            return false;
        for (SetId set_id: instance_.element(element_id_best).sets) {
            number_of_nodes_++;
            if (maximum_number_of_nodes_ != -1
                    && number_of_nodes_ > maximum_number_of_nodes_) {
                aborted = true;
                return false;
            }
            current_sets_.push_back(set_id);
            for (ElementId element_id: instance_.set(set_id).elements)
                covers_[element_id]++;
            if (search(depth - 1, aborted))
                return true;
            for (ElementId element_id: instance_.set(set_id).elements)
                covers_[element_id]--;
            current_sets_.pop_back();
            if (aborted)
                return false;
        }
        return false;
    }

    const Instance& instance_;

    const std::vector<LocalSearchRowWeightingSet>& sets_;

    /** Number of sets of 'current_sets_' covering each element. */
    std::vector<SetId> covers_;

    /** Elements to cover. */
    std::vector<ElementId> elements_;

    std::vector<SetId> current_sets_;

    std::vector<SetId> best_sets_;

    bool found_ = false;

    Counter number_of_nodes_ = 0;

    Counter maximum_number_of_nodes_ = -1;

};

}

const LocalSearchRowWeightingOutput setcoveringsolver::local_search_row_weighting(
        const Instance& instance,
        const Solution* initial_solution,
        std::mt19937_64& generator,
        const LocalSearchRowWeightingParameters& parameters)
{
    LocalSearchRowWeightingOutput output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output, initial_solution);
    algorithm_formatter.start("Row weighting local search");

    if (instance.number_of_elements() == 0) {
        algorithm_formatter.end();
        return output;
    }
    if (parameters.timer.needs_to_end()) {
        algorithm_formatter.end();
        return output;
    }

    // Reduction.
    if (parameters.reduction_parameters.reduce) {
        return solve_reduced_instance(
                [&generator](
                    const Instance& instance,
                    const Solution* initial_solution,
                    const LocalSearchRowWeightingParameters& parameters)
                {
                    return local_search_row_weighting(
                            instance,
                            initial_solution,
                            generator,
                            parameters);
                },
                instance,
                parameters,
                algorithm_formatter,
                output);
    }

    algorithm_formatter.print_header();

    // Compute initial bound.
    Parameters trivial_bound_parameters;
    trivial_bound_parameters.verbosity_level = 0;
    trivial_bound_parameters.reduction_parameters.reduce = false;
    Cost bound = trivial_bound(instance, nullptr, trivial_bound_parameters).bound;
    algorithm_formatter.update_bound(bound, "trivial bound");

    // Mandatory sets.
    std::vector<LocalSearchRowWeightingSet> sets(instance.number_of_sets());
    for (ElementId element_id = 0;
            element_id < instance.number_of_elements();
            ++element_id) {
        if (instance.element(element_id).sets.size() == 1)
            sets[instance.element(element_id).sets.front()].mandatory = true;
    }

    // Compute initial greedy solution.
    // The local search requires a feasible initial solution.
    Solution solution(instance);
    if (initial_solution != nullptr && initial_solution->feasible()) {
        solution = *initial_solution;
        algorithm_formatter.update_solution(solution, "initial solution");
    } else {
        Parameters greedy_parameters;
        greedy_parameters.verbosity_level = 0;
        greedy_parameters.reduction_parameters.reduce = false;
        Output greedy_output = greedy_or_greedy_reverse(instance, nullptr, greedy_parameters);
        algorithm_formatter.update_solution(greedy_output.solution, "initial solution");
        solution = greedy_output.solution;
    }
    for (SetId set_id = 0; set_id < instance.number_of_sets(); ++set_id)
        if (sets[set_id].mandatory && !solution.contains(set_id))
            solution.add(set_id);

    // Initialize local search structures.

    // Current best solution.
    optimizationtools::IndexedSet solution_tmp(instance.number_of_sets());
    for (SetId set_id: solution.sets())
        solution_tmp.add(set_id);

    std::vector<SetPos> sets_id_to_component_pos(instance.number_of_sets(), -1);
    optimizationtools::IndexedSet scores_in_to_update(instance.number_of_sets());
    optimizationtools::DoublyIndexedMap component_uncovered_elements(
            instance.number_of_elements(),
            instance.number_of_components() + 1);
    std::vector<LocalSearchRowWeightingComponent> components(instance.number_of_components());
    components[0].itmode_start = 0;
    for (ComponentId component_id = 0;
            component_id < instance.number_of_components();
            ++component_id) {
        const Component& component = instance.component(component_id);
        components[component_id].scores_in
            = optimizationtools::Indexed4aryHeap<std::pair<Penalty, double>>(component.sets.size());
        for (SetPos pos = 0;
                pos < (SetPos)component.sets.size();
                ++pos) {
            SetId set_id = component.sets[pos];
            sets_id_to_component_pos[set_id] = pos;
        }
        components[component_id].itmode_end = components[component_id].itmode_start
            + instance.component(component_id).elements.size();
        if (component_id + 1 < instance.number_of_components())
            components[component_id + 1].itmode_start = components[component_id].itmode_end;
    }
    std::uniform_real_distribution<double> d_score(0, 1);

    for (SetId set_id: solution.sets())
        sets[set_id].last_addition = 0;

    // Initialize element penalties and set scores.
    Penalty solution_penalty = 0;
    std::vector<Penalty> solution_penalties(instance.number_of_elements(), 1);
    for (ElementId element_id = 0;
            element_id < instance.number_of_elements();
            ++element_id) {
        solution_penalties[element_id] = (std::max)(
                (Penalty)1,
                (Penalty)(1e4 / instance.element(element_id).sets.size()));
        if (solution.covers(element_id) == 1)
            for (SetId set_id: instance.element(element_id).sets)
                if (solution.contains(set_id))
                    sets[set_id].score += solution_penalties[element_id];
    }

    // Initialize scores_in.
    for (SetId set_id: solution.sets()) {
        if (sets[set_id].mandatory)
            continue;
        const Set& set = instance.set(set_id);
        SetPos pos = sets_id_to_component_pos[set_id];
        LocalSearchRowWeightingComponent& component = components[set.component];
        component.scores_in.update_key(
                pos,
                {sets[set_id].score, d_score(generator)});
    }

    ComponentExhaustiveSearch exhaustive_search(instance, sets);

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
        if (output.optimal())
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

        // Update best solution.
        if (output.number_of_iterations % parameters.best_solution_update_frequency == 0) {
            if (output.solution.cost() > solution_tmp.size()) {
                Solution solution_best(instance);
                for (SetId set_id: solution_tmp)
                    solution_best.add(set_id);
                std::stringstream ss;
                ss << "it " << output.number_of_iterations;
                algorithm_formatter.update_solution(solution_best, ss.str());
            }
        }

        while (solution.feasible(component_id)) {
            for (SetId set_id: instance.component(component_id).sets) {
                if (solution.contains(set_id)
                        && !solution_tmp.contains(set_id)) {
                    solution_tmp.add(set_id);
                } else if (!solution.contains(set_id)
                        && solution_tmp.contains(set_id)) {
                    solution_tmp.remove(set_id);
                }
            }
            // Update statistics
            number_of_iterations_without_improvement = 0;
            if (component.iterations_without_improvment > 0)
                component.iterations_without_improvment = 0;

            // With few sets which are not mandatory, search exhaustively
            // for a solution of the component with fewer sets (the
            // component is optimal if all its sets are mandatory). Done with
            // up to 2 such sets, so that at least 2 sets can be removed in
            // the neighborhoods.
            bool component_optimal = component.scores_in.empty();
            SetId number_of_sets = component.scores_in.size();
            if (!component_optimal
                    && (number_of_sets <= 2
                        || number_of_sets <= parameters.exhaustive_search_maximum_number_of_sets)
                    && exhaustive_search.run(
                        component_id,
                        number_of_sets,
                        (number_of_sets <= 2)? -1:
                        parameters.exhaustive_search_maximum_number_of_nodes)) {
                component_optimal = true;
                if (exhaustive_search.found()) {
                    // Replace the component's sets which are not mandatory
                    // in the best solution.
                    for (SetId set_id: instance.component(component_id).sets)
                        if (!sets[set_id].mandatory && solution_tmp.contains(set_id))
                            solution_tmp.remove(set_id);
                    for (SetId set_id: exhaustive_search.sets())
                        solution_tmp.add(set_id);
                }
            }
            if (component_optimal) {
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
                    if (output.solution.cost() > solution_tmp.size()) {
                        Solution solution_best(instance);
                        for (SetId set_id: solution_tmp)
                            solution_best.add(set_id);
                        std::stringstream ss;
                        ss << "it " << output.number_of_iterations;
                        algorithm_formatter.update_solution(solution_best, ss.str());
                    }
                    algorithm_formatter.end();
                    return output;
                }
                break;
            }

            remove_set(
                    instance,
                    generator,
                    output,
                    sets_id_to_component_pos,
                    component_id,
                    solution,
                    component_uncovered_elements,
                    solution_penalty,
                    solution_penalties,
                    sets,
                    components,
                    scores_in_to_update,
                    d_score);
        }
        if (components[component_id].optimal)
            continue;

        if (component.iterations < 10 * instance.component(component_id).sets.size()) {
            explore_remove_add_neighborhood(
                    instance,
                    generator,
                    output,
                    sets_id_to_component_pos,
                    component_id,
                    solution,
                    component_uncovered_elements,
                    solution_penalty,
                    solution_penalties,
                    sets,
                    components,
                    scores_in_to_update,
                    d_score);
        } else {
            explore_swap_neighborhood(
                    instance,
                    generator,
                    output,
                    sets_id_to_component_pos,
                    component_id,
                    solution,
                    component_uncovered_elements,
                    solution_penalty,
                    solution_penalties,
                    sets,
                    components,
                    scores_in_to_update,
                    d_score);
        }

        // Update component.iterations and component.iterations_without_improvment.
        component.iterations++;
        component.iterations_without_improvment++;
    }

    // Uppdate best solution.
    if (output.solution.cost() > solution_tmp.size()) {
        Solution solution_best(instance);
        for (SetId set_id: solution_tmp)
            solution_best.add(set_id);
        std::stringstream ss;
        ss << "it " << output.number_of_iterations;
        algorithm_formatter.update_solution(solution_best, ss.str());
    }

    algorithm_formatter.end();
    return output;
}
