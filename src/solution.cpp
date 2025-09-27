#include "setcoveringsolver/solution.hpp"

#include "optimizationtools/utils/utils.hpp"

#include <fstream>
#include <iomanip>

using namespace setcoveringsolver;

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
        std::string certificate_path):
    Solution(instance)
{
    if (certificate_path.empty())
        return;
    std::ifstream file(certificate_path);
    if (!file.good()) {
        throw std::runtime_error(
                "Unable to open file \"" + certificate_path + "\".");
    }

    SetId number_of_sets;
    SetId set_id;
    file >> number_of_sets;
    for (SetPos set_pos = 0; set_pos < number_of_sets; ++set_pos) {
        file >> set_id;
        add(set_id);
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

    if (format == "" || format == "gecco2020" || format == "gecco") {
        write_gecco2020(file);
    } else if (format == "pace2025") {
        write_pace2025(file);
    } else {
        throw std::invalid_argument(
                "Unknown instance format \"" + format + "\".");
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
