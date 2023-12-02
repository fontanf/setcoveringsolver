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

        SetId lb = std::ceil(getDoubleInfo(GRB_CB_MIPSOL_OBJBND) - FFOT_TOL);
        algorithm_formatter_.update_bound(
                output_,
                lb,
                std::stringstream(""));

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
            std::stringstream ss;
            algorithm_formatter_.update_solution(
                    output_,
                    solution,
                    std::stringstream(""));
        }
    }

private:

    const Instance& instance_;
    AlgorithmFormatter& algorithm_formatter_;
    Output& output_;
    GRBVar* x_;

};

const Output setcoveringsolver::setcovering::milp_gurobi(
        const Instance& original_instance,
        const MilpGurobiParameters& parameters)
{
    AlgorithmFormatter algorithm_formatter(parameters);
    Output output(original_instance);
    algorithm_formatter.start(output, "MILP (Gurobi)");

    GRBEnv env;

    // Reduction.
    std::unique_ptr<Instance> reduced_instance = nullptr;
    if (parameters.reduction_parameters.reduce) {
        reduced_instance = std::unique_ptr<Instance>(
                new Instance(
                    original_instance.reduce(
                        parameters.reduction_parameters)));
        algorithm_formatter.print_reduced_instance(*reduced_instance);
    }
    const Instance& instance = (reduced_instance == nullptr)? original_instance: *reduced_instance;
    algorithm_formatter.print_header(output);

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
        algorithm_formatter.update_bound(
                output,
                instance.total_cost() + 1,
                std::stringstream(""));
    } else if (optimstatus == GRB_OPTIMAL) {
        Solution solution(instance);
        for (SetId set_id = 0; set_id < instance.number_of_sets(); ++set_id)
            if (x[set_id].get(GRB_DoubleAttr_X) > 0.5)
                solution.add(set_id);
        algorithm_formatter.update_solution(
                output,
                solution,
                std::stringstream(""));
        algorithm_formatter.update_bound(
                output,
                solution.cost(),
                std::stringstream(""));
    } else if (model.get(GRB_IntAttr_SolCount) > 0) {
        Solution solution(instance);
        for (SetId set_id = 0; set_id < instance.number_of_sets(); ++set_id)
            if (x[set_id].get(GRB_DoubleAttr_X) > 0.5)
                solution.add(set_id);
        algorithm_formatter.update_solution(
                output,
                solution,
                std::stringstream(""));
        SetId lb = std::ceil(model.get(GRB_DoubleAttr_ObjBound) - FFOT_TOL);
        algorithm_formatter.update_bound(
                output,
                lb,
                std::stringstream(""));
    } else {
        SetId lb = std::ceil(model.get(GRB_DoubleAttr_ObjBound) - FFOT_TOL);
        algorithm_formatter.update_bound(
                output,
                lb,
                std::stringstream(""));
    }

    algorithm_formatter.end(output);
    return output;
}

#endif
