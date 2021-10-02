#pragma once

#include "setcoveringsolver/instance.hpp"

#include "optimizationtools/indexed_set.hpp"
#include "optimizationtools/indexed_map.hpp"

#include <functional>

namespace setcoveringsolver
{

class Solution
{

public:

    /*
     * Constructors and destructor.
     */

    /** Create an empty solution. */
    Solution(const Instance& instance);
    /** Create a solution from a certificate file. */
    Solution(const Instance& instance, std::string certificate_path);
    /** Copy constructor. */
    Solution(const Solution& solution);
    /** Copy assignment operator. */
    Solution& operator=(const Solution& solution);
    /** Destructor. */
    ~Solution() { }

    /*
     * Getters.
     */

    /** Get the instance. */
    inline const Instance& instance() const { return instance_; }
    /** Get the number of covered elements. */
    inline ElementId number_of_elements() const { return elements_.size(); }
    /** Get the number of covered elements in component 'c'. */
    inline ElementId number_of_elements(ComponentId c) const { return component_number_of_elements_[c]; }
    /** Get the number of uncovered elements. */
    inline ElementId number_of_uncovered_elements() const { return instance().number_of_elements() - number_of_elements(); }
    /** Get the number of sets in the solution. */
    inline SetId number_of_sets() const { return sets_.size(); }
    /** Get the total cost of the sets of component 'c'. */
    inline Cost cost(ComponentId c) const { return component_costs_[c]; }
    /** Get the total cost of the solution. */
    inline Cost cost() const { return cost_; }
    /** Return 'true' iff element 'e' is covered in the solution. */
    inline SetId covers(ElementId e) const { return elements_[e]; }
    /** Return 'true' iff the solution contains set 's'. */
    inline bool contains(SetId s) const { assert(s >= 0); assert(s < instance().number_of_sets()); return sets_.contains(s); }
    /** Return 'true' iff the solution is feasible. */
    inline bool feasible() const { return number_of_elements() == instance().number_of_elements(); }
    /** Return 'true' iff the solution is feasible for component 'c'. */
    inline bool feasible(ComponentId c) const { return number_of_elements(c) == instance().number_of_elements(c); }

    /** Get the set of elements of the solution. */
    const optimizationtools::IndexedMap<SetPos>& elements() const { return elements_; };
    /** Get the set of sets of the solution. */
    const optimizationtools::IndexedSet& sets() const { return sets_; };

    /*
     * Setters.
     */

    /** Add set 's' to the solution. */
    inline void add(SetId s);
    /** Remove set 's' from the solution. */
    inline void remove(SetId s);

    /*
     * Export.
     */

    /** Write the solution to a file. */
    void write(std::string certificate_path);

private:

    /*
     * Private attributes.
     */

    /** Instance. */
    const Instance& instance_;
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

void Solution::add(SetId s)
{
    // Checks.
    instance().check_set_index(s);
    if (contains(s))
        throw std::invalid_argument(
                "Cannot add set " + std::to_string(s)
                + " which is already in the solution");

    ComponentId c = instance().set(s).component;
    for (ElementId e: instance().set(s).elements) {
        if (covers(e) == 0)
            component_number_of_elements_[c]++;
        elements_.set(e, elements_[e] + 1);
    }
    sets_.add(s);
    component_costs_[c] += instance().set(s).cost;
    cost_               += instance().set(s).cost;
}

void Solution::remove(SetId s)
{
    // Checks.
    instance().check_set_index(s);
    if (!contains(s))
        throw std::invalid_argument(
                "Cannot remove set " + std::to_string(s)
                + " which is not in the solution");

    ComponentId c = instance().set(s).component;
    for (ElementId e: instance().set(s).elements) {
        elements_.set(e, elements_[e] - 1);
        if (covers(e) == 0)
            component_number_of_elements_[c]--;
    }
    sets_.remove(s);
    component_costs_[c] -= instance().set(s).cost;
    cost_               -= instance().set(s).cost;
}

std::ostream& operator<<(std::ostream& os, const Solution& solution);

/*********************************** Output ***********************************/

struct Output
{
    Output(
            const Instance& instance,
            optimizationtools::Info& info);
    Solution solution;
    Cost lower_bound = 0;
    double time = -1;

    bool optimal() const { return solution.feasible() && solution.cost() == lower_bound; }
    Cost upper_bound() const { return (solution.feasible())? solution.cost(): solution.instance().total_cost(); }
    double gap() const;
    void print(
            optimizationtools::Info& info,
            const std::stringstream& s) const;

    void update_solution(
            const Solution& solution_new,
            const std::stringstream& s,
            optimizationtools::Info& info)
    {
        update_solution(solution_new, -1, s, info);
    }

    void update_solution(
            const Solution& solution_new,
            ComponentId c,
            const std::stringstream& s,
            optimizationtools::Info& info);

    void update_lower_bound(
            Cost lower_bound_new,
            const std::stringstream& s,
            optimizationtools::Info& info);

    Output& algorithm_end(optimizationtools::Info& info);
};

Cost algorithm_end(
        Cost lower_bound,
        optimizationtools::Info& info);

}

