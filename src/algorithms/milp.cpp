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

}

#ifdef CBC_FOUND

namespace
{

class EventHandler: public CbcEventHandler
{

public:

    virtual CbcAction event(CbcEvent which_event);

    EventHandler(
            const Instance& instance,
            AlgorithmFormatter& algorithm_formatter,
            Output& output):
        CbcEventHandler(),
        instance_(instance),
        algorithm_formatter_(algorithm_formatter),
        output_(output) { }

    EventHandler(
            CbcModel* model,
            const Instance& instance,
            AlgorithmFormatter& algorithm_formatter,
            Output& output):
        CbcEventHandler(model),
        instance_(instance),
        algorithm_formatter_(algorithm_formatter),
        output_(output) { }

    virtual ~EventHandler() { }

    EventHandler(const EventHandler &rhs):
        CbcEventHandler(rhs),
        instance_(rhs.instance_),
        algorithm_formatter_(rhs.algorithm_formatter_),
        output_(rhs.output_) { }

    EventHandler &operator=(const EventHandler &rhs)
    {
        if (this != &rhs) {
            CbcEventHandler::operator=(rhs);
            //this->instance_  = rhs.instance_;
            //this->parameters_ = rhs.parameters_;
            //this->algorithm_formatter = rhs.algorithm_formatter;
            this->output_ = rhs.output_;
        }
        return *this;
    }

    virtual CbcEventHandler* clone() const { return new EventHandler(*this); }

private:

    const Instance& instance_;
    AlgorithmFormatter& algorithm_formatter_;
    Output& output_;

};

CbcEventHandler::CbcAction EventHandler::event(CbcEvent which_event)
{
    if ((model_->specialOptions() & 2048) != 0) // not in subtree
        return noAction;

    Cost bound = std::ceil(model_->getBestPossibleObjValue() - FFOT_TOL);
    algorithm_formatter_.update_bound(bound, "");

    if ((which_event != solution && which_event != heuristicSolution)) // no solution found
        return noAction;

    OsiSolverInterface* origSolver = model_->solver();
    const OsiSolverInterface* pps = model_->postProcessedSolver(1);
    const OsiSolverInterface* solver = pps? pps: origSolver;

    if (!output_.solution.feasible() || output_.solution.cost() > solver->getObjValue() + 0.5) {
        const double* solution_cbc = solver->getColSolution();
        Solution solution(instance_);
        for (SetId set_id = 0; set_id < instance_.number_of_sets(); ++set_id)
            if (solution_cbc[set_id] > 0.5)
                solution.add(set_id);
        algorithm_formatter_.update_solution(solution, "");
    }

    return noAction;
}

}

const Output setcoveringsolver::milp_cbc(
        const Instance& instance,
        const Solution* initial_solution,
        const Parameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("MILP (CBC)");

    // Reduction.
    if (parameters.reduction_parameters.reduce) {
        return solve_reduced_instance(
                [](
                    const Instance& instance,
                    const Parameters& parameters)
                {
                    return milp_cbc(
                            instance,
                            nullptr,
                            parameters);
                },
                instance,
                parameters,
                algorithm_formatter,
                output);
    }

    algorithm_formatter.print_header();

    mathoptsolverscmake::MilpModel model = create_milp_model(instance);

    OsiCbcSolverInterface cbc_model;

    // Reduce printout.
    cbc_model.getModelPtr()->setLogLevel(0);
    cbc_model.messageHandler()->setLogLevel(0);
    cbc_model.setHintParam(OsiDoReducePrint, true, OsiHintTry);

    mathoptsolverscmake::cbc::load(cbc_model, model);

    // Pass data and solver to CbcModel.
    CbcModel cbc_model_2(cbc_model);

    // Reduce printout.
    cbc_model_2.setLogLevel(0);

    // Callback.
    EventHandler event_handler(instance, algorithm_formatter, output);
    cbc_model_2.passInEventHandler(&event_handler);

    // Set time limit.
    cbc_model_2.setMaximumSeconds(parameters.timer.remaining_time());

    // Add initial solution.
    std::vector<double> sol_init(instance.number_of_sets(), 0);
    if (initial_solution != NULL
            && initial_solution->feasible()) {
        for (SetId set_id = 0; set_id < instance.number_of_sets(); ++set_id)
            if (initial_solution->contains(set_id))
                sol_init[set_id] = 1;
        cbc_model_2.setBestSolution(
                sol_init.data(),
                instance.number_of_sets(),
                initial_solution->cost());
    }

    // Do complete search.
    cbc_model_2.branchAndBound();

    if (cbc_model_2.isProvenInfeasible()) {
        // Infeasible.

        // Update dual bound.
        algorithm_formatter.update_bound(instance.total_cost(), "");

    } else {
        if (cbc_model_2.bestSolution() != NULL) {
            // Feasible solution found.

            // Update primal solution.
            if (!output.solution.feasible()
                    || output.solution.cost() > cbc_model_2.getObjValue() + 0.5) {
                const double* solution_cbc = cbc_model_2.solver()->getColSolution();
                Solution solution(instance);
                for (SetId set_id = 0; set_id < instance.number_of_sets(); ++set_id)
                    if (solution_cbc[set_id] > 0.5)
                        solution.add(set_id);
                algorithm_formatter.update_solution(solution, "");
            }
        }

        // Update dual bound.
        Cost bound = std::ceil(cbc_model_2.getBestPossibleObjValue() - FFOT_TOL);
        algorithm_formatter.update_bound(bound, "");
    }

    algorithm_formatter.end();
    return output;
}

#endif

#ifdef HIGHS_FOUND

const Output setcoveringsolver::milp_highs(
        const Instance& instance,
        const Solution* initial_solution,
        const Parameters& parameters)
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
                    const Parameters& parameters)
                {
                    return milp_highs(
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

    // Set up model.
    mathoptsolverscmake::MilpModel model = create_milp_model(instance);
    mathoptsolverscmake::highs::load(highs, model);

    // Solve.
    return_status = highs.run();

    const HighsModelStatus& highs_model_status = highs.getModelStatus();
    const HighsInfo& highs_info = highs.getInfo();

    // Retrieve solution.
    std::vector<double> highs_solution = highs.getSolution().col_value;
    Solution solution(instance);
    for (SetId set_id = 0;
            set_id < instance.number_of_sets();
            ++set_id) {
        if (highs_solution[set_id] > 0.5)
            solution.add(set_id);
    }
    algorithm_formatter.update_solution(solution, "");

    // Retrieve bound.
    double bound = highs_info.mip_dual_bound;
    algorithm_formatter.update_bound(bound, "");

    algorithm_formatter.end();
    return output;
}

#endif
