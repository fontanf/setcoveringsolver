#if GUROBI_FOUND

#include "setcoveringsolver/setcovering/algorithms/milp_gurobi.hpp"

#include "setcoveringsolver/setcovering/algorithm_formatter.hpp"

#include "gurobi_c++.h"

using namespace setcoveringsolver::setcovering;

class MilpGurobiCallback: public GRBCallback
{

public:

    MilpGurobiCallback(
            const Instance& instance,
            AlgorithmFormatter& algorithm_formatter,
            Output& output,
            GRBVar* x):
        instance_(instance),
        algorithm_formatter_(algorithm_formatter),
        output_(output),
        x_(x) { }

protected:

    void callback()
    {
        if (where != GRB_CB_MIPSOL)
            return;

        SetId bound = std::ceil(getDoubleInfo(GRB_CB_MIPSOL_OBJBND) - FFOT_TOL);
        algorithm_formatter_.update_bound(bound, "");

        if (!output_.solution.feasible()
                || output_.solution.cost() > getDoubleInfo(GRB_CB_MIPSOL_OBJ) + 0.5) {
            Solution solution(instance_);
            double* x = getSolution(x_, instance_.number_of_sets());
            for (SetId set_id = 0;
                    set_id < instance_.number_of_sets();
                    ++set_id) {
                if (x[set_id] > 0.5)
                    solution.add(set_id);
            }
            algorithm_formatter_.update_solution(solution, "");
        }
    }

private:

    const Instance& instance_;
    AlgorithmFormatter& algorithm_formatter_;
    Output& output_;
    GRBVar* x_;

};

const Output setcoveringsolver::setcovering::milp_gurobi(
        const Instance& instance,
        const MilpGurobiParameters& parameters)
{
    Output output(instance);
    AlgorithmFormatter algorithm_formatter(parameters, output);
    algorithm_formatter.start("MILP (Gurobi)");

    GRBEnv env;

    // Reduction.
    if (parameters.reduction_parameters.reduce)
        return solve_reduced_instance(milp_gurobi, instance, parameters, algorithm_formatter, output);

    algorithm_formatter.print_header();

    GRBModel model(env);

    // Variables
    GRBVar* x = model.addVars(instance.number_of_sets(), GRB_BINARY);

    // Objective
    for (SetId set_id = 0; set_id < instance.number_of_sets(); ++set_id)
        x[set_id].set(GRB_DoubleAttr_Obj, instance.set(set_id).cost);
    model.set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);

    // Constraint
    for (ElementId element_id = 0;
            element_id < instance.number_of_elements();
            ++element_id) {
        GRBLinExpr expr;
        for (SetId set_id: instance.element(element_id).sets)
            expr += x[set_id];
        model.addConstr(expr >= 1);
    }

    // Initial solution
    if (parameters.initial_solution != NULL
            && parameters.initial_solution->feasible()) {
        for (SetId set_id = 0; set_id < instance.number_of_sets(); ++set_id) {
            x[set_id].set(
                    GRB_DoubleAttr_Start,
                    (parameters.initial_solution->contains(set_id))? 1: 0);
        }
    }

    // Redirect standard output to log file
    model.set(GRB_StringParam_LogFile, "gurobi.log"); // Write log to file
    model.set(GRB_IntParam_LogToConsole, 0); // Remove standard output
    model.set(GRB_DoubleParam_MIPGap, 0); // Fix precision issue
    //model.set(GRB_IntParam_MIPFocus, 3); // Focus on the bound
    //model.set(GRB_IntParam_Method, 1); // Dual simplex

    // Time limit
    if (parameters.timer.remaining_time() != std::numeric_limits<double>::infinity())
        model.set(GRB_DoubleParam_TimeLimit, parameters.timer.remaining_time());

    // Callback
    MilpGurobiCallback cb = MilpGurobiCallback(
            instance,
            algorithm_formatter,
            output,
            x);
    model.setCallback(&cb);

    // Optimize
    model.optimize();

    int optimstatus = model.get(GRB_IntAttr_Status);

    if (optimstatus == GRB_INFEASIBLE) {
        // Infeasible.

        // Update dual bound.
        algorithm_formatter.update_bound(instance.total_cost() + 1, "");

    } else if (optimstatus == GRB_OPTIMAL) {
        // Optimal.

        // Update primal solution.
        Solution solution(instance);
        for (SetId set_id = 0; set_id < instance.number_of_sets(); ++set_id)
            if (x[set_id].get(GRB_DoubleAttr_X) > 0.5)
                solution.add(set_id);
        algorithm_formatter.update_solution(solution, "");

        // Update dual bound.
        algorithm_formatter.update_bound(solution.cost(), "");

    } else if (model.get(GRB_IntAttr_SolCount) > 0) {
        // Feasible solution found.

        // Update primal solution.
        Solution solution(instance);
        for (SetId set_id = 0; set_id < instance.number_of_sets(); ++set_id)
            if (x[set_id].get(GRB_DoubleAttr_X) > 0.5)
                solution.add(set_id);
        algorithm_formatter.update_solution(solution, "");

        // Update dual bound.
        Cost bound = std::ceil(model.get(GRB_DoubleAttr_ObjBound) - FFOT_TOL);
        algorithm_formatter.update_bound(bound, "");

    } else {
        // No feasible solution found.

        // Update dual bound.
        Cost bound = std::ceil(model.get(GRB_DoubleAttr_ObjBound) - FFOT_TOL);
        algorithm_formatter.update_bound(bound, "");
    }

    algorithm_formatter.end();
    return output;
}

#endif
