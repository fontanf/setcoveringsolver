#if GUROBI_FOUND

#include "setcoveringsolver/algorithms/milp_gurobi.hpp"

#include "gurobi_c++.h"

using namespace setcoveringsolver;

MilpGurobiOutput& MilpGurobiOutput::algorithm_end(
        optimizationtools::Info& info)
{
    Output::algorithm_end(info);
    return *this;
}

class MilpGurobiCallback: public GRBCallback
{

public:

    MilpGurobiCallback(
            const Instance& ins,
            MilpGurobiOptionalParameters& p,
            MilpGurobiOutput& output,
            GRBVar* x):
        instance_(ins), p_(p), output_(output), x_(x) { }

protected:

    void callback()
    {
        if (where != GRB_CB_MIPSOL)
            return;

        SetId lb = std::ceil(getDoubleInfo(GRB_CB_MIPSOL_OBJBND) - FFOT_TOL);
        output_.update_lower_bound(lb, std::stringstream(""), p_.info);

        if (!output_.solution.feasible() || output_.solution.cost() > getDoubleInfo(GRB_CB_MIPSOL_OBJ) + 0.5) {
            Solution solution(instance_);
            double* x = getSolution(x_, instance_.number_of_sets());
            for (SetId set_id = 0;
                    set_id < instance_.number_of_sets();
                    ++set_id) {
                if (x[set_id] > 0.5)
                    solution.add(set_id);
            }
            std::stringstream ss;
            output_.update_solution(solution, std::stringstream(""), p_.info);
        }
    }

private:

    const Instance& instance_;
    MilpGurobiOptionalParameters& p_;
    MilpGurobiOutput& output_;
    GRBVar* x_;

};

MilpGurobiOutput setcoveringsolver::milp_gurobi(
        const Instance& instance,
        MilpGurobiOptionalParameters parameters)
{
    GRBEnv env;
    init_display(instance, parameters.info);
    parameters.info.os()
            << "Algorithm" << std::endl
            << "---------" << std::endl
            << "MILP (Gurobi)" << std::endl
            << std::endl;

    MilpGurobiOutput output(instance, parameters.info);

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
    if (parameters.info.time_limit != std::numeric_limits<double>::infinity())
        model.set(GRB_DoubleParam_TimeLimit, parameters.info.time_limit);

    // Callback
    MilpGurobiCallback cb = MilpGurobiCallback(instance, parameters, output, x);
    model.setCallback(&cb);

    // Optimize
    model.optimize();

    int optimstatus = model.get(GRB_IntAttr_Status);
    if (optimstatus == GRB_INFEASIBLE) {
        output.update_lower_bound(
                instance.total_cost() + 1,
                std::stringstream(""),
                parameters.info);
    } else if (optimstatus == GRB_OPTIMAL) {
        Solution solution(instance);
        for (SetId set_id = 0; set_id < instance.number_of_sets(); ++set_id)
            if (x[set_id].get(GRB_DoubleAttr_X) > 0.5)
                solution.add(set_id);
        output.update_solution(
                solution,
                std::stringstream(""),
                parameters.info);
        output.update_lower_bound(
                solution.cost(),
                std::stringstream(""),
                parameters.info);
    } else if (model.get(GRB_IntAttr_SolCount) > 0) {
        Solution solution(instance);
        for (SetId set_id = 0; set_id < instance.number_of_sets(); ++set_id)
            if (x[set_id].get(GRB_DoubleAttr_X) > 0.5)
                solution.add(set_id);
        output.update_solution(
                solution,
                std::stringstream(""),
                parameters.info);
        SetId lb = std::ceil(model.get(GRB_DoubleAttr_ObjBound) - FFOT_TOL);
        output.update_lower_bound(lb, std::stringstream(""), parameters.info);
    } else {
        SetId lb = std::ceil(model.get(GRB_DoubleAttr_ObjBound) - FFOT_TOL);
        output.update_lower_bound(lb, std::stringstream(""), parameters.info);
    }

    return output.algorithm_end(parameters.info);
}

#endif
