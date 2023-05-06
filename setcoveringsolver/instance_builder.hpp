#pragma once

#include "setcoveringsolver/instance.hpp"

namespace setcoveringsolver
{

class InstanceBuilder
{

public:

    /** Constructor. */
    InstanceBuilder() { }

    /** Add sets. */
    void add_sets(SetId number_of_sets);

    /** Add elements. */
    void add_elements(ElementId number_of_elements);

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

    /** Read an instance from a file. */
    void read(
            std::string instance_path,
            std::string format);

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
