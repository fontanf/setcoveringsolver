#pragma once

#include "setcoveringsolver/setcovering/instance.hpp"

#include "optimizationtools/containers/indexed_set.hpp"
#include "optimizationtools/containers/indexed_map.hpp"

namespace setcoveringsolver
{
namespace setcovering
{

class Solution
{

public:

    /*
     * Constructors and destructor
     */

    /** Create an empty solution. */
    Solution(const Instance& instance);

    /** Create a solution from a certificate file. */
    Solution(
            const Instance& instance,
            std::string certificate_path);

    void update(const Solution& solution);

    /*
     * Getters
     */

    /** Get the instance. */
    inline const Instance& instance() const { return *instance_; }

    /** Get the number of covered elements. */
    inline ElementId number_of_elements() const { return elements_.size(); }

    /** Get the number of covered elements in a component. */
    inline ElementId number_of_elements(ComponentId component_id) const { return component_number_of_elements_[component_id]; }

    /** Get the number of uncovered elements. */
    inline ElementId number_of_uncovered_elements() const { return instance().number_of_elements() - number_of_elements(); }

    /** Get the number of sets in the solution. */
    inline SetId number_of_sets() const { return sets_.size(); }

    /** Get the total cost of the sets of a component. */
    inline Cost cost(ComponentId component_id) const { return component_costs_[component_id]; }

    /** Get the total cost of the solution. */
    inline Cost cost() const { return cost_; }

    /** Return 'true' iff a given element is covered in the solution. */
    inline SetId covers(ElementId element_id) const { return elements_[element_id]; }

    /** Return 'true' iff the solution contains a given set. */
    inline bool contains(SetId set_id) const { assert(set_id >= 0); assert(set_id < instance().number_of_sets()); return sets_.contains(set_id); }

    /** Return 'true' iff the solution is feasible. */
    inline bool feasible() const { return number_of_elements() == instance().number_of_elements(); }

    /** Return 'true' iff the solution is feasible for a component. */
    inline bool feasible(ComponentId component_id) const { return number_of_elements(component_id) == instance().number_of_elements(component_id); }

    /** Get the set of elements of the solution. */
    const optimizationtools::IndexedMap<SetPos>& elements() const { return elements_; };

    /** Get the set of sets of the solution. */
    const optimizationtools::IndexedSet& sets() const { return sets_; };

    bool is_strictly_better_than(const Solution& solution) const;

    /*
     * Setters
     */

    /** Add a set to the solution. */
    inline void add(SetId set_id);

    /** Remove a set from the solution. */
    inline void remove(SetId set_id);

    /*
     * Export
     */

    /** Print the instance. */
    std::ostream& print(
            std::ostream& os,
            int verbose = 1) const;

    /** Write the solution to a file. */
    void write(std::string certificate_path);

private:

    /*
     * Private attributes
     */

    /** Instance. */
    const Instance* instance_;

    /** Elements. */
    optimizationtools::IndexedMap<SetPos> elements_;

    /** Sets. */
    optimizationtools::IndexedSet sets_;

    /** Number of elements in each component. */
    std::vector<ElementPos> component_number_of_elements_;

    /** Cost of each component. */
    std::vector<Cost> component_costs_;

    /** Total cost of the solution. */
    Cost cost_ = 0;

};

void Solution::add(SetId set_id)
{
    // Checks.
    instance().check_set_index(set_id);
    if (contains(set_id))
        throw std::invalid_argument(
                "Cannot add set " + std::to_string(set_id)
                + " which is already in the solution");

    ComponentId component_id = instance().set(set_id).component;
    for (ElementId element_id: instance().set(set_id).elements) {
        if (covers(element_id) == 0)
            component_number_of_elements_[component_id]++;
        elements_.set(element_id, elements_[element_id] + 1);
    }
    sets_.add(set_id);
    component_costs_[component_id] += instance().set(set_id).cost;
    cost_ += instance().set(set_id).cost;
}

void Solution::remove(SetId set_id)
{
    // Checks.
    instance().check_set_index(set_id);
    if (!contains(set_id))
        throw std::invalid_argument(
                "Cannot remove set " + std::to_string(set_id)
                + " which is not in the solution");

    ComponentId component_id = instance().set(set_id).component;
    for (ElementId element_id: instance().set(set_id).elements) {
        elements_.set(element_id, elements_[element_id] - 1);
        if (covers(element_id) == 0)
            component_number_of_elements_[component_id]--;
    }
    sets_.remove(set_id);
    component_costs_[component_id] -= instance().set(set_id).cost;
    cost_ -= instance().set(set_id).cost;
}

std::ostream& operator<<(std::ostream& os, const Solution& solution);

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// Output ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/**
 * Output structure for a set covering problem.
 */
struct Output
{
    /** Constructor. */
    Output(
            const Instance& instance,
            optimizationtools::Info& info);

    /** Solution. */
    Solution solution;

    /** Bound. */
    Cost bound = 0;

    /** Elapsed time. */
    double time = -1;

    /** Return 'true' iff the solution is optimal. */
    bool optimal() const { return solution.feasible() && solution.cost() == bound; }

    /** Print current state. */
    void print(
            optimizationtools::Info& info,
            const std::stringstream& s) const;

    /** Update the solution. */
    void update_solution(
            const Solution& solution_new,
            const std::stringstream& s,
            optimizationtools::Info& info);

    /** Update the bound. */
    void update_bound(
            Cost bound_new,
            const std::stringstream& s,
            optimizationtools::Info& info);

    /** Print the algorithm statistics. */
    virtual void print_statistics(
            optimizationtools::Info& info) const { (void)info; }

    /** Method to call at the end of the algorithm. */
    Output& algorithm_end(optimizationtools::Info& info);
};

}
}

