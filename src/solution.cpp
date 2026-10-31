#include "setcoveringsolver/solution.hpp"

#include "optimizationtools/utils/utils.hpp"

#include <fstream>
#include <iomanip>

using namespace setcoveringsolver;

namespace
{

/**
 * Get the index of the first set in a certificate format.
 *
 * Each input format has a certificate format with the same name, which uses
 * the same set indices.
 */
SetId certificate_first_set_id(
        const std::string& format)
{
    if (format == ""
            || format == "gecco2020"
            || format == "gecco") {
        return 0;
    } else if (format == "fulkerson1974"
            || format == "sts"
            || format == "balas1980"
            || format == "orlibrary"
            || format == "balas1996"
            || format == "faster1994"
            || format == "faster"
            || format == "wedelin1995"
            || format == "wedelin"
            || format == "pace2019_vc"
            || format == "pace2025"
            || format == "pace2025_ds"
            || format == "dimacs2010_vc") {
        return 1;
    } else {
        throw std::invalid_argument(
                "Unknown certificate format \"" + format + "\".");
    }
}

}

Solution::Solution(const Instance& instance):
    instance_(&instance),
    elements_(instance.number_of_elements(), 0),
    sets_(instance.number_of_sets()),
    component_number_of_elements_(instance.number_of_components(), 0),
    component_costs_(instance.number_of_components(), 0)
{
}

Solution::Solution(
        const Instance& instance,
        const std::string& certificate_path,
        const std::string& format):
    Solution(instance)
{
    if (certificate_path.empty())
        return;
    SetId first_set_id = certificate_first_set_id(format);
    std::ifstream file(certificate_path);
    if (!file.good()) {
        throw std::runtime_error(
                "Unable to open file \"" + certificate_path + "\".");
    }

    SetId number_of_sets;
    SetId set_id;
    if (!(file >> number_of_sets)) {
        throw std::runtime_error(
                "Unable to read the number of sets of certificate file \""
                + certificate_path + "\".");
    }
    for (SetPos set_pos = 0; set_pos < number_of_sets; ++set_pos) {
        if (!(file >> set_id)) {
            throw std::runtime_error(
                    "Certificate file \"" + certificate_path + "\" contains "
                    + std::to_string(set_pos) + " sets instead of "
                    + std::to_string(number_of_sets) + ".");
        }
        if (set_id < first_set_id
                || set_id >= instance.number_of_sets() + first_set_id) {
            throw std::runtime_error(
                    "Invalid set index " + std::to_string(set_id)
                    + " in certificate file \"" + certificate_path + "\";"
                    + " set indices should belong to ["
                    + std::to_string(first_set_id) + ", "
                    + std::to_string(instance.number_of_sets() - 1 + first_set_id)
                    + "].");
        }
        if (contains(set_id - first_set_id)) {
            throw std::runtime_error(
                    "Set " + std::to_string(set_id)
                    + " appears several times in certificate file \""
                    + certificate_path + "\".");
        }
        add(set_id - first_set_id);
    }
}

void Solution::fill()
{
    for (ElementId element_id = 0;
            element_id < instance().number_of_elements();
            ++element_id) {
        const Element& element = instance().element(element_id);
        elements_.set(element_id, element.sets.size());
    }
    sets_.fill();
    cost_ = instance().total_cost();
    for (ComponentId component_id = 0;
            component_id < instance().number_of_components();
            ++component_id) {
        const Component& component = instance().component(component_id);
        component_number_of_elements_[component_id]
            = component.elements.size();
        component_costs_[component_id] = 0;
        for (SetId set_id: component.sets) {
            const Set& set = instance().set(set_id);
            component_costs_[component_id] += set.cost;
        }
    }
}

void Solution::write(
        const std::string& certificate_path,
        const std::string& format) const
{
    if (certificate_path.empty())
        return;
    std::ofstream file(certificate_path);
    if (!file.good()) {
        throw std::runtime_error(
                "Unable to open file \"" + certificate_path + "\".");
    }

    if (certificate_first_set_id(format) == 0) {
        write_gecco2020(file);
    } else {
        write_pace2025(file);
    }
    file.close();
}

void Solution::write_gecco2020(
        std::ofstream& file) const
{
    file << number_of_sets() << std::endl;
    for (SetId set_id = 0;
            set_id < instance().number_of_sets();
            ++set_id) {
        if (contains(set_id))
            file << set_id << " ";
    }
}

void Solution::write_pace2025(
        std::ostream& file) const
{
    file << number_of_sets() << std::endl;
    for (SetId set_id = 0;
            set_id < instance().number_of_sets();
            ++set_id) {
        if (contains(set_id))
            file << set_id + 1 << std::endl;
    }
}

void Solution::format(
        std::ostream& os,
        int verbosity_level) const
{
    if (verbosity_level >= 1) {
        os
            << "Number of sets:                " << optimizationtools::Ratio<SetId>(number_of_sets(), instance().number_of_sets()) << std::endl
            << "Number of uncovered elements:  " << optimizationtools::Ratio<SetId>(number_of_uncovered_elements(), instance().number_of_elements()) << std::endl
            << "Feasible:                      " << feasible() << std::endl
            << "Cost:                          " << cost() << std::endl
            << "Unselected sets cost:          " << unselected_sets_cost() << std::endl
            ;
    }

    if (verbosity_level >= 2) {
        os << std::right << std::endl
            << std::setw(12) << "Set"
            << std::setw(12) << "Cost"
            << std::endl
            << std::setw(12) << "---"
            << std::setw(12) << "----"
            << std::endl;
        for (SetId set_id = 0;
                set_id < instance().number_of_sets();
                ++set_id) {
            if (contains(set_id)) {
                os
                     << std::setw(12) << set_id
                     << std::setw(12) << instance().set(set_id).cost
                     << std::endl;
            }
        }
    }
}

nlohmann::json Solution::to_json() const
{
    return nlohmann::json {
        {"NumberOfSets", number_of_sets()},
        {"NumberOfUncoveredElements", number_of_uncovered_elements()},
        {"Feasible", feasible()},
        {"Cost", cost()},
        {"UnselectedSetsCost", unselected_sets_cost()},
    };
}
