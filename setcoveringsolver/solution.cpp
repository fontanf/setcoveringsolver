#include "setcoveringsolver/solution.hpp"

#include <iomanip>

using namespace setcoveringsolver;

Solution::Solution(const Instance& instance):
    instance_(instance),
    elements_(instance.element_number(), 0),
    sets_(instance.set_number()),
    component_element_numbers_(instance.component_number(), 0),
    component_costs_(instance.component_number(), 0),
    penalties_(instance.element_number(), 1),
    penalty_(instance.element_number())
{
    for (ElementId e: instance.fixed_elements()) {
        elements_.set(e, 1);
        penalty_--;
    }
}

Solution::Solution(const Instance& instance, std::string filepath):
    instance_(instance),
    elements_(instance.element_number(), 0),
    sets_(instance.set_number()),
    component_element_numbers_(instance.component_number(), 0),
    component_costs_(instance.component_number(), 0),
    penalties_(instance.element_number(), 1),
    penalty_(instance.element_number())
{
    if (filepath.empty())
        return;
    std::ifstream file(filepath);
    if (!file.good()) {
        std::cerr << "\033[31m" << "ERROR, unable to open file \"" << filepath << "\"" << "\033[0m" << std::endl;
        return;
    }

    for (ElementId e: instance.fixed_elements()) {
        elements_.set(e, 1);
        penalty_--;
    }

    SetId set_number;
    SetId s;
    file >> set_number;
    for (SetPos s_pos = 0; s_pos < set_number; ++s_pos) {
        file >> s;
        add(s);
    }
}

Solution::Solution(const Solution& solution):
    instance_(solution.instance_),
    elements_(solution.elements_),
    sets_(solution.sets_),
    component_element_numbers_(solution.component_element_numbers_),
    component_costs_(solution.component_costs_),
    penalties_(solution.penalties_),
    cost_(solution.cost_),
    penalty_(solution.penalty_)
{ }

Solution& Solution::operator=(const Solution& solution)
{
    if (this != &solution) {
        assert(&instance_ == &solution.instance_);
        elements_                  = solution.elements_;
        sets_                      = solution.sets_;
        component_element_numbers_ = solution.component_element_numbers_;
        component_costs_           = solution.component_costs_;
        penalties_                 = solution.penalties_;
        cost_                      = solution.cost_;
        penalty_                   = solution.penalty_;
    }
    return *this;
}

void Solution::set_penalty(ElementId e, Cost p)
{
    if (covers(e) == 0)
        penalty_ -= penalties_[e];
    penalties_[e] = p;
    if (covers(e) == 0)
        penalty_ += penalties_[e];
}

void Solution::increment_penalty(ElementId e, Cost p)
{
    penalties_[e] += p;
    if (covers(e) == 0)
        penalty_ += p;
}

void Solution::write(std::string filepath)
{
    if (filepath.empty())
        return;
    std::ofstream cert(filepath);
    if (!cert.good()) {
        std::cerr << "\033[31m" << "ERROR, unable to open file \"" << filepath << "\"" << "\033[0m" << std::endl;
        assert(false);
        return;
    }

    cert << set_number() << std::endl;
    for (SetId s = 0; s < instance().set_number(); ++s)
        if (contains(s))
            cert << s << " ";
    cert.close();
}

std::ostream& setcoveringsolver::operator<<(std::ostream& os, const Solution& solution)
{
    os << solution.set_number() << std::endl;
    for (SetId s = 0; s < solution.instance().set_number(); ++s)
        if (solution.contains(s))
            os << s << " ";
    return os;
}

/*********************************** Output ***********************************/

Output::Output(const Instance& instance, Info& info): solution(instance)
{
    VER(info, std::left << std::setw(12) << "T (s)");
    VER(info, std::left << std::setw(12) << "UB");
    VER(info, std::left << std::setw(12) << "LB");
    VER(info, std::left << std::setw(12) << "GAP");
    VER(info, std::left << std::setw(12) << "GAP (%)");
    VER(info, "");
    VER(info, std::endl);
    print(info, std::stringstream(""));
}

void Output::print(Info& info, const std::stringstream& s) const
{
    double gap = (lower_bound == 0)?
        std::numeric_limits<double>::infinity():
        (double)(upper_bound() - lower_bound) / lower_bound * 100;
    double t = round(info.elapsed_time() * 10000) / 10000;

    VER(info, std::left << std::setw(12) << t);
    VER(info, std::left << std::setw(12) << upper_bound());
    VER(info, std::left << std::setw(12) << lower_bound);
    VER(info, std::left << std::setw(12) << upper_bound() - lower_bound);
    VER(info, std::left << std::setw(12) << gap);
    VER(info, s.str() << std::endl);

    if (!info.output->onlywriteattheend)
        info.write_ini();
}

void Output::update_solution(
        const Solution& solution_new,
        ComponentId c,
        const std::stringstream& s,
        Info& info)
{
    info.output->mutex_sol.lock();

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
            for (SetId s = 0; s < solution.instance().set_number(); ++s) {
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

        info.output->sol_number++;
        double t = round(info.elapsed_time() * 10000) / 10000;
        std::string sol_str = "Solution" + std::to_string(info.output->sol_number);
        PUT(info, sol_str, "Value", solution.cost());
        PUT(info, sol_str, "Time", t);
        PUT(info, sol_str, "String", s.str());
        if (!info.output->onlywriteattheend) {
            info.write_ini();
            solution.write(info.output->certfile);
        }
    }

    info.output->mutex_sol.unlock();
}

void Output::update_lower_bound(Cost lower_bound_new, const std::stringstream& s, Info& info)
{
    if (lower_bound >= lower_bound_new)
        return;

    info.output->mutex_sol.lock();

    if (lower_bound < lower_bound_new) {
        lower_bound = lower_bound_new;
        print(info, s);

        info.output->bnd_number++;
        double t = round(info.elapsed_time() * 10000) / 10000;
        std::string sol_str = "Bound" + std::to_string(info.output->bnd_number);
        PUT(info, sol_str, "Bound", lower_bound);
        PUT(info, sol_str, "Time", t);
        PUT(info, sol_str, "String", s.str());
        if (!info.output->onlywriteattheend)
            solution.write(info.output->certfile);
    }

    info.output->mutex_sol.unlock();
}

Output& Output::algorithm_end(Info& info)
{
    double t = round(info.elapsed_time() * 10000) / 10000;
    double gap = (lower_bound == 0)?
        std::numeric_limits<double>::infinity():
        (double)(upper_bound() - lower_bound) / lower_bound * 100;
    PUT(info, "Solution", "Value", upper_bound());
    PUT(info, "Bound", "Value", lower_bound);
    PUT(info, "Solution", "Time", t);
    PUT(info, "Bound", "Time", t);
    VER(info, "---" << std::endl
            << "Value: " << upper_bound() << std::endl
            << "Bound: " << lower_bound << std::endl
            << "Gap: " << upper_bound() - lower_bound << std::endl
            << "Gap (%): " << gap << std::endl
            << "Time (s): " << t << std::endl);

    info.write_ini();
    solution.write(info.output->certfile);
    return *this;
}

Cost setcoveringsolver::algorithm_end(Cost lower_bound, Info& info)
{
    double t = round(info.elapsed_time() * 10000) / 10000;
    PUT(info, "Bound", "Value", lower_bound);
    PUT(info, "Bound", "Time", t);
    VER(info, "---" << std::endl
            << "Bound: " << lower_bound << std::endl
            << "Time (s): " << t << std::endl);

    info.write_ini();
    return lower_bound;
}

