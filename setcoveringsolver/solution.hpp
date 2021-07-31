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

    Solution(const Instance& instance);
    Solution(const Instance& instance, std::string filepath);
    Solution(const Solution& solution);
    Solution& operator=(const Solution& solution);
    ~Solution() { }
    bool operator==(const Solution& solution);

    inline const Instance& instance() const { return instance_; }
    inline ElementId number_of_elements() const { return elements_.size(); }
    inline ElementId number_of_elements(ComponentId c) const { return number_of_components_of_elementss_[c]; }
    inline ElementId number_of_uncovered_elements() const { return instance().number_of_elements() - number_of_elements(); }
    inline SetId number_of_sets() const { return sets_.size(); }
    inline Cost cost(ComponentId c) const { return component_costs_[c]; }
    inline Cost cost() const { return cost_; }
    inline Cost penalty() const { return penalty_; }
    inline Cost penalty(ElementId e) const { return penalties_[e]; }
    inline SetId covers(ElementId e) const { return elements_[e]; }
    inline bool contains(SetId s) const { assert(s >= 0); assert(s < instance().number_of_sets()); return sets_.contains(s); }
    inline bool feasible() const { return number_of_elements() == instance().number_of_elements(); }
    inline bool feasible(ComponentId c) const { return number_of_elements(c) == instance().number_of_elements(c); }

    const optimizationtools::IndexedMap<SetPos>& elements() const { return elements_; };
    const optimizationtools::IndexedSet& sets() const { return sets_; };

    inline void add(SetId s);
    inline void remove(SetId s);

    void increment_penalty(ElementId e, Cost p = 1);
    void set_penalty(ElementId e, Cost p);

    void write(std::string filepath);

private:

    const Instance& instance_;

    optimizationtools::IndexedMap<SetPos> elements_;
    optimizationtools::IndexedSet sets_;
    std::vector<ElementPos> number_of_components_of_elementss_;
    std::vector<Cost> component_costs_;
    std::vector<Cost> penalties_;
    Cost cost_ = 0;
    Cost penalty_ = 0;

};

void Solution::add(SetId s)
{
    assert(s >= 0);
    assert(s < instance().number_of_sets());
    assert(!contains(s));
    ComponentId c = instance().set(s).component;
    for (ElementId e: instance().set(s).elements) {
        if (covers(e) == 0) {
            penalty_ -= penalties_[e];
            number_of_components_of_elementss_[c]++;
        }
        elements_.set(e, elements_[e] + 1);
    }
    sets_.add(s);
    component_costs_[c] += instance().set(s).cost;
    cost_               += instance().set(s).cost;
}

void Solution::remove(SetId s)
{
    assert(s >= 0);
    assert(s < instance().number_of_sets());
    assert(contains(s));
    ComponentId c = instance().set(s).component;
    for (ElementId e: instance().set(s).elements) {
        elements_.set(e, elements_[e] - 1);
        if (covers(e) == 0) {
            penalty_ += penalties_[e];
            number_of_components_of_elementss_[c]--;
        }
    }
    sets_.remove(s);
    component_costs_[c] -= instance().set(s).cost;
    cost_               -= instance().set(s).cost;
}

std::ostream& operator<<(std::ostream& os, const Solution& solution);

/*********************************** Output ***********************************/

struct Output
{
    Output(const Instance& instance, Info& info);
    Solution solution;
    Cost lower_bound = 0;
    double time = -1;

    bool optimal() const { return solution.feasible() && solution.cost() == lower_bound; }
    Cost upper_bound() const { return (solution.feasible())? solution.cost(): solution.instance().total_cost(); }
    double gap() const;
    void print(Info& info, const std::stringstream& s) const;

    void update_solution(const Solution& solution_new, const std::stringstream& s, Info& info) { update_solution(solution_new, -1, s, info); }
    void update_solution(const Solution& solution_new, ComponentId c, const std::stringstream& s, Info& info);
    void update_lower_bound(Cost lower_bound_new, const std::stringstream& s, Info& info);

    Output& algorithm_end(Info& info);
};

Cost algorithm_end(Cost lower_bound, Info& info);

}

