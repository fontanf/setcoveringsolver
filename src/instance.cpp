#include "setcoveringsolver/instance.hpp"

#include "optimizationtools/containers/indexed_set.hpp"

#include <fstream>
#include <iomanip>
#include <thread>

using namespace setcoveringsolver;

const std::vector<std::vector<SetId>>& Instance::set_neighbors() const
{
    if (set_neighbors_.empty())
        compute_set_neighbors(1);
    return set_neighbors_;
}

const std::vector<std::vector<ElementId>>& Instance::element_neighbors() const
{
    if (element_neighbors_.empty())
        compute_element_neighbors();
    return element_neighbors_;
}

const std::vector<std::vector<ElementId>>& Instance::element_set_neighbors() const
{
    if (element_set_neighbors_.empty())
        compute_element_set_neighbors();
    return element_set_neighbors_;
}

void Instance::compute_set_neighbors(
        Counter number_of_threads) const
{
    set_neighbors_ = std::vector<std::vector<SetId>>(number_of_sets());
    std::vector<std::thread> threads;
    for (Counter thread_id = 0; thread_id < number_of_threads; ++thread_id)
        threads.push_back(std::thread(&Instance::compute_set_neighbors_worker,
                    this,
                    thread_id       * number_of_sets() / number_of_threads,
                    (thread_id + 1) * number_of_sets() / number_of_threads));
    for (Counter thread_id = 0; thread_id < number_of_threads; ++thread_id)
        threads[thread_id].join();
}

void Instance::compute_set_neighbors_worker(
        SetId s_start,
        SetId s_end) const
{
    optimizationtools::IndexedSet neighbors(number_of_sets());
    for (SetId set_id_1 = s_start; set_id_1 < s_end; ++set_id_1) {
        neighbors.clear();
        for (ElementId element_id: set(set_id_1).elements)
            for (SetId set_id_2: element(element_id).sets)
                neighbors.add(set_id_2);
        if (neighbors.contains(set_id_1))
            neighbors.remove(set_id_1);
        for (SetId set_id_2: neighbors)
            set_neighbors_[set_id_1].push_back(set_id_2);
    }
}

void Instance::compute_element_neighbors() const
{
    element_neighbors_ = std::vector<std::vector<ElementId>>(number_of_elements());
    optimizationtools::IndexedSet neighbors(number_of_elements());
    for (ElementId element_id_1 = 0; element_id_1 < number_of_elements(); ++element_id_1) {
        neighbors.clear();
        for (SetId set_id: element(element_id_1).sets)
            for (ElementId element_id_2: set(set_id).elements)
                if (element_id_2 != element_id_1)
                    neighbors.add(element_id_2);
        for (ElementId element_id_2: neighbors)
            element_neighbors_[element_id_1].push_back(element_id_2);
    }
}

void Instance::compute_element_set_neighbors() const
{
    element_set_neighbors_ = std::vector<std::vector<SetId>>(number_of_elements());
    const auto& set_neighbors = this->set_neighbors();
    optimizationtools::IndexedSet neighbors(number_of_sets());
    for (ElementId element_id = 0;
            element_id < number_of_elements();
            ++element_id) {
        neighbors.clear();
        for (SetId set_id: element(element_id).sets) {
            neighbors.add(set_id);
            for (SetId set_id_2: set_neighbors[set_id])
                neighbors.add(set_id_2);
        }
        for (SetId set_id: neighbors)
            element_set_neighbors_[element_id].push_back(set_id);
    }
}

void Instance::write(
        const std::string& instance_path,
        const std::string& format) const
{
    std::ofstream file(instance_path);
    if (!file.good()) {
        throw std::runtime_error(
                "Unable to open file \"" + instance_path + "\".");
    }

    if (format == "gecco2020" || format == "gecco") {
        //write_geccod2020(file);
    } else if (format == "pace2025") {
        write_pace2025(file);
    } else if (format == "fulkerson1974" || format == "sts") {
        //write_fulkerson1974(file);
    } else if (format == "balas1980" || format == "orlibrary") {
        write_balas1980(file);
    } else if (format == "balas1996") {
        //write_balas1996(file);
    } else if (format == "faster1994" || format == "faster" || format == "wedelin1995" || format == "wedelin") {
        //write_faster1994(file);
    } else {
        throw std::invalid_argument(
                "Unknown instance format \"" + format + "\".");
    }
}

void Instance::write_pace2025(std::ofstream& file) const
{
    file << "p hs " << number_of_sets() << " " << number_of_elements() << std::endl;
    for (SetId element_id = 0;
            element_id < number_of_elements();
            ++element_id) {
        for (SetId set_id: element(element_id).sets) {
            file << (set_id + 1) << " ";
        }
        file << std::endl;
    }
}

void Instance::write_balas1980(std::ofstream& file) const
{
    file << number_of_elements() << " " << number_of_sets() << std::endl;
    for (SetId set_id = 0; set_id < number_of_sets(); ++set_id)
        file << " " << set(set_id).cost;
    file << std::endl;

    for (ElementId element_id = 0;
            element_id < number_of_elements();
            ++element_id) {
        file << element(element_id).sets.size();
        for (SetId s: element(element_id).sets)
            file << " " << (s + 1);
        file << std::endl;
    }
}

double Instance::compute_average_number_of_set_neighbors_estimate() const
{
    SetPos number_of_set_neighbors_estimate = 0;
    for (ElementId element_id = 0;
            element_id < this->number_of_elements();
            ++element_id) {
        const Element& element = this->element(element_id);
        number_of_set_neighbors_estimate += element.sets.size() * (element.sets.size() - 1);
    }
    return (double)number_of_set_neighbors_estimate / this->number_of_sets();
}

double Instance::compute_average_number_of_element_neighbors_estimate() const
{
    ElementPos number_of_element_neighbors_estimate = 0;
    for (SetId set_id = 0;
            set_id < this->number_of_sets();
            ++set_id) {
        const Set& set = this->set(set_id);
        number_of_element_neighbors_estimate += set.elements.size() * (set.elements.size() - 1);
    }
    return (double)number_of_element_neighbors_estimate / this->number_of_elements();
}

void Instance::format(
        std::ostream& os,
        int verbosity_level) const
{
    if (verbosity_level >= 1) {
        os
            << "Number of elements:                           " << number_of_elements() << std::endl
            << "Number of sets:                               " << number_of_sets() << std::endl
            << "Number of arcs:                               " << number_of_arcs() << std::endl
            << "Average number of sets covering an element:   " << (double)number_of_arcs() / number_of_elements() << std::endl
            << "Average number of elements covered by a set:  " << (double)number_of_arcs() / number_of_sets() << std::endl
            << "Average number of set neighbors estimate:     " << compute_average_number_of_set_neighbors_estimate() << std::endl
            << "Average number of elt. neighbors estimate:    " << compute_average_number_of_element_neighbors_estimate() << std::endl
            << "Total cost:                                   " << total_cost() << std::endl
            << "Number of connected components:               " << number_of_components() << std::endl
            ;
    }

    if (verbosity_level >= 2) {
        // For each number of elements, print the number of sets that cover this
        // number of elements.
        std::vector<SetPos> number_of_sets(this->number_of_elements(), 0);
        for (SetId set_id = 0; set_id < this->number_of_sets(); ++set_id) {
            const Set& set = this->set(set_id);
            number_of_sets[set.elements.size()]++;
        }
        os << std::right << std::endl
            << std::setw(12) << "# elts"
            << std::setw(12) << "# sets"
            << std::endl
            << std::setw(12) << "------"
            << std::setw(12) << "------"
            << std::endl;
        for (ElementPos e = 0; e < this->number_of_elements(); ++e) {
            if (number_of_sets[e] == 0)
                continue;
            os
                << std::setw(12) << e
                << std::setw(12) << number_of_sets[e]
                << std::endl;
        }

        // For each number of sets, print the number of elements covered by this
        // number of sets.
        std::vector<SetPos> number_of_elements(this->number_of_sets(), 0);
        for (ElementId element_id = 0; element_id < this->number_of_elements(); ++element_id) {
            const Element& element = this->element(element_id);
            number_of_elements[element.sets.size()]++;
        }
        os << std::right << std::endl
            << std::setw(12) << "# sets"
            << std::setw(12) << "# elts"
            << std::endl
            << std::setw(12) << "------"
            << std::setw(12) << "------"
            << std::endl;
        for (SetPos s = 0; s < this->number_of_sets(); ++s) {
            if (number_of_elements[s] == 0)
                continue;
            os
                << std::setw(12) << s
                << std::setw(12) << number_of_elements[s]
                << std::endl;
        }

        // For each component, print the number of elements and the number of
        // sets in the component.
        os << std::right << std::endl
            << std::setw(12) << "Comp."
            << std::setw(12) << "# elts"
            << std::setw(12) << "# sets"
            << std::endl
            << std::setw(12) << "-----"
            << std::setw(12) << "------"
            << std::setw(12) << "------"
            << std::endl;
        for (ComponentId component_id = 0;
                component_id < this->number_of_components();
                ++component_id) {
            os
                << std::setw(12) << component_id
                << std::setw(12) << this->component(component_id).elements.size()
                << std::setw(12) << this->component(component_id).sets.size()
                << std::endl;
        }
    }

    if (verbosity_level >= 3) {
        os << std::right << std::endl
            << std::setw(12) << "SetId"
            << std::setw(12) << "Cost"
            << std::setw(12) << "# elem."
            << std::endl
            << std::setw(12) << "-----"
            << std::setw(12) << "----"
            << std::setw(12) << "-------"
            << std::endl;
        for (SetId set_id = 0;
                set_id < number_of_sets();
                ++set_id) {
            const Set& set = this->set(set_id);
            os
                << std::setw(12) << set_id
                << std::setw(12) << set.cost
                << std::setw(12) << set.elements.size()
                << std::endl;
        }
    }

    if (verbosity_level >= 3) {
        os << std::right << std::endl
            << std::setw(12) << "Set"
            << std::setw(12) << "Element"
            << std::endl
            << std::setw(12) << "---"
            << std::setw(12) << "-------"
            << std::endl;
        for (SetId set_id = 0;
                set_id < number_of_sets();
                ++set_id) {
            const Set& set = this->set(set_id);
            for (ElementId element_id: set.elements) {
                os
                    << std::setw(12) << set_id
                    << std::setw(12) << element_id
                    << std::endl;
            }
        }
    }
}
