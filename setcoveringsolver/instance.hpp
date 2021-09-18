#pragma once

#include "optimizationtools/info.hpp"
#include "optimizationtools/indexed_set.hpp"
#include "optimizationtools/indexed_map.hpp"

#include <random>
#include <set>

namespace setcoveringsolver
{

typedef int64_t ElementId; // e
typedef int64_t ElementPos; // e_pos
typedef int64_t SetId; // s
typedef int64_t SetPos; // s_pos
typedef int64_t ComponentId; // c
typedef int64_t Cost;
typedef int64_t Counter;
typedef int64_t Seed;

class Solution;

/******************************************************************************/

struct Element
{
    ElementId id;

    std::vector<SetId> sets;

    /**
     * Neighbors of the element.
     * Two elements are neighbors if there exists a set which covers both of them.
     * This attribute is not computed by default, use instance.compute_element_neighbors().
     * This might require a large amount of memory for large instances.
     */
    std::vector<SetId> neighbors;

    /**
     * Index of the connected component in which the element belongs.
     * This attribute is not computed by default, use instance.compute_components().
     */
    ComponentId component = -1;
};

struct Set
{
    SetId id;

    Cost cost = 1;

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

struct Component
{
    ComponentId id;
    std::vector<ElementId> elements;
    std::vector<SetId> sets;
};

class Instance 
{

public:

    /*
     * Constructors and destructor.
     */

    /** Create an instance from a file. */
    Instance(std::string instance_path, std::string format);

    /** Create an instance manually. */
    Instance(SetId number_of_sets, ElementId number_of_elements);
    /** Add an between set 's' and element 'e'. */
    void add_arc(SetId s, ElementId e);

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
    void compute_set_neighbors(Counter number_of_threads, optimizationtools::Info& info);

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

    /** Get element 'e'. */
    inline const Element& element(ElementId e) const { return elements_[e]; }
    /** Get set 's'. */
    inline const Set& set(SetId s) const { return sets_[s]; }
    /** Get component 'c'. */
    inline const Component& component(ComponentId c) const { return components_[c]; }

    /** Get the number of unfixed elements. */
    inline ElementId number_of_unfixed_elements() const { return elements_.size() - fixed_elements_.size(); }
    /** Get the number of unfixed sets. */
    inline SetId number_of_unfixed_sets() const { return sets_.size() - fixed_sets_.size(); }
    /** Get the set of fixed elements. */
    inline const optimizationtools::IndexedSet& fixed_elements() const { return fixed_elements_; }
    /** Get the number of elements in component 'c'. */
    inline ElementId number_of_elements(ComponentId c) const { return components_[c].elements.size(); }

    /*
     * Export.
     */

    /** Write the instance to a file. */
    void write(std::string instance_path, std::string format);

    /*
     * Checkers.
     */

    /** Check if set index 's' is within the correct range. */
    inline void check_set_index(SetId s) const
    {
        if (s < 0 || s >= number_of_sets())
            throw std::out_of_range(
                    "Invalid set index: \"" + std::to_string(s) + "\"."
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

    void compute_set_neighbors_worker(SetId s_start, SetId s_end);
    void remove_elements(const optimizationtools::IndexedSet& elements);
    void remove_sets(const optimizationtools::IndexedSet& sets);

};

std::ostream& operator<<(std::ostream &os, const Instance& ins);

}

