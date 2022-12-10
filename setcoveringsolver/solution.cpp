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
    for (ElementId e: instance.fixed_elements())
        elements_.set(e, 1);
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

    for (ElementId e: instance.fixed_elements())
        elements_.set(e, 1);

    SetId number_of_sets;
    SetId s;
    file >> number_of_sets;
    for (SetPos s_pos = 0; s_pos < number_of_sets; ++s_pos) {
        file >> s;
        add(s);
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
    for (SetId s = 0; s < instance().number_of_sets(); ++s)
        if (contains(s))
            file << s << " ";
    file.close();
}

std::ostream& setcoveringsolver::operator<<(std::ostream& os, const Solution& solution)
{
    os << solution.number_of_sets() << std::endl;
    for (SetId s = 0; s < solution.instance().number_of_sets(); ++s)
        if (solution.contains(s))
            os << s << " ";
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
        ComponentId c,
        const std::stringstream& s,
        optimizationtools::Info& info)
{
    info.lock();

    bool ok = false;
    if (c == -1) {
        if (solution_new.feasible() && (!solution.feasible() || solution.cost() > solution_new.cost()))
            ok = true;
    } else {
        if (solution_new.feasible(c) && solution.cost(c) > solution_new.cost(c))
            ok = true;
    }

    if (ok) {
        if (c == -1) {
            for (SetId s = 0; s < solution.instance().number_of_sets(); ++s) {
                if (solution.contains(s) && !solution_new.contains(s)) {
                    solution.remove(s);
                } else if (!solution.contains(s) && solution_new.contains(s)) {
                    solution.add(s);
                }
            }
        } else {
            for (SetId s: solution.instance().component(c).sets) {
                if (solution.contains(s) && !solution_new.contains(s)) {
                    solution.remove(s);
                } else if (!solution.contains(s) && solution_new.contains(s)) {
                    solution.add(s);
                }
            }
        }
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

