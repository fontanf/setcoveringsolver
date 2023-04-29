#pragma once

#include "optimizationtools/utils/info.hpp"
#include "optimizationtools/containers/indexed_set.hpp"
#include "optimizationtools/containers/indexed_map.hpp"

#include <random>
#include <set>

namespace setcoveringsolver
{

using ElementId = int64_t;
using ElementPos = int64_t;
using SetId = int64_t;
using SetPos = int64_t;
using ComponentId = int64_t;
using Cost = int64_t;
using Penalty = int64_t;
using Counter = int64_t;
using Seed = int64_t;

class Solution;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/**
 * Structure for an element.
 */
struct Element
{
    /** Sets that cover the element. */
    std::vector<SetId> sets;

    /**
     * Neighbors of the element.
     * Two elements are neighbors if there exists a set which covers both of them.
     * This attribute is not computed by default, use instance.compute_element_neighbors().
     * This might require a large amount of memory for large instances.
     */
    std::vector<ElementId> neighbors;

    std::vector<SetId> neighbor_sets;

    /**
     * Index of the connected component in which the element belongs.
     * This attribute is not computed by default, use instance.compute_components().
     */
    ComponentId component = -1;
};

/**
 * Structure for a set.
 */
struct Set
{
    /** Cost. */
    Cost cost = 1;

    /** Elements covered by the set. */
    std::vector<ElementId> elements;

    /**
     * Neighbors of the set.
     * Two sets are neighbors if there exists an element which they both cover.
     * This attribute is not computed by default, use instance.compute_set_neighbors().
     * This might require a large amount of memory for large instances.
     */
    std::vector<SetId> neighbors;

    /**
     * Index of the connected component in which the set belongs.
     * This attribute is not computed by default, use instance.compute_components().
     */
    ComponentId component = -1;

    bool mandatory = false;
};

/**
 * Structure for a connected component.
 */
struct Component
{
    /** Elements. */
    std::vector<ElementId> elements;

    /** Sets. */
    std::vector<SetId> sets;
};

class Instance 
{

public:

    /*
     * Constructors and destructor.
     */

    /** Create an instance from a file. */
    Instance(
            std::string instance_path,
            std::string format);

    /** Create an instance manually. */
    Instance(
            SetId number_of_sets,
            ElementId number_of_elements);

    /** Set the cost of a set. */
    void set_cost(
            SetId set_id,
            Cost cost);

    /** Add an between a set and an element. */
    void add_arc(
            SetId set_id,
            ElementId element_id);

    /** Set the cost of all sets to 1. */
    void set_unicost();

    /** Compute the connected components of the instance. */
    void compute_components();

    /** Fix identical sets and elements. */
    void fix_identical(optimizationtools::Info& info);

    /** Fix dominanted sets and elements. */
    void fix_dominated(optimizationtools::Info& info);

    /**
     * Compute the neighbors of the sets.
     *
     * They can then be retrieved with 'set(s).neighbors'.
     */
    void compute_set_neighbors(
            Counter number_of_threads,
            optimizationtools::Info& info);

    void compute_element_neighbor_sets(optimizationtools::Info& info);

    /**
     * Compute the neighbors of the elements.
     *
     * They can then be retrieved with 'element(e).neighbors'.
     */
    void compute_element_neighbors(optimizationtools::Info& info);

    /*
     * Getters.
     */

    /** Get the number of elements. */
    inline ElementId number_of_elements() const { return elements_.size(); }

    /** Get the number of sets. */
    inline SetId number_of_sets() const { return sets_.size(); }

    /** Get the number of arcs. */
    inline ElementPos number_of_arcs() const { return number_of_arcs_; }

    /** Get the number of conntected components. */
    inline ComponentId number_of_components() const { return components_.size(); }

    /** Get the total cost of the sets. */
    inline Cost total_cost() const { return total_cost_; }

    /** Get an element. */
    inline const Element& element(ElementId element_id) const { return elements_[element_id]; }

    /** Get a set. */
    inline const Set& set(SetId set_id) const { return sets_[set_id]; }

    /** Get a component. */
    inline const Component& component(ComponentId component_id) const { return components_[component_id]; }

    /** Get the number of unfixed elements. */
    inline ElementId number_of_unfixed_elements() const { return elements_.size() - fixed_elements_.size(); }

    /** Get the number of unfixed sets. */
    inline SetId number_of_unfixed_sets() const { return sets_.size() - fixed_sets_.size(); }

    /** Get the set of fixed elements. */
    inline const optimizationtools::IndexedSet& fixed_elements() const { return fixed_elements_; }

    /** Get the number of elements in a component. */
    inline ElementId number_of_elements(ComponentId component_id) const { return components_[component_id].elements.size(); }

    /*
     * Export.
     */

    /** Write the instance to a file. */
    void write(std::string instance_path, std::string format);

    /*
     * Checkers.
     */

    /** Check if a set index is within the correct range. */
    inline void check_set_index(SetId set_id) const
    {
        if (set_id < 0 || set_id >= number_of_sets())
            throw std::out_of_range(
                    "Invalid set index: \"" + std::to_string(set_id) + "\"."
                    + " Set indices should belong to [0, "
                    + std::to_string(number_of_sets() - 1) + "].");
    }

private:

    /*
     * Attributes.
     */

    /** Elements. */
    std::vector<Element> elements_;

    /** Sets. */
    std::vector<Set> sets_;

    /** Total cost of the sets. */
    Cost total_cost_ = 0;

    /** Number of arcs. */
    ElementPos number_of_arcs_ = 0;

    /** Set of fixed sets. */
    optimizationtools::IndexedSet fixed_sets_;

    /** Set of fixed elements. */
    optimizationtools::IndexedSet fixed_elements_;

    /** Components. */
    std::vector<Component> components_;

    /*
     * Private methods.
     */

    /** Read an instance file in 'fulkerson1974' format. */
    void read_fulkerson1974(std::ifstream& file);

    /** Read an instance file in 'balas1980' format. */
    void read_balas1980(std::ifstream& file);

    /** Read an instance file in 'balas1996' format. */
    void read_balas1996(std::ifstream& file);

    /** Read an instance file in 'faster1994' format. */
    void read_faster1994(std::ifstream& file);

    /** Read an instance file in 'geccod2020' format. */
    void read_geccod2020(std::ifstream& file);

    /** Write an instance in 'balas1980' format. */
    void write_balas1980(std::ofstream& file);

    void compute_set_neighbors_worker(
            SetId set_id_start,
            SetId set_id_end);

    void remove_elements(const optimizationtools::IndexedSet& elements);

    void remove_sets(
            const optimizationtools::IndexedSet& sets,
            optimizationtools::Info& info);

};

std::ostream& operator<<(std::ostream &os, const Instance& ins);

void init_display(
        const Instance& instance,
        optimizationtools::Info& info);

}

