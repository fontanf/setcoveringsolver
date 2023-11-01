#pragma once

#include "setcoveringsolver/setcovering/solution.hpp"

#include "setcoveringsolver/setcovering/algorithms/greedy.hpp"
#include "setcoveringsolver/setcovering/algorithms/milp_cbc.hpp"
#include "setcoveringsolver/setcovering/algorithms/milp_gurobi.hpp"
#include "setcoveringsolver/setcovering/algorithms/local_search_row_weighting.hpp"
#include "setcoveringsolver/setcovering/algorithms/large_neighborhood_search.hpp"

namespace setcoveringsolver
{
namespace setcovering
{

Output run(
        std::string algorithm,
        Instance& instance,
        Cost goal,
        std::mt19937_64& generator,
        optimizationtools::Info info);

}
}

