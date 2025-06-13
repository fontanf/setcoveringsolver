#include "setcoveringsolver/algorithms/milp_cbc.hpp"

#include "setcoveringsolver/algorithm_formatter.hpp"

using namespace setcoveringsolver;

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// Callback ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

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

struct CoinLP
{
    CoinLP(const Instance& instance);

    std::vector<double> column_lower_bounds;
    std::vector<double> column_upper_bounds;
    std::vector<double> objective;

    std::vector<double> row_lower_bounds;
    std::vector<double> row_upper_bounds;
    CoinPackedMatrix matrix;
};

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

const Output setcoveringsolver::milp_cbc(
        const Instance& instance,
        const MilpCbcParameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("MILP (CBC)");

    // Reduction.
    if (parameters.reduction_parameters.reduce)
        return solve_reduced_instance(milp_cbc, instance, parameters, algorithm_formatter, output);

    algorithm_formatter.print_header();

    CoinLP problem(instance);

    OsiCbcSolverInterface solver1;

    // Reduce printout.
    //solver1.getModelPtr()->setLogLevel(0);
    //solver1.messageHandler()->setLogLevel(0);

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
    EventHandler event_handler(instance, algorithm_formatter, output);
    model.passInEventHandler(&event_handler);

    // Reduce printout.
    //model.setLogLevel(0);
    //model.solver()->setHintParam(OsiDoReducePrint, true, OsiHintTry);

    // Set time limit.
    model.setMaximumSeconds(parameters.timer.remaining_time());

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

    if (model.isProvenInfeasible()) {
        // Infeasible.

        // Update dual bound.
        algorithm_formatter.update_bound(instance.total_cost(), "");

    } else if (model.isProvenOptimal()) {
        // Optimal.

        // Update primal solution.
        if (!output.solution.feasible()
                || output.solution.cost() > model.getObjValue() + 0.5) {
            const double* solution_cbc = model.solver()->getColSolution();
            Solution solution(instance);
            for (SetId set_id = 0; set_id < instance.number_of_sets(); ++set_id)
                if (solution_cbc[set_id] > 0.5)
                        solution.add(set_id);
            algorithm_formatter.update_solution(solution, "");
        }

        // Update dual bound.
        algorithm_formatter.update_bound(output.solution.cost(), "");

    } else if (model.bestSolution() != NULL) {
        // Feasible solution found.

        // Update primal solution.
        if (!output.solution.feasible()
                || output.solution.cost() > model.getObjValue() + 0.5) {
            const double* solution_cbc = model.solver()->getColSolution();
            Solution solution(instance);
            for (SetId set_id = 0; set_id < instance.number_of_sets(); ++set_id)
                if (solution_cbc[set_id] > 0.5)
                    solution.add(set_id);
            algorithm_formatter.update_solution(solution, "");
        }

        // Update dual bound.
        Cost bound = std::ceil(model.getBestPossibleObjValue() - FFOT_TOL);
        algorithm_formatter.update_bound(bound, "");

    } else {
        // No feasible solution found.

        // Update dual bound.
        Cost bound = std::ceil(model.getBestPossibleObjValue() - FFOT_TOL);
        algorithm_formatter.update_bound(bound, "");
    }

    algorithm_formatter.end();
    return output;
}
