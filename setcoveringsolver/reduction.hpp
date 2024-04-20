#pragma once

#include "setcoveringsolver/solution.hpp"

namespace setcoveringsolver
{

/**
 * Structure passed as parameters of the reduction algorithm and the other
 * algorithm to determine whether and how to reduce.
 */
struct ReductionParameters
{
    /** Boolean indicating if the reduction should be performed. */
    bool reduce = true;

    /** Maximum number of rounds. */
    Counter maximum_number_of_rounds = 10;

    /**
     * Booelean indicating if the dominated sets/elements removal should be
     * performed.
     *
     * These reduction operations are expensive on large problems.
     */
    bool remove_domianted = false;
};

class Reduction
{

public:

    /** Constructor. */
    Reduction(
            const Instance& instance,
            const ReductionParameters& parameters = {});

    /** Get the reduced instance. */
    const Instance& instance() const { return instance_; };

    /** Unreduce a solution of the reduced instance. */
    Solution unreduce_solution(
            const Solution& solution) const;

    /** Unreduce a bound of the reduced instance. */
    Cost unreduce_bound(
            Cost bound) const;

private:

    /*
     * Private methods
     */

    /** Remove mandatory sets. */
    bool reduce_mandatory_sets();

    /** Remove identical elements. */
    bool reduce_identical_elements();

    /** Remove identical sets. */
    bool reduce_identical_sets();

    /** Remove dominated elements. */
    bool reduce_domianted_elements();

    /** Remove dominated sets. */
    bool reduce_domianted_sets();

    /*
     * Private attributes
     */

    /** Original instance. */
    const Instance* original_instance_ = nullptr;

    /** Reduced instance. */
    Instance instance_;

    /** For each set, the corresponding set in the original instance. */
    std::vector<SetId> unreduction_operations_;

    /** Mandatory sets (from the original instance). */
    std::vector<SetId> mandatory_sets_;

    /**
     * Cost to add to a solution of the reduced instance to get the cost of
     * the corresponding solution of the original instance.
     **/
    Cost extra_cost_;

};

}
