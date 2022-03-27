#include "setcoveringsolver/solution.hpp"

#include <iomanip>

using namespace setcoveringsolver;

Solution::Solution(const Instance& instance):
    instance_(instance),
    elements_(instance.number_of_elements(), 0),
    sets_(instance.number_of_sets()),
    component_number_of_elements_(instance.number_of_components(), 0),
    component_costs_(instance.number_of_components(), 0)
{
    for (ElementId e: instance.fixed_elements())
        elements_.set(e, 1);
}

Solution::Solution(const Instance& instance, std::string certificate_path):
    instance_(instance),
    elements_(instance.number_of_elements(), 0),
    sets_(instance.number_of_sets()),
    component_number_of_elements_(instance.number_of_components(), 0),
    component_costs_(instance.number_of_components(), 0)
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

Solution::Solution(const Solution& solution):
    instance_(solution.instance_),
    elements_(solution.elements_),
    sets_(solution.sets_),
    component_number_of_elements_(solution.component_number_of_elements_),
    component_costs_(solution.component_costs_),
    cost_(solution.cost_)
{ }

Solution& Solution::operator=(const Solution& solution)
{
    if (this != &solution) {
        assert(&instance_ == &solution.instance_);
        elements_                     = solution.elements_;
        sets_                         = solution.sets_;
        component_number_of_elements_ = solution.component_number_of_elements_;
        component_costs_              = solution.component_costs_;
        cost_                         = solution.cost_;
    }
    return *this;
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
    VER(info,
               std::setw(12) << "T (s)"
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
            << std::endl);
    print(info, std::stringstream(""));
}

void Output::print(
        optimizationtools::Info& info,
        const std::stringstream& s) const
{
    double gap = (lower_bound == 0)?
        std::numeric_limits<double>::infinity():
        (double)(upper_bound() - lower_bound) / lower_bound * 100;
    double t = round(info.elapsed_time() * 10000) / 10000;

    VER(info,
               std::setw(12) << t
            << std::setw(12) << upper_bound()
            << std::setw(12) << lower_bound
            << std::setw(12) << upper_bound() - lower_bound
            << std::setw(12) << gap
            << std::setw(24) << s.str() << std::endl);

    if (!info.output->only_write_at_the_end)
        info.write_json_output();
}

void Output::update_solution(
        const Solution& solution_new,
        ComponentId c,
        const std::stringstream& s,
        optimizationtools::Info& info)
{
    info.output->mutex_solutions.lock();

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
        double t = round(info.elapsed_time() * 10000) / 10000;
        std::string sol_str = "Solution" + std::to_string(info.output->number_of_solutions);
        PUT(info, sol_str, "Value", solution.cost());
        PUT(info, sol_str, "Time", t);
        PUT(info, sol_str, "String", s.str());
        if (!info.output->only_write_at_the_end) {
            info.write_json_output();
            solution.write(info.output->certificate_path);
        }
    }

    info.output->mutex_solutions.unlock();
}

void Output::update_lower_bound(
        Cost lower_bound_new,
        const std::stringstream& s,
        optimizationtools::Info& info)
{
    if (lower_bound >= lower_bound_new)
        return;

    info.output->mutex_solutions.lock();

    if (lower_bound < lower_bound_new) {
        lower_bound = lower_bound_new;
        print(info, s);

        info.output->number_of_bounds++;
        double t = round(info.elapsed_time() * 10000) / 10000;
        std::string sol_str = "Bound" + std::to_string(info.output->number_of_bounds);
        PUT(info, sol_str, "Bound", lower_bound);
        PUT(info, sol_str, "Time", t);
        PUT(info, sol_str, "String", s.str());
        if (!info.output->only_write_at_the_end)
            solution.write(info.output->certificate_path);
    }

    info.output->mutex_solutions.unlock();
}

Output& Output::algorithm_end(optimizationtools::Info& info)
{
    double t = round(info.elapsed_time() * 10000) / 10000;
    double gap = (lower_bound == 0)?
        std::numeric_limits<double>::infinity():
        (double)(upper_bound() - lower_bound) / lower_bound * 100;
    PUT(info, "Solution", "Value", upper_bound());
    PUT(info, "Bound", "Value", lower_bound);
    PUT(info, "Solution", "Time", t);
    PUT(info, "Bound", "Time", t);
    VER(info,
            std::endl
            << "Final statistics" << std::endl
            << "----------------" << std::endl
            << "Value:                 " << upper_bound() << std::endl
            << "Bound:                 " << lower_bound << std::endl
            << "Gap:                   " << upper_bound() - lower_bound << std::endl
            << "Gap (%):               " << gap << std::endl
            << "Time (s):              " << t << std::endl);

    info.write_json_output();
    solution.write(info.output->certificate_path);
    return *this;
}

Cost setcoveringsolver::algorithm_end(
        Cost lower_bound,
        optimizationtools::Info& info)
{
    double t = round(info.elapsed_time() * 10000) / 10000;
    PUT(info, "Bound", "Value", lower_bound);
    PUT(info, "Bound", "Time", t);
    VER(info, "---" << std::endl
            << "Bound:                 " << lower_bound << std::endl
            << "Time (s):              " << t << std::endl);

    info.write_json_output();
    return lower_bound;
}

