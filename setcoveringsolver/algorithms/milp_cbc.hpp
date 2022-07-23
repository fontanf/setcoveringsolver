#pragma once

#if COINOR_FOUND

#include "setcoveringsolver/solution.hpp"

#include <coin/CbcModel.hpp>
#include <coin/OsiCbcSolverInterface.hpp>
#include <coin/CglKnapsackCover.hpp>
#include <coin/CglClique.hpp>

namespace setcoveringsolver
{

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

struct MilpCbcOptionalParameters
{
    optimizationtools::Info info = optimizationtools::Info();

    const Solution* initial_solution = NULL;
};

struct MilpCbcOutput: Output
{
    MilpCbcOutput(
            const Instance& instance,
            optimizationtools::Info& info):
        Output(instance, info) { }

    MilpCbcOutput& algorithm_end(optimizationtools::Info& info);
};

MilpCbcOutput milp_cbc(
        const Instance& instance,
        MilpCbcOptionalParameters parameters = {});

}

#endif

