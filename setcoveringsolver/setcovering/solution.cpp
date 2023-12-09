#include "setcoveringsolver/setcovering/solution.hpp"

using namespace setcoveringsolver::setcovering;

Solution::Solution(const Instance& instance):
    instance_(&instance),
    elements_(instance.number_of_elements(), 0),
    sets_(instance.number_of_sets()),
    component_number_of_elements_(instance.number_of_components(), 0),
    component_costs_(instance.number_of_components(), 0)
{
    if (instance.is_reduced())
        cost_ = instance.unreduction_info().extra_cost;
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

void Solution::update(const Solution& solution)
{
    if (&instance() != &solution.instance()
            && &instance() != solution.instance().original_instance()) {
        throw std::runtime_error(
                "Cannot update a solution with a solution from a different instance.");
    }

    if (solution.instance().is_reduced()
            && solution.instance().original_instance() == &instance()) {
        for (SetId set_id = 0;
                set_id < instance().number_of_sets();
                ++set_id) {
            if (contains(set_id))
                remove(set_id);
        }
        for (SetId set_id: solution.instance().unreduction_info().mandatory_sets) {
            //std::cout << "mandatory " << set_id << std::endl;
            add(set_id);
        }
        for (SetId set_id = 0;
                set_id < solution.instance().number_of_sets();
                ++set_id) {
            if (solution.contains(set_id)) {
                SetId set_id_2 = solution.instance().unreduction_info().unreduction_operations[set_id];
                add(set_id_2);
            }
        }
        //if (cost() != solution.cost() + solution.instance().unreduction_info().extra_cost) {
        //    throw std::runtime_error(
        //            "Wrong cost after unreduction. Weight: "
        //            + std::to_string(cost())
        //            + "; reduced solution cost: "
        //            + std::to_string(solution.cost())
        //            + "; extra cost: "
        //            + std::to_string(solution.instance().unreduction_info().extra_cost)
        //            + ".");
        //}
    } else {
        *this = solution;
    }
}

void Solution::write(std::string certificate_path) const
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
