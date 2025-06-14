#pragma once

#include "setcoveringsolver/solution.hpp"

#include "optimizationtools/utils/timer.hpp"

namespace setcoveringsolver
{

/**
 * Structure passed as parameters of the reduction algorithm and the other
 * algorithm to determine whether and how to reduce.
 */
struct ReductionParameters
{
    /** Timer. */
    optimizationtools::Timer timer;

    /** Boolean indicating if the reduction should be performed. */
    bool reduce = true;

    /** Maximum number of rounds. */
    Counter maximum_number_of_rounds = 999;

    /**
     * Booelean indicating if the dominated sets/elements removal should be
     * performed.
     *
     * These reduction operations are expensive on large problems.
     */
    bool remove_dominated = false;
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

    void unreduce_solution(
            Solution& new_solution,
            const Solution& solution) const;

    /** Unreduce a bound of the reduced instance. */
    Cost unreduce_bound(
            Cost bound) const;

private:

    /**
     * Structure that stores the unreduction operation for a considered set.
     */
    struct UnreductionOperations
    {
        /**
         * List of sets from the original instance to add if the considered set
         * is in the solution of the reduced instance.
         */
        std::vector<SetId> in;

        /**
         * List of sets from the original instance to add if the considered set
         * is NOT in the solution of the reduced instance.
         */
        std::vector<SetId> out;
    };

    struct ReductionSet
    {
        bool removed = false;
        std::vector<ElementId> elements;
        Cost cost;
    };

    struct ReductionElement
    {
        bool removed = false;
        std::vector<SetId> sets;
    };

    struct ReductionInstance
    {
        std::vector<ReductionSet> sets;
        std::vector<ReductionElement> elements;

        SetId number_of_sets() const { return this->sets.size(); }
        SetId number_of_elements() const { return this->elements.size(); }
        ReductionSet& set(SetId set_id) { return this->sets[set_id]; }
        ReductionElement& element(ElementId element_id) { return this->elements[element_id]; }
        const ReductionSet& set(SetId set_id) const { return this->sets[set_id]; }
        const ReductionElement& element(ElementId element_id) const { return this->elements[element_id]; }
    };

    struct Tmp
    {
        Tmp(const Instance& instance):
            unreduction_operations(instance.number_of_sets()),
            hashes_((std::max)(instance.number_of_elements(), instance.number_of_sets())),
            indexed_set_((std::max)(instance.number_of_elements(), instance.number_of_sets())),
            indexed_set_2_((std::max)(instance.number_of_elements(), instance.number_of_sets())),
            indexed_set_3_((std::max)(instance.number_of_elements(), instance.number_of_sets())),
            indexed_set_4_((std::max)(instance.number_of_elements(), instance.number_of_sets())),
            indexed_set_5_((std::max)(instance.number_of_elements(), instance.number_of_sets())),
            indexed_set_6_((std::max)(instance.number_of_elements(), instance.number_of_sets())),
            indexed_set_7_((std::max)(instance.number_of_elements(), instance.number_of_sets())),
            indexed_map_((std::max)(instance.number_of_elements(), instance.number_of_sets())),
            random_((std::max)(instance.number_of_elements(), instance.number_of_sets())),
            set_((std::max)(instance.number_of_elements(), instance.number_of_sets())) { }

        ReductionInstance instance;

        ReductionInstance instance_2;

        std::vector<UnreductionOperations> unreduction_operations;

        std::vector<int64_t> hashes_;

        std::vector<uint64_t> random_;

        std::vector<SetId> set_;

        optimizationtools::IndexedSet indexed_set_;

        optimizationtools::IndexedSet indexed_set_2_;

        optimizationtools::IndexedSet indexed_set_3_;

        optimizationtools::IndexedSet indexed_set_4_;

        optimizationtools::IndexedSet indexed_set_5_;

        optimizationtools::IndexedSet indexed_set_6_;

        optimizationtools::IndexedSet indexed_set_7_;

        optimizationtools::IndexedMap<SetPos> indexed_map_;

        std::mt19937_64 generator_;
    };

    ReductionInstance instance_to_reduction(
            const Instance& instance);

    bool check(
            const ReductionInstance& reduction_instance);

    bool needs_update(
            ReductionInstance& reduction_instance);

    void update(
            ReductionInstance& reduction_instance,
            std::vector<UnreductionOperations>& unreduction_operations);

    Instance reduction_to_instance(
            const ReductionInstance& reduction_instance);

    /*
     * Private methods
     */

    /** Remove mandatory sets. */
    bool reduce_mandatory_sets(Tmp& tmp);

    /** Remove identical elements. */
    bool reduce_identical_elements(Tmp& tmp);

    /** Remove identical sets. */
    bool reduce_identical_sets(Tmp& tmp);

    /** Perform set folding reduction. */
    bool reduce_set_folding(Tmp& tmp);

    /** Perform set folding reduction. */
    bool reduce_twin(Tmp& tmp);

    /** Perform unconfined sets reduction. */
    bool reduce_unconfined_sets(Tmp& tmp);

    /** Perform crown reduction. */
    bool reduce_crown(Tmp& tmp);

    /** Remove elements dominated by elements covered by 2 sets. */
    bool reduce_dominated_elements_2(
            Tmp& tmp,
            const ReductionParameters& parameters);

    /** Remove dominated elements. */
    bool reduce_dominated_elements(
            Tmp& tmp,
            const ReductionParameters& parameters);

    /** Remove dominated sets among the sets covering 2 elements. */
    bool reduce_dominated_sets_2(
            Tmp& tmp,
            const ReductionParameters& parameters);

    /** Remove dominated sets. */
    bool reduce_dominated_sets(
            Tmp& tmp,
            const ReductionParameters& parameters);

    void reduce_small_components(Tmp& tmp);

    /*
     * Private attributes
     */

    /** Original instance. */
    const Instance* original_instance_ = nullptr;

    /** Reduced instance. */
    Instance instance_;

    /** For each set, the corresponding set in the original instance. */
    std::vector<UnreductionOperations> unreduction_operations_;

    /** Mandatory sets (from the original instance). */
    std::vector<SetId> mandatory_sets_;

    /**
     * Cost to add to a solution of the reduced instance to get the cost of
     * the corresponding solution of the original instance.
     **/
    Cost extra_cost_;

};

}
