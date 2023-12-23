#pragma once

#include "optimizationtools/utils/output.hpp"

namespace setcoveringsolver
{
namespace setcovering
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

/**
 * Structure for an element.
 */
struct Element
{
    /** Sets that cover the element. */
    std::vector<SetId> sets;

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
     * Index of the connected component in which the set belongs.
     * This attribute is not computed by default, use instance.compute_components().
     */
    ComponentId component = -1;
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

/**
 * Instance class for a Set Covering problem.
 */
class Instance
{

public:

    /*
     * Getters
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

    /** Get the number of elements in a component. */
    inline ElementId number_of_elements(ComponentId component_id) const { return components_[component_id].elements.size(); }

    /** Get set neighbors. */
    const std::vector<std::vector<SetId>>& set_neighbors() const;

    /** Get element neighbors. */
    const std::vector<std::vector<ElementId>>& element_neighbors() const;

    /** Get element set neighbors. */
    const std::vector<std::vector<ElementId>>& element_set_neighbors() const;

    /*
     * Export
     */

    /** Write the instance to a file. */
    void write(
            const std::string& instance_path,
            const std::string& format);

    /** Write a formatted output of the object to a stream. */
    void format(
            std::ostream& os,
            int verbosity_level) const;

    /*
     * Checkers
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

    /** Check if a element index is within the correct range. */
    inline void check_element_index(ElementId element_id) const
    {
        if (element_id < 0 || element_id >= number_of_elements())
            throw std::out_of_range(
                    "Invalid element index: \"" + std::to_string(element_id) + "\"."
                    + " Element indices should belong to [0, "
                    + std::to_string(number_of_elements() - 1) + "].");
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

    /** Components. */
    std::vector<Component> components_;

    /** Set neighbors. */
    mutable std::vector<std::vector<SetId>> set_neighbors_;

    /** Element neighbors. */
    mutable std::vector<std::vector<ElementId>> element_neighbors_;

    /** Element set neighbors. */
    mutable std::vector<std::vector<ElementId>> element_set_neighbors_;

    /*
     * Private methods.
     */

    /** Create an instance manually. */
    Instance() { }

    /** Compute the neighbors of the sets. */
    void compute_set_neighbors(
            Counter number_of_threads) const;

    void compute_set_neighbors_worker(
            SetId set_id_start,
            SetId set_id_end) const;

    /** Compute the neighbors of the elements. */
    void compute_element_neighbors() const;

    /** Compute the set neighbors of the elements. */
    void compute_element_set_neighbors() const;

    /*
     * Write to a file
     */

    /** Write an instance in 'balas1980' format. */
    void write_balas1980(std::ofstream& file);

    friend class InstanceBuilder;

};

}
}
