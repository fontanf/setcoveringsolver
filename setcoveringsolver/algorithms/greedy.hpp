#pragma once

#include "setcoveringsolver/solution.hpp"

namespace setcoveringsolver
{

Output greedy(const Instance& instance, Info info = Info());

Output greedy_dual(const Instance& instance, Info info = Info());

}

