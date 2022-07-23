#if COINOR_FOUND

#include "setcoveringsolver/algorithms/milp_cbc.hpp"

using namespace setcoveringsolver;

MilpCbcOutput& MilpCbcOutput::algorithm_end(optimizationtools::Info& info)
{
    //FFOT_PUT(info, "Algorithm", "Iterations", it);
    Output::algorithm_end(info);
    //FFOT_VER(info, "Iterations: " << it << std::endl);
    return *this;
}

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
            MilpCbcOutput& output):
        CbcEventHandler(),
        instance_(instance),
        parameters_(parameters),
        output_(output) { }

    EventHandler(
            CbcModel* model,
            const Instance& instance,
            MilpCbcOptionalParameters& parameters,
            MilpCbcOutput& output):
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
    MilpCbcOutput& output_;

};

CbcEventHandler::CbcAction EventHandler::event(CbcEvent which_event)
{
    if ((model_->specialOptions() & 2048) != 0) // not in subtree
        return noAction;

    Cost lb = std::ceil(model_->getBestPossibleObjValue() - FFOT_TOL);
    output_.update_lower_bound(lb, std::stringstream(""), parameters_.info);

    if ((which_event != solution && which_event != heuristicSolution)) // no solution found
        return noAction;

    OsiSolverInterface* origSolver = model_->solver();
    const OsiSolverInterface* pps = model_->postProcessedSolver(1);
    const OsiSolverInterface* solver = pps? pps: origSolver;

    if (!output_.solution.feasible() || output_.solution.cost() > solver->getObjValue() + 0.5) {
        const double* solution_cbc = solver->getColSolution();
        Solution solution(instance_);
        for (SetId s = 0; s < instance_.number_of_sets(); ++s)
            if (solution_cbc[s] > 0.5)
                solution.add(s);
        output_.update_solution(solution, std::stringstream(""), parameters_.info);
    }

    return noAction;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CoinLP::CoinLP(const Instance& instance)
{
    SetId n = instance.number_of_sets();
    ElementId m = instance.number_of_elements();

    // Variables
    int number_of_columns = n;
    column_lower_bounds.resize(number_of_columns, 0);
    column_upper_bounds.resize(number_of_columns, 1);

    // Objective
    objective = std::vector<double>(number_of_columns);
    for (SetId s = 0; s < n; ++s)
        objective[s] = instance.set(s).cost;

    // Constraints
    int number_of_rows = 0; // will be increased each time we add a constraint
    std::vector<CoinBigIndex> row_starts;
    std::vector<int> number_of_elements_in_rows;
    std::vector<int> element_columns;
    std::vector<double> elements;

    // Every element needs to be covered
    for (ElementId e = 0; e < m; ++e) {
        // Initialize new row
        row_starts.push_back(elements.size());
        number_of_elements_in_rows.push_back(0);
        number_of_rows++;
        // Add sets
        for (SetId s: instance.element(e).sets) {
            elements.push_back(1);
            element_columns.push_back(s);
            number_of_elements_in_rows.back()++;
        }
        // Add row bounds
        row_lower_bounds.push_back(1);
        row_upper_bounds.push_back(n);
    }

    // Create matrix
    row_starts.push_back(elements.size());
    matrix = CoinPackedMatrix(false,
            number_of_columns, number_of_rows, elements.size(),
            elements.data(), element_columns.data(),
            row_starts.data(), number_of_elements_in_rows.data());
}

MilpCbcOutput setcoveringsolver::milp_cbc(
        const Instance& instance,
        MilpCbcOptionalParameters parameters)
{
    init_display(instance, parameters.info);
    parameters.info.os()
            << "Algorithm" << std::endl
            << "---------" << std::endl
            << "MILP (CBC)" << std::endl
            << std::endl;

    MilpCbcOutput output(instance, parameters.info);

    SetId n = instance.number_of_sets();
    if (n == 0)
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
    for (SetId s = 0; s < n; ++s)
        solver1.setInteger(s);

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
    std::vector<double> sol_init(n, 0);
    if (parameters.initial_solution != NULL
            && parameters.initial_solution->feasible()) {
        for (SetId s = 0; s < n; ++s)
            if (parameters.initial_solution->contains(s))
                sol_init[s] = 1;
        model.setBestSolution(
                sol_init.data(),
                n,
                parameters.initial_solution->cost());
    }

    // Do complete search.
    model.branchAndBound();

    if (model.isProvenInfeasible()) {  // Infeasible.
        // Update dual bound.
        output.update_lower_bound(
                instance.total_cost(),
                std::stringstream(""),
                parameters.info);
    } else if (model.isProvenOptimal()) {  // Optimal
        // Update primal solution.
        if (!output.solution.feasible()
                || output.solution.cost() > model.getObjValue() + 0.5) {
            const double* solution_cbc = model.solver()->getColSolution();
            Solution solution(instance);
            for (SetId s = 0; s < n; ++s)
                if (solution_cbc[s] > 0.5)
                        solution.add(s);
            output.update_solution(
                    solution,
                    std::stringstream(""),
                    parameters.info);
        }
        // Update dual bound.
        output.update_lower_bound(
                output.solution.cost(),
                std::stringstream(""),
                parameters.info);
    } else if (model.bestSolution() != NULL) {  // Feasible solution found.
        // Update primal solution.
        if (!output.solution.feasible()
                || output.solution.cost() > model.getObjValue() + 0.5) {
            const double* solution_cbc = model.solver()->getColSolution();
            Solution solution(instance);
            for (SetId s = 0; s < n; ++s)
                if (solution_cbc[s] > 0.5)
                        solution.add(s);
            output.update_solution(
                    solution,
                    std::stringstream(""),
                    parameters.info);
        }
        // Update dual bound.
        Cost lb = std::ceil(model.getBestPossibleObjValue() - FFOT_TOL);
        output.update_lower_bound(lb, std::stringstream(""), parameters.info);
    } else {   // No feasible solution found.
        // Update dual bound.
        Cost lb = std::ceil(model.getBestPossibleObjValue() - FFOT_TOL);
        output.update_lower_bound(lb, std::stringstream(""), parameters.info);
    }

    return output.algorithm_end(parameters.info);
}

#endif

