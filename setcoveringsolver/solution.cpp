#include "setcoveringsolver/solution.hpp"

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
    double gap = (lower_bound == 0)?
        std::numeric_limits<double>::infinity():
        (double)(upper_bound() - lower_bound) / lower_bound * 100;

    double t = info.elapsed_time();
    std::streamsize precision = std::cout.precision();
    info.os()
        << std::setw(12) << std::fixed << std::setprecision(3) << t << std::defaultfloat << std::setprecision(precision)
        << std::setw(12) << upper_bound()
        << std::setw(12) << lower_bound
        << std::setw(12) << upper_bound() - lower_bound
        << std::setw(12) << gap
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

        info.output->number_of_solutions++;
        double t = info.elapsed_time();
        std::string sol_str = "Solution" + std::to_string(info.output->number_of_solutions);
        info.add_to_json(sol_str, "Value", solution.cost());
        info.add_to_json(sol_str, "Time", t);
        info.add_to_json(sol_str, "String", s.str());
        if (!info.output->only_write_at_the_end) {
            info.write_json_output();
            solution.write(info.output->certificate_path);
        }
    }

    info.unlock();
}

void Output::update_lower_bound(
        Cost lower_bound_new,
        const std::stringstream& s,
        optimizationtools::Info& info)
{
    if (lower_bound >= lower_bound_new)
        return;

    info.lock();

    if (lower_bound < lower_bound_new) {
        lower_bound = lower_bound_new;
        print(info, s);

        info.output->number_of_bounds++;
        double t = info.elapsed_time();
        std::string sol_str = "Bound" + std::to_string(info.output->number_of_bounds);
        info.add_to_json(sol_str, "Bound", lower_bound);
        info.add_to_json(sol_str, "Time", t);
        info.add_to_json(sol_str, "String", s.str());
        if (!info.output->only_write_at_the_end)
            solution.write(info.output->certificate_path);
    }

    info.unlock();
}

Output& Output::algorithm_end(optimizationtools::Info& info)
{
    double t = info.elapsed_time();
    double gap = (lower_bound == 0)?
        std::numeric_limits<double>::infinity():
        (double)(upper_bound() - lower_bound) / lower_bound * 100;
    info.add_to_json("Solution", "Value", upper_bound());
    info.add_to_json("Bound", "Value", lower_bound);
    info.add_to_json("Solution", "Time", t);
    info.add_to_json("Bound", "Time", t);
    info.os()
            << std::endl
            << "Final statistics" << std::endl
            << "----------------" << std::endl
            << "Value:                         " << upper_bound() << std::endl
            << "Bound:                         " << lower_bound << std::endl
            << "Gap:                           " << upper_bound() - lower_bound << std::endl
            << "Gap (%):                       " << gap << std::endl
            << "Time (s):                      " << t << std::endl;

    info.write_json_output();
    solution.write(info.output->certificate_path);
    return *this;
}

Cost setcoveringsolver::algorithm_end(
        Cost lower_bound,
        optimizationtools::Info& info)
{
    double t = info.elapsed_time();
    info.add_to_json("Bound", "Value", lower_bound);
    info.add_to_json("Bound", "Time", t);
    info.os() << "---" << std::endl
            << "Bound:                         " << lower_bound << std::endl
            << "Time (s):                      " << t << std::endl;

    info.write_json_output();
    return lower_bound;
}

