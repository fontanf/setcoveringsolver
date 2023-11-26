#if CBC_FOUND

#include "setcoveringsolver/setcovering/algorithms/milp_cbc.hpp"

using namespace setcoveringsolver::setcovering;

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// Callback ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class EventHandler: public CbcEventHandler
{

public:

    virtual CbcAction event(CbcEvent which_event);

    EventHandler(
            const Instance& instance,
            MilpCbcOptionalParameters& parameters,
            Output& output):
        CbcEventHandler(),
        instance_(instance),
        parameters_(parameters),
        output_(output) { }

    EventHandler(
            CbcModel* model,
            const Instance& instance,
            MilpCbcOptionalParameters& parameters,
            Output& output):
        CbcEventHandler(model),
        instance_(instance),
        parameters_(parameters),
        output_(output) { }

    virtual ~EventHandler() { }

    EventHandler(const EventHandler &rhs):
        CbcEventHandler(rhs),
        instance_(rhs.instance_),
        parameters_(rhs.parameters_),
        output_(rhs.output_) { }

    EventHandler &operator=(const EventHandler &rhs)
    {
        if (this != &rhs) {
            CbcEventHandler::operator=(rhs);
            //this->instance_  = rhs.instance_;
            this->parameters_ = rhs.parameters_;
            this->output_ = rhs.output_;
        }
        return *this;
    }

    virtual CbcEventHandler* clone() const { return new EventHandler(*this); }

private:

    const Instance& instance_;
    MilpCbcOptionalParameters& parameters_;
    Output& output_;

};

CbcEventHandler::CbcAction EventHandler::event(CbcEvent which_event)
{
    if ((model_->specialOptions() & 2048) != 0) // not in subtree
        return noAction;

    Cost lb = std::ceil(model_->getBestPossibleObjValue() - FFOT_TOL);
    output_.update_bound(lb, std::stringstream(""), parameters_.info);

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
        output_.update_solution(
                solution,
                std::stringstream(""),
                parameters_.info);
    }

    return noAction;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CoinLP::CoinLP(const Instance& instance)
{
    // Variables
    int number_of_columns = instance.number_of_sets();
    column_lower_bounds.resize(number_of_columns, 0);
    column_upper_bounds.resize(number_of_columns, 1);

    // Objective
    objective = std::vector<double>(number_of_columns);
    for (SetId set_id = 0; set_id < instance.number_of_sets(); ++set_id)
        objective[set_id] = instance.set(set_id).cost;

    // Constraints
    int number_of_rows = 0; // will be increased each time we add a constraint
    std::vector<CoinBigIndex> row_starts;
    std::vector<int> number_of_elements_in_rows;
    std::vector<int> element_columns;
    std::vector<double> elements;

    // Every element needs to be covered
    for (ElementId element_id = 0;
            element_id < instance.number_of_elements();
            ++element_id) {
        // Initialize new row
        row_starts.push_back(elements.size());
        number_of_elements_in_rows.push_back(0);
        number_of_rows++;
        // Add sets
        for (SetId set_id: instance.element(element_id).sets) {
            elements.push_back(1);
            element_columns.push_back(set_id);
            number_of_elements_in_rows.back()++;
        }
        // Add row bounds
        row_lower_bounds.push_back(1);
        row_upper_bounds.push_back(instance.number_of_sets());
    }

    // Create matrix
    row_starts.push_back(elements.size());
    matrix = CoinPackedMatrix(
            false,
            number_of_columns,
            number_of_rows,
            elements.size(),
            elements.data(),
            element_columns.data(),
            row_starts.data(),
            number_of_elements_in_rows.data());
}

const Output setcoveringsolver::setcovering::milp_cbc(
        const Instance& original_instance,
        MilpCbcOptionalParameters parameters)
{
    init_display(original_instance, parameters.info);
    parameters.info.output()
            << "Algorithm" << std::endl
            << "---------" << std::endl
            << "MILP (CBC)" << std::endl
            << std::endl;

    // Reduction.
    std::unique_ptr<Instance> reduced_instance = nullptr;
    if (parameters.reduction_parameters.reduce) {
        reduced_instance = std::unique_ptr<Instance>(
                new Instance(
                    original_instance.reduce(
                        parameters.reduction_parameters)));
        parameters.info.output()
            << "Reduced instance" << std::endl
            << "----------------" << std::endl
            << InstanceFormatter{*reduced_instance, parameters.info.output().verbosity_level()}
            << std::endl;
    }
    const Instance& instance = (reduced_instance == nullptr)? original_instance: *reduced_instance;

    Output output(original_instance, parameters.info);

    if (instance.number_of_sets() == 0)
        return output.algorithm_end(parameters.info);

    CoinLP problem(instance);

    OsiCbcSolverInterface solver1;

    // Reduce printout.
    solver1.getModelPtr()->setLogLevel(0);
    solver1.messageHandler()->setLogLevel(0);

    // Load problem.
    solver1.loadProblem(
            problem.matrix,
            problem.column_lower_bounds.data(),
            problem.column_upper_bounds.data(),
            problem.objective.data(),
            problem.row_lower_bounds.data(),
            problem.row_upper_bounds.data());

    // Mark integer.
    for (SetId set_id = 0; set_id < instance.number_of_sets(); ++set_id)
        solver1.setInteger(set_id);

    // Pass data and solver to CbcModel.
    CbcModel model(solver1);

    // Callback.
    EventHandler event_handler(instance, parameters, output);
    model.passInEventHandler(&event_handler);

    // Reduce printout.
    model.setLogLevel(0);
    model.solver()->setHintParam(OsiDoReducePrint, true, OsiHintTry);

    // Set time limit.
    model.setMaximumSeconds(parameters.info.remaining_time());

    // Add initial solution.
    std::vector<double> sol_init(instance.number_of_sets(), 0);
    if (parameters.initial_solution != NULL
            && parameters.initial_solution->feasible()) {
        for (SetId set_id = 0; set_id < instance.number_of_sets(); ++set_id)
            if (parameters.initial_solution->contains(set_id))
                sol_init[set_id] = 1;
        model.setBestSolution(
                sol_init.data(),
                instance.number_of_sets(),
                parameters.initial_solution->cost());
    }

    // Do complete search.
    model.branchAndBound();

    if (model.isProvenInfeasible()) {  // Infeasible.
        // Update dual bound.
        output.update_bound(
                instance.total_cost(),
                std::stringstream(""),
                parameters.info);
    } else if (model.isProvenOptimal()) {  // Optimal
        // Update primal solution.
        if (!output.solution.feasible()
                || output.solution.cost() > model.getObjValue() + 0.5) {
            const double* solution_cbc = model.solver()->getColSolution();
            Solution solution(instance);
            for (SetId set_id = 0; set_id < instance.number_of_sets(); ++set_id)
                if (solution_cbc[set_id] > 0.5)
                        solution.add(set_id);
            output.update_solution(
                    solution,
                    std::stringstream(""),
                    parameters.info);
        }
        // Update dual bound.
        output.update_bound(
                output.solution.cost(),
                std::stringstream(""),
                parameters.info);
    } else if (model.bestSolution() != NULL) {  // Feasible solution found.
        // Update primal solution.
        if (!output.solution.feasible()
                || output.solution.cost() > model.getObjValue() + 0.5) {
            const double* solution_cbc = model.solver()->getColSolution();
            Solution solution(instance);
            for (SetId set_id = 0; set_id < instance.number_of_sets(); ++set_id)
                if (solution_cbc[set_id] > 0.5)
                    solution.add(set_id);
            output.update_solution(
                    solution,
                    std::stringstream(""),
                    parameters.info);
        }
        // Update dual bound.
        Cost lb = std::ceil(model.getBestPossibleObjValue() - FFOT_TOL);
        output.update_bound(lb, std::stringstream(""), parameters.info);
    } else {   // No feasible solution found.
        // Update dual bound.
        Cost lb = std::ceil(model.getBestPossibleObjValue() - FFOT_TOL);
        output.update_bound(lb, std::stringstream(""), parameters.info);
    }

    return output.algorithm_end(parameters.info);
}

#endif

