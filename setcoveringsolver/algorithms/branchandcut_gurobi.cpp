#if GUROBI_FOUND

#include "setcoveringsolver/algorithms/branchandcut_gurobi.hpp"

#include "gurobi_c++.h"

using namespace setcoveringsolver;

BranchAndCutGurobiOutput& BranchAndCutGurobiOutput::algorithm_end(Info& info)
{
    //PUT(info, "Algorithm", "Iterations", it);
    Output::algorithm_end(info);
    //VER(info, "Iterations: " << it << std::endl);
    return *this;
}

class BranchAndCutGurobiCallback: public GRBCallback
{

public:

    BranchAndCutGurobiCallback(
            const Instance& ins,
            BranchAndCutGurobiOptionalParameters& p,
            BranchAndCutGurobiOutput& output,
            GRBVar* x):
        instance_(ins), p_(p), output_(output), x_(x) { }

protected:

    void callback()
    {
        if (where != GRB_CB_MIPSOL)
            return;

        SetId lb = std::ceil(getDoubleInfo(GRB_CB_MIPSOL_OBJBND) - TOL);
        output_.update_lower_bound(lb, std::stringstream(""), p_.info);

        if (!output_.solution.feasible() || output_.solution.cost() > getDoubleInfo(GRB_CB_MIPSOL_OBJ) + 0.5) {
            Solution solution(instance_);
            double* x = getSolution(x_, instance_.set_number());
            for (SetId s = 0; s < instance_.set_number(); ++s)
                if (x[s] > 0.5)
                    solution.add(s);
            std::stringstream ss;
            output_.update_solution(solution, std::stringstream(""), p_.info);
        }
    }

private:

    const Instance& instance_;
    BranchAndCutGurobiOptionalParameters& p_;
    BranchAndCutGurobiOutput& output_;
    GRBVar* x_;

};

BranchAndCutGurobiOutput setcoveringsolver::branchandcut_gurobi(
        const Instance& instance, BranchAndCutGurobiOptionalParameters p)
{
    GRBEnv env;
    VER(p.info, "*** branchandcut_gurobi ***" << std::endl);

    BranchAndCutGurobiOutput output(instance, p.info);

    GRBModel model(env);

    // Variables
    GRBVar* x = model.addVars(instance.set_number(), GRB_BINARY);

    // Objective
    for (SetId s = 0; s < instance.set_number(); ++s)
        x[s].set(GRB_DoubleAttr_Obj, instance.set(s).cost);
    model.set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);

    // Constraint
    for (ElementId e = 0; e < instance.element_number(); ++e) {
        GRBLinExpr expr;
        for (SetId s: instance.element(e).sets)
            expr += x[s];
        model.addConstr(expr >= 1);
    }

    // Initial solution
    if (p.initial_solution != NULL && p.initial_solution->feasible())
        for (SetId s = 0; s < instance.set_number(); ++s)
            x[s].set(GRB_DoubleAttr_Start, (p.initial_solution->contains(s))? 1: 0);

    // Redirect standard output to log file
    model.set(GRB_StringParam_LogFile, "gurobi.log"); // Write log to file
    model.set(GRB_IntParam_LogToConsole, 0); // Remove standard output
    model.set(GRB_DoubleParam_MIPGap, 0); // Fix precision issue
    //model.set(GRB_IntParam_MIPFocus, 3); // Focus on the bound
    //model.set(GRB_IntParam_Method, 1); // Dual simplex

    // Time limit
    if (p.info.timelimit != std::numeric_limits<double>::infinity())
        model.set(GRB_DoubleParam_TimeLimit, p.info.timelimit);

    // Callback
    BranchAndCutGurobiCallback cb = BranchAndCutGurobiCallback(instance, p, output, x);
    model.setCallback(&cb);

    // Optimize
    model.optimize();

    int optimstatus = model.get(GRB_IntAttr_Status);
    if (optimstatus == GRB_INFEASIBLE) {
        output.update_lower_bound(instance.total_cost() + 1, std::stringstream(""), p.info);
    } else if (optimstatus == GRB_OPTIMAL) {
        Solution solution(instance);
        for (SetId s = 0; s < instance.set_number(); ++s)
            if (x[s].get(GRB_DoubleAttr_X) > 0.5)
                solution.add(s);
        output.update_solution(solution, std::stringstream(""), p.info);
        output.update_lower_bound(solution.cost(), std::stringstream(""), p.info);
    } else if (model.get(GRB_IntAttr_SolCount) > 0) {
        Solution solution(instance);
        for (SetId s = 0; s < instance.set_number(); ++s)
            if (x[s].get(GRB_DoubleAttr_X) > 0.5)
                solution.add(s);
        output.update_solution(solution, std::stringstream(""), p.info);
        SetId lb = std::ceil(model.get(GRB_DoubleAttr_ObjBound) - TOL);
        output.update_lower_bound(lb, std::stringstream(""), p.info);
    } else {
        SetId lb = std::ceil(model.get(GRB_DoubleAttr_ObjBound) - TOL);
        output.update_lower_bound(lb, std::stringstream(""), p.info);
    }

    return output.algorithm_end(p.info);
}

#endif
