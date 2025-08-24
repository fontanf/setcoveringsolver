#include "setcoveringsolver/algorithms/milp.hpp"

#include "setcoveringsolver/algorithm_formatter.hpp"

#include "mathoptsolverscmake/milp.hpp"

using namespace setcoveringsolver;

namespace
{

mathoptsolverscmake::MilpModel create_milp_model(
        const Instance& instance)
{
    int number_of_variables = instance.number_of_sets();
    int number_of_constraints = instance.number_of_elements();
    int number_of_elements = instance.number_of_arcs();

    mathoptsolverscmake::MilpModel model(
            number_of_variables,
            number_of_constraints,
            number_of_elements);

    // Variable and objective.
    model.objective_direction = mathoptsolverscmake::ObjectiveDirection::Minimize;
    for (SetId set_id = 0; set_id < instance.number_of_sets(); ++set_id) {
        const Set& set = instance.set(set_id);
        model.variables_lower_bounds[set_id] = 0;
        model.variables_upper_bounds[set_id] = 1;
        model.variables_types[set_id] = mathoptsolverscmake::VariableType::Binary;
        model.objective_coefficients[set_id] = set.cost;
    }

    // Constraints.
    int milp_element_id = 0;
    int constraints_id = 0;

    // Each element must be covered.
    for (ElementId element_id = 0;
            element_id < instance.number_of_elements();
            ++element_id) {
        const Element& element = instance.element(element_id);
        model.constraints_starts[constraints_id] = milp_element_id;
        for (SetId set_id: element.sets) {
            model.elements_variables[milp_element_id] = set_id;
            model.elements_coefficients[milp_element_id] = 1.0;
            milp_element_id++;
        }
        model.constraints_lower_bounds[constraints_id] = 1;
        constraints_id++;
    }

    return model;
}

Solution retrieve_solution(
        const Instance& instance,
        const std::vector<double>& milp_solution)
{
    Solution solution(instance);
    for (SetId set_id = 0;
            set_id < instance.number_of_sets();
            ++set_id) {
        if (milp_solution[set_id] > 0.5)
            solution.add(set_id);
    }
    return solution;
}

#ifdef CBC_FOUND

class EventHandler: public CbcEventHandler
{

public:

    virtual CbcAction event(CbcEvent which_event);

    EventHandler(
            const Instance& instance,
            const MilpParameters& parameters,
            Output& output,
            AlgorithmFormatter& algorithm_formatter):
        CbcEventHandler(),
        instance_(instance),
        parameters_(parameters),
        output_(output),
        algorithm_formatter_(algorithm_formatter) { }

    virtual ~EventHandler() { }

    EventHandler(const EventHandler &rhs):
        CbcEventHandler(rhs),
        instance_(rhs.instance_),
        parameters_(rhs.parameters_),
        output_(rhs.output_),
        algorithm_formatter_(rhs.algorithm_formatter_) { }

    virtual CbcEventHandler* clone() const { return new EventHandler(*this); }

private:

    const Instance& instance_;
    const MilpParameters& parameters_;
    Output& output_;
    AlgorithmFormatter& algorithm_formatter_;

};

CbcEventHandler::CbcAction EventHandler::event(CbcEvent which_event)
{
    // Not in subtree.
    if ((model_->specialOptions() & 2048) != 0)
        return noAction;
    const CbcModel& cbc_model = *model_;

    int number_of_nodes = mathoptsolverscmake::get_number_of_nodes(cbc_model);

    // Retrieve solution.
    double milp_objective_value = mathoptsolverscmake::get_solution_value(cbc_model);
    if (output_.solution.cost() > milp_objective_value) {
        std::vector<double> milp_solution = mathoptsolverscmake::get_solution(cbc_model);
        Solution solution = retrieve_solution(instance_, milp_solution);
        algorithm_formatter_.update_solution(solution, "node " + std::to_string(number_of_nodes));
    }

    // Retrieve bound.
    double bound = mathoptsolverscmake::get_bound(cbc_model);
    algorithm_formatter_.update_bound(bound, "node " + std::to_string(number_of_nodes));

    // Check end.
    if (parameters_.timer.needs_to_end())
        return stop;
    if (parameters_.goal >= 0
            && output_.solution.objective_value() >= parameters_.goal) {
        return stop;
    }

    return noAction;
}

#endif

#ifdef XPRESS_FOUND

struct XpressCallbackUser
{
    const Instance& instance;
    const MilpParameters& parameters;
    Output& output;
    AlgorithmFormatter& algorithm_formatter;
};

void xpress_callback(
        XPRSprob xpress_model,
        void* user,
        int*)
{
    const XpressCallbackUser& d = *(const XpressCallbackUser*)(user);

    // Retrieve solution.
    double milp_objective_value = mathoptsolverscmake::get_solution_value(xpress_model);
    if (d.output.solution.cost() > milp_objective_value) {
        std::vector<double> milp_solution = mathoptsolverscmake::get_solution(xpress_model);
        Solution solution = retrieve_solution(d.instance, milp_solution);
        d.algorithm_formatter.update_solution(solution, "");
    }

    // Retrieve bound.
    double bound = mathoptsolverscmake::get_bound(xpress_model);
    d.algorithm_formatter.update_bound(bound, "");

    // Check end.
    if (d.parameters.timer.needs_to_end())
        XPRSinterrupt(xpress_model, XPRS_STOP_USER);
    if (d.parameters.goal >= 0
            && d.output.solution.objective_value() >= d.parameters.goal) {
        XPRSinterrupt(xpress_model, XPRS_STOP_USER);
    }
};

#endif

}

const Output setcoveringsolver::milp(
        const Instance& instance,
        const Solution* initial_solution,
        const MilpParameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("MILP (HIGHS)");

    algorithm_formatter.print_header();

    // Reduction.
    if (parameters.reduction_parameters.reduce) {
        return solve_reduced_instance(
                [](
                    const Instance& instance,
                    const MilpParameters& parameters)
                {
                    return milp(
                            instance,
                            nullptr,
                            parameters);
                },
                instance,
                parameters,
                algorithm_formatter,
                output);
    }

    // Initialize Highs.
    Highs highs;
    HighsStatus return_status;

    // Reduce printout.
    return_status = highs.setOptionValue(
            "log_to_console",
            false);

    return_status = highs.setOptionValue(
            "log_file",
            "highs.log");

    mathoptsolverscmake::MilpModel milp_model = create_milp_model(instance);
    std::vector<double> milp_solution;
    double milp_bound = 0;

    if (parameters.solver == mathoptsolverscmake::SolverName::Cbc) {
#ifdef CBC_FOUND
        OsiCbcSolverInterface osi_solver;
        CbcModel cbc_model(osi_solver);
        mathoptsolverscmake::reduce_printout(cbc_model);
        mathoptsolverscmake::set_time_limit(cbc_model, parameters.timer.remaining_time());
        mathoptsolverscmake::load(cbc_model, milp_model);
        EventHandler cbc_event_handler(instance, parameters, output, algorithm_formatter);
        cbc_model.passInEventHandler(&cbc_event_handler);
        mathoptsolverscmake::solve(cbc_model);
        milp_solution = mathoptsolverscmake::get_solution(cbc_model);
        milp_bound = mathoptsolverscmake::get_bound(cbc_model);
#else
        throw std::invalid_argument("");
#endif

    } else if (parameters.solver == mathoptsolverscmake::SolverName::Highs) {
#ifdef HIGHS_FOUND
        Highs highs;
        mathoptsolverscmake::reduce_printout(highs);
        mathoptsolverscmake::set_time_limit(highs, parameters.timer.remaining_time());
        mathoptsolverscmake::set_log_file(highs, "highs.log");
        mathoptsolverscmake::load(highs, milp_model);
        highs.setCallback([
                &instance,
                &parameters,
                &output,
                &algorithm_formatter](
                    const int,
                    const std::string& message,
                    const HighsCallbackOutput* highs_output,
                    HighsCallbackInput* highs_input,
                    void*)
                {
                    if (!highs_output->mip_solution.empty()) {
                        // Retrieve solution.
                        double milp_objective_value = highs_output->mip_primal_bound;
                        if (output.solution.cost() > milp_objective_value) {
                            Solution solution = retrieve_solution(instance, highs_output->mip_solution);
                            algorithm_formatter.update_solution(solution, "node " + std::to_string(highs_output->mip_node_count));
                        }

                        // Retrieve bound.
                        double bound = highs_output->mip_dual_bound;
                        if (bound != std::numeric_limits<double>::infinity())
                            algorithm_formatter.update_bound(bound, "node " + std::to_string(highs_output->mip_node_count));
                    }

                    // Check end.
                    if (parameters.timer.needs_to_end())
                        highs_input->user_interrupt = 1;
                    if (parameters.goal >= 0
                            && output.solution.objective_value() >= parameters.goal) {
                        highs_input->user_interrupt = 1;
                    }
                },
                nullptr);
        HighsStatus highs_status;
        highs_status = highs.startCallback(HighsCallbackType::kCallbackMipImprovingSolution);
        highs_status = highs.startCallback(HighsCallbackType::kCallbackMipInterrupt);
        mathoptsolverscmake::solve(highs);
        milp_solution = mathoptsolverscmake::get_solution(highs);
        milp_bound = mathoptsolverscmake::get_bound(highs);
#else
        throw std::invalid_argument("");
#endif

    } else if (parameters.solver == mathoptsolverscmake::SolverName::Xpress) {
#ifdef XPRESS_FOUND
        XPRSprob xpress_model;
        XPRScreateprob(&xpress_model);
        mathoptsolverscmake::set_time_limit(xpress_model, parameters.timer.remaining_time());
        mathoptsolverscmake::set_log_file(xpress_model, "xpress.log");
        mathoptsolverscmake::load(xpress_model, milp_model);
        //mathoptsolverscmake::write_mps(xpress_model, "kpc.mps");
        XpressCallbackUser xpress_callback_user{instance, parameters, output, algorithm_formatter};
        XPRSaddcbprenode(xpress_model, xpress_callback, (void*)&xpress_callback_user, 0);
        mathoptsolverscmake::solve(xpress_model);
        milp_solution = mathoptsolverscmake::get_solution(xpress_model);
        milp_bound = mathoptsolverscmake::get_bound(xpress_model);
        XPRSdestroyprob(xpress_model);
#else
        throw std::invalid_argument("");
#endif

    } else {
        throw std::invalid_argument("");
    }

    algorithm_formatter.end();
    return output;
}
