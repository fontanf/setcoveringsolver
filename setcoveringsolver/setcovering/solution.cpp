#include "setcoveringsolver/setcovering/solution.hpp"

#include "optimizationtools/utils/utils.hpp"

#include <fstream>
#include <iomanip>

using namespace setcoveringsolver::setcovering;

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

void Solution::write(
        const std::string& certificate_path) const
{
    if (certificate_path.empty())
        return;
    std::ofstream file(certificate_path);
    if (!file.good()) {
        throw std::runtime_error(
                "Unable to open file \"" + certificate_path + "\".");
    }

    file << number_of_sets() << std::endl;
    for (SetId set_id = 0; set_id < instance().number_of_sets(); ++set_id)
        if (contains(set_id))
            file << set_id << " ";
    file.close();
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
            ;
    }

    if (verbosity_level >= 2) {
        os << std::endl
            << std::setw(12) << "Set"
            << std::setw(12) << "Cost"
            << std::endl
            << std::setw(12) << "--------"
            << std::setw(12) << "---"
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
        {"Cost", cost()}
    };
}
