#pragma once

#include "setcoveringsolver/instance.hpp"

namespace setcoveringsolver
{

class InstanceBuilder
{

public:

    /** Constructor. */
    InstanceBuilder() { }

    void move(
            Instance&& instance,
            SetId new_number_of_sets,
            ElementId new_number_of_elements);

    /** Add sets. */
    void add_sets(SetId number_of_sets);

    /** Add elements. */
    void add_elements(ElementId number_of_elements);

    /** Set the cost of a set. */
    void set_cost(
            SetId set_id,
            Cost cost);

    /** Add an between a set and an element. */
    inline void add_arc(
            SetId set_id,
            ElementId element_id)
    {
        //instance_.check_set_index(set_id);
        //instance_.check_element_index(element_id);

        instance_.elements_[element_id].sets.push_back(set_id);
        instance_.sets_[set_id].elements.push_back(element_id);
    }

    /** Set the cost of all sets to 1. */
    void set_unicost();

    /** Read an instance from a file. */
    void read(
            const std::string& instance_path,
            const std::string& format);

    /** Read an instance file in 'pace2019' format. */
    void read_pace2019_vc(FILE* file);

    /** Read an instance file in 'pace2025' format. */
    void read_pace2025(FILE* file);

    /** Read an instance file in 'pace2025_ds' format. */
    void read_pace2025_ds(FILE* file);

    /*
     * Build
     */

    /** Build. */
    Instance build();

private:

    /*
     * Private methods
     */

    /** Compute the number of arcs. */
    void compute_number_of_arcs();

    /** Compute the total cost. */
    void compute_total_cost();

    /** Compute the connected components of the instance. */
    void compute_components();

    /*
     * Read input file
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

    /*
     * Private attributes
     */

    /** Instance. */
    Instance instance_;

};

}
