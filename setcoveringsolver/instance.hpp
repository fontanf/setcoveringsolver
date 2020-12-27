#pragma once

#include "optimizationtools/info.hpp"
#include "optimizationtools/indexed_set.hpp"
#include "optimizationtools/indexed_map.hpp"

#include <random>
#include <set>

namespace setcoveringsolver
{

using optimizationtools::Info;

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

    Instance(std::string filepath, std::string format);
    void set_unicost();
    void write(std::string filepath, std::string format);

    Instance(SetId set_number, ElementId element_number);
    void add_arc(SetId s, ElementId e);
    void compute_components();

    inline ElementId      element_number() const { return elements_.size(); }
    inline SetId              set_number() const { return sets_.size(); }
    inline ElementPos         arc_number() const { return arc_number_; }
    inline ComponentId  component_number() const { return components_.size(); }
    inline Cost               total_cost() const { return total_cost_; }

    inline const Element& element(ElementId id) const { return elements_[id]; }
    inline const Set& set(SetId id) const { return sets_[id]; }
    inline const Component& component(ComponentId c) const { return components_[c]; }

    inline ElementId unfixed_element_number() const { return elements_.size() - fixed_elements_.size(); }
    inline SetId unfixed_set_number() const { return sets_.size() - fixed_sets_.size(); }
    inline const optimizationtools::IndexedSet& fixed_elements() const { return fixed_elements_; }
    inline ElementId element_number(ComponentId c) const { return components_[c].elements.size(); }

    void fix_identical(Info& info);
    void fix_dominated(Info& info);
    void compute_set_neighbors(Counter thread_number, Info& info);
    void compute_element_neighbors(Info& info);

private:

    /**
     * Attributes.
     */

    std::vector<Element> elements_;
    std::vector<Set> sets_;
    Cost total_cost_ = 0;
    ElementPos arc_number_ = 0;

    optimizationtools::IndexedSet fixed_sets_;
    optimizationtools::IndexedSet fixed_elements_;
    std::vector<Component> components_;

    /**
     * Private methods.
     */

    void read_fulkerson1974(std::ifstream& file);
    void read_balas1980(std::ifstream& file);
    void read_balas1996(std::ifstream& file);
    void read_faster1994(std::ifstream& file);
    void read_geccod2020(std::ifstream& file);

    void compute_set_neighbors_worker(SetId s_start, SetId s_end);
    void remove_elements(const optimizationtools::IndexedSet& elements);
    void remove_sets(const optimizationtools::IndexedSet& sets);

};

std::ostream& operator<<(std::ostream &os, const Instance& ins);

}

