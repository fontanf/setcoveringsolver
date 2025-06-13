#include "setcoveringsolver/algorithms/clique_cover_bound.hpp"

#include "setcoveringsolver/algorithm_formatter.hpp"

#include "coloringsolver/instance.hpp"
#include "coloringsolver/algorithms/greedy.hpp"

using namespace setcoveringsolver;

const Output setcoveringsolver::clique_cover_bound(
        const Instance& instance,
        const Parameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("Clique cover bound");

    if (instance.number_of_elements() == 0) {
        algorithm_formatter.end();
        return output;
    }

    // Reduction.
    if (parameters.reduction_parameters.reduce)
        return solve_reduced_instance(clique_cover_bound, instance, parameters, algorithm_formatter, output);

    algorithm_formatter.print_header();

    ElementPos number_of_2_elements = 0;
    for (ElementId element_id = 0;
            element_id < instance.number_of_elements();
            ++element_id) {
        const Element& element = instance.element(element_id);
        if (element.sets.size() == 2)
            number_of_2_elements++;
    }
    ElementPos number_of_edges_complementary
        = instance.number_of_sets() * (instance.number_of_sets() - 1) / 2
        - number_of_2_elements;

    if (number_of_edges_complementary <= 1e7) {
        // Graph coloring bound.

        optimizationtools::AdjacencyListGraphBuilder graph_builder;
        for (SetId set_id = 0;
                set_id < instance.number_of_sets();
                ++set_id) {
            graph_builder.add_vertex();
        }
        for (ElementId element_id = 0;
                element_id < instance.number_of_elements();
                ++element_id) {
            const Element& element = instance.element(element_id);
            if (element.sets.size() != 2)
                continue;
            graph_builder.add_edge(element.sets[0], element.sets[1]);
        }
        auto graph = std::shared_ptr<const optimizationtools::AbstractGraph>(
                new optimizationtools::AdjacencyListGraph(graph_builder.build().complementary()));

        // Build graph coloring instance.
        coloringsolver::Instance gc_instance(graph);

        // Solve graph coloring instance.
        coloringsolver::Parameters gc_parameters;
        gc_parameters.timer = parameters.timer;
        gc_parameters.verbosity_level = 0;
        std::mt19937_64 generator;
        auto gc_output = coloringsolver::greedy_dsatur(
                gc_instance,
                gc_parameters);
        Cost bound = 0;
        std::vector<SetId> maximum_cost_set_id(gc_output.solution.number_of_colors(), -1);
        for (SetId set_id = 0;
                set_id < instance.number_of_sets();
                ++set_id) {
            const Set& set = instance.set(set_id);
            coloringsolver::ColorId color_id = gc_output.solution.color(set_id);
            if (maximum_cost_set_id[color_id] == -1
                    || instance.set(maximum_cost_set_id[color_id]).cost < set.cost) {
                maximum_cost_set_id[color_id] = set_id;
            }
        }
        for (SetId set_id = 0;
                set_id < instance.number_of_sets();
                ++set_id) {
            const Set& set = instance.set(set_id);
            coloringsolver::ColorId color_id = gc_output.solution.color(set_id);
            if (set_id == maximum_cost_set_id[color_id])
                continue;
            bound += set.cost;
        }
        algorithm_formatter.update_bound(bound, "graph coloring bound");

    } else {
        // Clqiue cover bound.

        // Sort sets by increasing number of 2-neighbors.
        std::vector<ElementPos> sets_number_of_2_neighbors(instance.number_of_sets(), 0);
        for (ElementId element_id = 0;
                element_id < instance.number_of_elements();
                ++element_id) {
            const Element& element = instance.element(element_id);
            if (element.sets.size() != 2)
                continue;
            sets_number_of_2_neighbors[element.sets[0]]++;
            sets_number_of_2_neighbors[element.sets[1]]++;
        }
        std::vector<SetId> sorted_sets;
        for (SetId set_id = 0;
                set_id < instance.number_of_sets();
                ++set_id) {
            const Set& set = instance.set(set_id);
            if (sets_number_of_2_neighbors[set_id] > 0) {
                sorted_sets.push_back(set_id);
            }
        }
        sort(sorted_sets.begin(), sorted_sets.end(),
                [&sets_number_of_2_neighbors](SetId set_id_1, SetId set_id_2) -> bool
                {
                    return sets_number_of_2_neighbors[set_id_1] < sets_number_of_2_neighbors[set_id_2];
                });

        optimizationtools::IndexedSet set_neighbors(instance.number_of_sets());
        std::vector<std::vector<SetId>> cliques;
        for (SetPos set_pos = 0;
                set_pos < (SetPos)sorted_sets.size();
                ++set_pos) {
            SetId set_id = sorted_sets[set_pos];
            const Set& set = instance.set(set_id);
            //std::cout << "set_pos " << set_pos
            //    << " set_id " << set_id
            //    << " d " << sets_number_of_2_neighbors[set_id]
            //    << std::endl;

            set_neighbors.clear();
            for (ElementId element_id: set.elements) {
                const Element& element = instance.element(element_id);
                if (element.sets.size() != 2)
                    continue;
                if (element.sets[0] == set_id) {
                    set_neighbors.add(element.sets[1]);
                } else {
                    set_neighbors.add(element.sets[0]);
                }
            }

            SetId clique_id_best = -1;
            for (SetId clique_id = 0;
                    clique_id < (SetId)cliques.size();
                    ++clique_id) {
                const std::vector<SetId>& clique = cliques[clique_id];

                bool ok = true;
                for (SetId clique_set_id: clique) {
                    if (!set_neighbors.contains(clique_set_id)) {
                        ok = false;
                        break;
                    }
                }
                if (!ok)
                    continue;

                if (clique_id_best == -1
                        || cliques[clique_id_best].size() < clique.size()) {
                    clique_id_best = clique_id;
                }
            }

            if (clique_id_best != -1) {
                cliques[clique_id_best].push_back(set_id);
            } else {
                cliques.push_back({set_id});
            }
        }

        // Compute bound.
        Cost bound = 0;
        for (SetId clique_id = 0;
                clique_id < (SetId)cliques.size();
                ++clique_id) {
            const std::vector<SetId>& clique = cliques[clique_id];
            SetId maximum_cost_set_id = -1;
            for (SetId set_id: clique) {
                const Set& set = instance.set(set_id);
                if (maximum_cost_set_id == -1
                        || instance.set(maximum_cost_set_id).cost < set.cost) {
                    maximum_cost_set_id = set_id;
                }
            }
            for (SetId set_id: clique) {
                const Set& set = instance.set(set_id);
                if (set_id == maximum_cost_set_id)
                    continue;
                bound += set.cost;
            }
        }
        algorithm_formatter.update_bound(bound, "clique cover bound");
    }

    algorithm_formatter.end();
    return output;
}
