#pragma once

#include "setcoveringsolver/solution.hpp"

#include "setcoveringsolver/algorithms/greedy.hpp"
#include "setcoveringsolver/algorithms/milp_cbc.hpp"
#include "setcoveringsolver/algorithms/milp_gurobi.hpp"
#include "setcoveringsolver/algorithms/local_search_row_weighting.hpp"
#include "setcoveringsolver/algorithms/large_neighborhood_search.hpp"

namespace setcoveringsolver
{

Output run(
        std::string algorithm,
        Instance& instance,
        Cost goal,
        std::mt19937_64& generator,
        optimizationtools::Info info);

}

