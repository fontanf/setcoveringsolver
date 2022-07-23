#pragma once

#include "setcoveringsolver/solution.hpp"

#include "setcoveringsolver/algorithms/greedy.hpp"
#include "setcoveringsolver/algorithms/milp_cbc.hpp"
#include "setcoveringsolver/algorithms/milp_gurobi.hpp"
#include "setcoveringsolver/algorithms/localsearch_rowweighting.hpp"
#include "setcoveringsolver/algorithms/largeneighborhoodsearch.hpp"

namespace setcoveringsolver
{

Output run(
        std::string algorithm,
        Instance& instance,
        Cost goal,
        std::mt19937_64& generator,
        optimizationtools::Info info);

}

