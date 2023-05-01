#include "setcoveringsolver/solution.hpp"

#include "optimizationtools/utils/utils.hpp"

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
        if (cost() != solution.cost() + solution.instance().unreduction_info().extra_cost) {
            throw std::runtime_error(
                    "Wrong cost after unreduction. Weight: "
                    + std::to_string(cost())
                    + "; reduced solution cost: "
                    + std::to_string(solution.cost())
                    + "; extra cost: "
                    + std::to_string(solution.instance().unreduction_info().extra_cost)
                    + ".");
        }
    } else {
        *this = solution;
    }
}

std::ostream& Solution::print(
        std::ostream& os,
        int verbose) const
{
    if (verbose >= 1) {
        os
            << "Number of sets:                " << optimizationtools::Ratio<SetId>(number_of_sets(), instance().number_of_sets()) << std::endl
            << "Number of uncovered elements:  " << optimizationtools::Ratio<SetId>(number_of_uncovered_elements(), instance().number_of_elements()) << std::endl
            << "Feasible:                      " << feasible() << std::endl
            << "Cost:                          " << cost() << std::endl
            ;
    }

    if (verbose >= 2) {
        os << std::endl
            << std::setw(12) << "Set"
            << std::setw(12) << "Cost"
            << std::endl
            << std::setw(12) << "--------"
            << std::setw(12) << "---"
            << std::endl;
        for (SetId set_id: sets_) {
            os
                << std::setw(12) << set_id
                << std::setw(12) << instance().set(set_id).cost
                << std::endl;
        }
    }

    return os;
}

void Solution::write(std::string certificate_path)
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

bool Solution::is_strictly_better_than(const Solution& solution) const
{
    if (!feasible())
        return false;
    if (!solution.feasible())
        return true;
    Cost c1 = cost();
    if (instance().is_reduced())
        c1 += instance().unreduction_info().extra_cost;
    Cost c2 = solution.cost();
    //if (instance().is_reduced())
    //    w2 += solution.instance().unreduction_info().extra_cost;
    return c1 < c2;
}

std::ostream& setcoveringsolver::operator<<(std::ostream& os, const Solution& solution)
{
    os << solution.number_of_sets() << std::endl;
    for (SetId set_id = 0;
            set_id < solution.instance().number_of_sets();
            ++set_id)
        if (solution.contains(set_id))
            os << set_id << " ";
    return os;
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// Output ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Output::Output(
        const Instance& instance,
        optimizationtools::Info& info):
    solution(instance)
{
    info.os()
        << std::setw(12) << "T (s)"
        << std::setw(12) << "UB"
        << std::setw(12) << "LB"
        << std::setw(12) << "GAP"
        << std::setw(12) << "GAP (%)"
        << std::setw(24) << "Comment"
        << std::endl
        << std::setw(12) << "-----"
        << std::setw(12) << "--"
        << std::setw(12) << "--"
        << std::setw(12) << "---"
        << std::setw(12) << "-------"
        << std::setw(24) << "-------"
        << std::endl;
    print(info, std::stringstream(""));
}

void Output::print(
        optimizationtools::Info& info,
        const std::stringstream& s) const
{
    std::string solution_value = optimizationtools::solution_value(
            optimizationtools::ObjectiveDirection::Minimize,
            solution.feasible(),
            solution.cost());
    double absolute_optimality_gap = optimizationtools::absolute_optimality_gap(
            optimizationtools::ObjectiveDirection::Minimize,
            solution.feasible(),
            solution.cost(),
            bound);
    double relative_optimality_gap = optimizationtools::relative_optimality_gap(
            optimizationtools::ObjectiveDirection::Minimize,
            solution.feasible(),
            solution.cost(),
            bound);
    double t = info.elapsed_time();
    std::streamsize precision = std::cout.precision();

    info.os()
        << std::setw(12) << std::fixed << std::setprecision(3) << t << std::defaultfloat << std::setprecision(precision)
        << std::setw(12) << solution_value
        << std::setw(12) << bound
        << std::setw(12) << absolute_optimality_gap
        << std::setw(12) << std::fixed << std::setprecision(2) << relative_optimality_gap * 100 << std::defaultfloat << std::setprecision(precision)
        << std::setw(24) << s.str() << std::endl;

    if (!info.output->only_write_at_the_end)
        info.write_json_output();
}

void Output::update_solution(
        const Solution& solution_new,
        const std::stringstream& s,
        optimizationtools::Info& info)
{
    info.lock();

    if (solution_new.is_strictly_better_than(solution)) {
        solution.update(solution_new);
        print(info, s);

        std::string solution_value = optimizationtools::solution_value(
                optimizationtools::ObjectiveDirection::Minimize,
                solution.feasible(),
                solution.cost());
        double t = info.elapsed_time();

        info.output->number_of_solutions++;
        std::string sol_str = "Solution" + std::to_string(info.output->number_of_solutions);
        info.add_to_json(sol_str, "Value", solution_value);
        info.add_to_json(sol_str, "Time", t);
        info.add_to_json(sol_str, "String", s.str());
        if (!info.output->only_write_at_the_end) {
            info.write_json_output();
            solution.write(info.output->certificate_path);
        }
    }

    info.unlock();
}

void Output::update_bound(
        Cost bound_new,
        const std::stringstream& s,
        optimizationtools::Info& info)
{
    if (bound >= bound_new)
        return;

    info.lock();

    if (bound < bound_new) {
        bound = bound_new;
        print(info, s);

        double t = info.elapsed_time();
        info.output->number_of_bounds++;
        std::string sol_str = "Bound" + std::to_string(info.output->number_of_bounds);
        info.add_to_json(sol_str, "Bound", bound);
        info.add_to_json(sol_str, "Time", t);
        info.add_to_json(sol_str, "String", s.str());
        if (!info.output->only_write_at_the_end)
            solution.write(info.output->certificate_path);
    }

    info.unlock();
}

Output& Output::algorithm_end(optimizationtools::Info& info)
{
    std::string solution_value = optimizationtools::solution_value(
            optimizationtools::ObjectiveDirection::Minimize,
            solution.feasible(),
            solution.cost());
    double absolute_optimality_gap = optimizationtools::absolute_optimality_gap(
            optimizationtools::ObjectiveDirection::Minimize,
            solution.feasible(),
            solution.cost(),
            bound);
    double relative_optimality_gap = optimizationtools::relative_optimality_gap(
            optimizationtools::ObjectiveDirection::Minimize,
            solution.feasible(),
            solution.cost(),
            bound);
    time = info.elapsed_time();

    info.add_to_json("Solution", "Value", solution_value);
    info.add_to_json("Bound", "Value", bound);
    info.add_to_json("Solution", "Time", time);
    info.add_to_json("Bound", "Time", time);
    info.os()
        << std::endl
        << "Final statistics" << std::endl
        << "----------------" << std::endl
        << "Value:                         " << solution_value << std::endl
        << "Bound:                         " << bound << std::endl
        << "Absolute optimality gap:       " << absolute_optimality_gap << std::endl
        << "Relative optimality gap (%):   " << relative_optimality_gap * 100 << std::endl
        << "Time (s):                      " << time << std::endl
        ;
    print_statistics(info);
    info.os() << std::endl
        << "Solution" << std::endl
        << "--------" << std::endl ;
    solution.print(info.os(), info.verbosity_level());

    info.write_json_output();
    solution.write(info.output->certificate_path);
    return *this;
}
