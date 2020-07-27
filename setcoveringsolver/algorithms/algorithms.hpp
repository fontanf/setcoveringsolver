#pragma once

#include "setcoveringsolver/solution.hpp"

#include "setcoveringsolver/algorithms/greedy.hpp"
#include "setcoveringsolver/algorithms/branchandcut.hpp"
#include "setcoveringsolver/algorithms/localsearch.hpp"
#include "setcoveringsolver/algorithms/largeneighborhoodsearch.hpp"

namespace setcoveringsolver
{

Output run(std::string algorithm, Instance& instance, std::mt19937_64& generator, Info info);

}

