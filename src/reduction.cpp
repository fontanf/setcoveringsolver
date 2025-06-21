#include "setcoveringsolver/reduction.hpp"

#include "setcoveringsolver/instance_builder.hpp"
#include "setcoveringsolver/algorithms/trivial_bound.hpp"
#include "setcoveringsolver/algorithms/clique_cover_bound.hpp"
#include "setcoveringsolver/algorithms/greedy.hpp"

#include "optimizationtools/containers/indexed_set.hpp"

using namespace setcoveringsolver;

bool Reduction::check(const ReductionInstance& reduction_instance)
{
    std::vector<SetPos> elements_number_of_sets(reduction_instance.number_of_elements(), 0);
    for (SetId set_id = 0;
            set_id < reduction_instance.number_of_sets();
            ++set_id) {
        const ReductionSet& set = reduction_instance.set(set_id);
        if (set.removed)
            continue;
        for (ElementId element_id: set.elements) {
            const ReductionElement& element = reduction_instance.elements[element_id];
            if (element_id < 0
                    || element_id >= reduction_instance.number_of_elements()) {
                throw std::logic_error(
                        "elementcoveringsolver::Reduction::check; "
                        "set_id: " + std::to_string(set_id) + "; "
                        "element_id: " + std::to_string(element_id) + +"; "
                        "number_of_elements: " + std::to_string(reduction_instance.number_of_elements()) + +"; ");
            }
            if (element.removed) {
                throw std::logic_error(
                        "elementcoveringsolver::Reduction::check; "
                        "set_id: " + std::to_string(set_id) + "; "
                        "element_id: " + std::to_string(element_id) + +"; "
                        "removed: " + std::to_string(element.removed) + +"; ");

            }
            elements_number_of_sets[element_id]++;
        }
    }

    std::vector<ElementPos> sets_number_of_elements(reduction_instance.number_of_sets(), 0);
    for (ElementId element_id = 0;
            element_id < reduction_instance.number_of_elements();
            ++element_id) {
        const ReductionElement& element = reduction_instance.element(element_id);
        if (element.removed)
            continue;
        for (SetId set_id: element.sets) {
            const ReductionSet& set = reduction_instance.sets[set_id];
            if (set_id < 0
                    || set_id >= reduction_instance.number_of_sets()) {
                throw std::logic_error(
                        "setcoveringsolver::Reduction::check; "
                        "element_id: " + std::to_string(element_id) + "; "
                        "set_id: " + std::to_string(set_id) + +"; "
                        "number_of_sets: " + std::to_string(reduction_instance.number_of_sets()) + +"; ");
            }
            if (set.removed) {
                throw std::logic_error(
                        "setcoveringsolver::Reduction::check; "
                        "element_id: " + std::to_string(element_id) + "; "
                        "set_id: " + std::to_string(set_id) + +"; "
                        "removed: " + std::to_string(set.removed) + +"; ");

            }
            sets_number_of_elements[set_id]++;
        }
        if (element.sets.size() == 0) {
            throw std::logic_error(
                    "setcoveringsolver::Reduction::check: "
                    "uncoverable element; "
                    "element_id: " + std::to_string(element_id) + ".");
        }
    }

    for (SetId set_id = 0;
            set_id < reduction_instance.number_of_sets();
            ++set_id) {
        const ReductionSet& set = reduction_instance.set(set_id);
        if (set.removed)
            continue;
        if (set.elements.size() != sets_number_of_elements[set_id]) {
            throw std::logic_error(
                    "setcoveringsolver::Reduction::check: "
                    "set_id: " + std::to_string(set_id) + "; "
                    "set.elements.size(): " + std::to_string(set.elements.size()) + "; "
                    "sets_number_of_elements: " + std::to_string(sets_number_of_elements[set_id]) + ".");
        }
    }

    for (ElementId element_id = 0;
            element_id < reduction_instance.number_of_elements();
            ++element_id) {
        const ReductionElement& element = reduction_instance.element(element_id);
        if (element.removed)
            continue;
        if (element.sets.size() != elements_number_of_sets[element_id]) {
            throw std::logic_error(
                    "setcoveringsolver::Reduction::check: "
                    "element_id: " + std::to_string(element_id) + ".");
        }
    }

    return true;
}

Reduction::ReductionInstance Reduction::instance_to_reduction(
        const Instance& instance)
{
    ReductionInstance reduction_instance;
    reduction_instance.sets = std::vector<ReductionSet>(instance.number_of_sets());
    for (SetId set_id = 0;
            set_id < instance.number_of_sets();
            ++set_id) {
        reduction_instance.sets[set_id].cost = instance.set(set_id).cost;
        reduction_instance.sets[set_id].elements = instance.set(set_id).elements;
    }
    reduction_instance.elements = std::vector<ReductionElement>(instance.number_of_elements());
    for (ElementId element_id = 0;
            element_id < instance.number_of_elements();
            ++element_id) {
        reduction_instance.elements[element_id].sets = instance.element(element_id).sets;
    }
    //check(reduction_instance);
    return reduction_instance;
}

bool Reduction::needs_update(
        ReductionInstance& reduction_instance)
{
    // Check number of sets.
    SetId n = 0;
    for (SetId set_id = 0;
            set_id < reduction_instance.number_of_sets();
            ++set_id) {
        ReductionSet& set = reduction_instance.set(set_id);
        if (!set.removed)
            n++;
    }
    if (n < 0.9 * reduction_instance.number_of_sets())
        return true;

    // Check number of elements.
    ElementId m = 0;
    for (ElementId element_id = 0;
            element_id < reduction_instance.number_of_elements();
            ++element_id) {
        ReductionElement& element = reduction_instance.element(element_id);
        if (!element.removed)
            m++;
    }
    if (m < 0.9 * reduction_instance.number_of_elements())
        return true;

    return false;
}

void Reduction::update(
        ReductionInstance& reduction_instance,
        std::vector<UnreductionOperations>& unreduction_operations)
{
    //std::cout << "update"
    //    << " m " << reduction_instance.number_of_elements()
    //    << " n " << reduction_instance.number_of_sets()
    //    << std::endl;
    std::vector<SetId> sets_original2reduced(reduction_instance.number_of_sets(), -1);
    while (!reduction_instance.sets.empty()
            && reduction_instance.sets.back().removed) {
        reduction_instance.sets.pop_back();
        unreduction_operations.pop_back();
    }
    for (SetId set_id = 0;
            set_id < reduction_instance.number_of_sets();
            ++set_id) {
        ReductionSet& set = reduction_instance.set(set_id);
        if (set.removed) {
            reduction_instance.sets[set_id] = reduction_instance.sets.back();
            unreduction_operations[set_id] = unreduction_operations.back();
            sets_original2reduced[reduction_instance.number_of_sets() - 1] = set_id;
            sets_original2reduced[set_id] = -1;
            reduction_instance.sets.pop_back();
            unreduction_operations.pop_back();
            while (!reduction_instance.sets.empty()
                    && reduction_instance.sets.back().removed) {
                reduction_instance.sets.pop_back();
                unreduction_operations.pop_back();
            }
        } else {
            sets_original2reduced[set_id] = set_id;
        }
    }

    std::vector<ElementId> elements_original2reduced(reduction_instance.number_of_elements(), -1);
    while (!reduction_instance.elements.empty()
            && reduction_instance.elements.back().removed) {
        reduction_instance.elements.pop_back();
    }
    for (ElementId element_id = 0;
            element_id < reduction_instance.number_of_elements();
            ++element_id) {
        ReductionElement& element = reduction_instance.element(element_id);
        if (element.removed) {
            ElementId element_id_2 = reduction_instance.number_of_elements() - 1;
            reduction_instance.elements[element_id] = reduction_instance.elements.back();
            elements_original2reduced[element_id_2] = element_id;
            elements_original2reduced[element_id] = -1;
            reduction_instance.elements.pop_back();
            while (!reduction_instance.elements.empty()
                    && reduction_instance.elements.back().removed) {
                ElementId element_id_2 = reduction_instance.number_of_elements() - 1;
                reduction_instance.elements.pop_back();
            }
        } else {
            elements_original2reduced[element_id] = element_id;
        }
    }

    for (SetId set_id = 0;
            set_id < reduction_instance.number_of_sets();
            ++set_id) {
        ReductionSet& set = reduction_instance.set(set_id);
        for (ElementId& element_id: set.elements)
            element_id = elements_original2reduced[element_id];
    }

    for (ElementId element_id = 0;
            element_id < reduction_instance.number_of_elements();
            ++element_id) {
        ReductionElement& element = reduction_instance.element(element_id);
        for (SetId& set_id: element.sets)
            set_id = sets_original2reduced[set_id];
    }

    //std::cout << "      "
    //    << " m " << reduction_instance.number_of_elements()
    //    << " n " << reduction_instance.number_of_sets()
    //    << std::endl;
    //check(reduction_instance);
}

Instance Reduction::reduction_to_instance(
        const ReductionInstance& reduction_instance)
{
    //std::cout << "reduction_to_instance" << std::endl;
    InstanceBuilder instance_builder;
    instance_builder.add_sets(reduction_instance.number_of_sets());
    instance_builder.add_elements(reduction_instance.number_of_elements());
    for (SetId set_id = 0;
            set_id < reduction_instance.number_of_sets();
            ++set_id) {
        const ReductionSet& set = reduction_instance.set(set_id);
        instance_builder.set_cost(set_id, set.cost);
        for (ElementId element_id: set.elements)
            instance_builder.add_arc(set_id, element_id);
    }
    return instance_builder.build();
}

bool Reduction::reduce_mandatory_sets(Tmp& tmp)
{
    //std::cout << "remove_mandatory_sets..." << std::endl;

    optimizationtools::IndexedSet& covered_elements = tmp.indexed_set_;
    covered_elements.resize_and_clear(tmp.instance.number_of_elements());
    optimizationtools::IndexedSet& covering_sets = tmp.indexed_set_3_;
    covering_sets.resize_and_clear(tmp.instance.number_of_sets());
    optimizationtools::IndexedSet& sets_to_remove = tmp.indexed_set_2_;
    sets_to_remove.resize_and_clear(tmp.instance.number_of_sets());
    optimizationtools::IndexedSet& fixed_sets = tmp.indexed_set_5_;
    fixed_sets.resize_and_clear(tmp.instance.number_of_sets());
    optimizationtools::IndexedSet& elements_to_remove = tmp.indexed_set_4_;
    elements_to_remove.resize_and_clear(tmp.instance.number_of_elements());
    optimizationtools::IndexedSet& touched_sets = tmp.indexed_set_6_;
    touched_sets.resize_and_clear(tmp.instance.number_of_sets());
    optimizationtools::IndexedSet& touched_elements = tmp.indexed_set_7_;
    touched_elements.resize_and_clear(tmp.instance.number_of_elements());

    for (SetId set_id = 0;
            set_id < tmp.instance.number_of_sets();
            ++set_id) {
        const ReductionSet& set = tmp.instance.set(set_id);
        if (set.removed)
            continue;
        touched_sets.add(set_id);
    }
    for (ElementId element_id = 0;
            element_id < tmp.instance.number_of_elements();
            ++element_id) {
        const ReductionElement& element = tmp.instance.element(element_id);
        if (element.removed)
            continue;
        touched_elements.add(element_id);
    }
    for (int i = 0; /*i < 4096*/; ++i) {

        // Remove sets covering a single element.
        for (SetId set_id: touched_sets) {
            if (sets_to_remove.contains(set_id))
                continue;
            const ReductionSet& set = tmp.instance.set(set_id);

            covered_elements.clear();
            for (ElementId element_id: set.elements) {
                if (elements_to_remove.contains(element_id))
                    continue;
                covered_elements.add(element_id);
                if (covered_elements.size() > 1)
                    break;
            }
            if (covered_elements.size() == 0) {
                sets_to_remove.add(set_id);
                continue;
            }
            if (covered_elements.size() > 1)
                continue;

            ElementId element_id = *covered_elements.begin();
            bool dominated = false;
            for (SetId set_id_2: tmp.instance.element(element_id).sets) {
                if (set_id_2 == set_id || sets_to_remove.contains(set_id_2))
                    continue;
                if (tmp.instance.set(set_id_2).cost > set.cost)
                    continue;
                dominated = true;
                break;
            }
            if (dominated) {
                sets_to_remove.add(set_id);
                touched_elements.add(element_id);
            }
        }
        touched_sets.clear();

        // Remove element covered by a single set.
        bool found = false;
        for (ElementId element_id: touched_elements) {
            if (elements_to_remove.contains(element_id))
                continue;
            const ReductionElement& element = tmp.instance.element(element_id);
            covering_sets.clear();
            for (SetId set_id: element.sets) {
                if (!sets_to_remove.contains(set_id)) {
                    covering_sets.add(set_id);
                    if (covering_sets.size() > 1)
                        break;
                }
            }
            if (covering_sets.size() == 1) {
                SetId set_id = *covering_sets.begin();
                sets_to_remove.add(set_id);
                fixed_sets.add(set_id);
                for (ElementId element_id_2: tmp.instance.set(set_id).elements) {
                    if (elements_to_remove.contains(element_id_2))
                        continue;
                    elements_to_remove.add(element_id_2);

                    const ReductionElement& element_2 = tmp.instance.element(element_id_2);
                    for (SetId set_id_2: element_2.sets) {
                        if (sets_to_remove.contains(set_id_2))
                            continue;
                        touched_sets.add(set_id_2);
                    }

                }
                found = true;
            }
        }
        if (!found)
            break;
        touched_elements.clear();

        //std::cout << sets_to_remove.size() << " " << fixed_sets.size() << " " << elements_to_remove.size() << std::endl;
    }

    if (sets_to_remove.size() == 0)
        return false;

    //std::cout << sets_to_remove.size() << " " << fixed_sets.size() << " " << elements_to_remove.size() << std::endl;

    // Update mandatory_sets.
    for (SetId set_id: sets_to_remove) {
        if (fixed_sets.contains(set_id)) {
            for (SetId orig_set_id: unreduction_operations_[set_id].in)
                mandatory_sets_.push_back(orig_set_id);
        } else {
            for (SetId orig_set_id: unreduction_operations_[set_id].out)
                mandatory_sets_.push_back(orig_set_id);
        }
    }
    // Update sets.
    for (SetId set_id = 0;
            set_id < tmp.instance.number_of_sets();
            ++set_id) {
        ReductionSet& set = tmp.instance.set(set_id);
        if (set.removed)
            continue;
        if (sets_to_remove.contains(set_id)) {
            set.removed = true;
        } else {
            for (ElementPos pos = 0;
                    pos < (ElementPos)set.elements.size();
                    ) {
                ElementId element_id = set.elements[pos];
                if (elements_to_remove.contains(element_id)) {
                    set.elements[pos] = set.elements.back();
                    set.elements.pop_back();
                } else {
                    pos++;
                }
            }
        }
    }
    // Update elements.
    for (ElementId element_id = 0;
            element_id < tmp.instance.number_of_elements();
            ++element_id) {
        ReductionElement& element = tmp.instance.element(element_id);
        if (element.removed)
            continue;
        if (elements_to_remove.contains(element_id)) {
            element.removed = true;
        } else {
            for (ElementPos pos = 0;
                    pos < (ElementPos)element.sets.size();
                    ) {
                ElementId set_id = element.sets[pos];
                if (sets_to_remove.contains(set_id)) {
                    element.sets[pos] = element.sets.back();
                    element.sets.pop_back();
                } else {
                    pos++;
                }
            }
        }
    }

    //check(tmp.instance);
    if (needs_update(tmp.instance))
        update(tmp.instance, unreduction_operations_);
    return true;
}

bool Reduction::reduce_identical_elements(Tmp& tmp)
{
    //std::cout << "reduce_identical_elements..." << std::endl;

    // Compute hashes.
    for (ElementId element_id = 0;
            element_id < tmp.instance.number_of_elements();
            ++element_id) {
        const ReductionElement& element = tmp.instance.element(element_id);
        if (element.removed)
            continue;
        int64_t hash = 0;
        for (SetId set_id: element.sets)
            hash ^= tmp.random_[set_id];
        tmp.hashes_[element_id] = hash;
    }

    std::vector<std::vector<ElementId>> elements_by_number_of_sets_covering;
    for (ElementId element_id = 0;
            element_id < tmp.instance.number_of_elements();
            ++element_id) {
        const ReductionElement& element = tmp.instance.element(element_id);
        if (element.removed)
            continue;
        while (elements_by_number_of_sets_covering.size()
                <= element.sets.size()) {
            elements_by_number_of_sets_covering.push_back({});
        }
        elements_by_number_of_sets_covering[element.sets.size()].push_back(element_id);
    }

    optimizationtools::IndexedSet& elements_to_remove = tmp.indexed_set_2_;
    elements_to_remove.resize_and_clear(tmp.instance.number_of_elements());
    optimizationtools::IndexedSet& covering_sets = tmp.indexed_set_;
    covering_sets.resize_and_clear(tmp.instance.number_of_sets());
    for (SetPos n = 0;
            n < (SetPos)elements_by_number_of_sets_covering.size();
            ++n) {

        // Compute sorted elements.
        std::vector<ElementId>& elements_sorted = elements_by_number_of_sets_covering[n];
        if (elements_sorted.empty())
            continue;
        sort(elements_sorted.begin(), elements_sorted.end(),
                [this, &tmp](ElementId element_id_1, ElementId element_id_2) -> bool
                {
                    if (tmp.hashes_[element_id_1] != tmp.hashes_[element_id_2])
                        return tmp.hashes_[element_id_1] < tmp.hashes_[element_id_2];
                    const ReductionElement& element_1 = tmp.instance.element(element_id_1);
                    const ReductionElement& element_2 = tmp.instance.element(element_id_2);
                    return element_1.sets.size() < element_2.sets.size();
                });

        for (ElementPos element_pos = 0;
                element_pos < (ElementPos)elements_sorted.size();
                ++element_pos) {
            ElementId element_id = elements_sorted[element_pos];
            const ReductionElement& element = tmp.instance.element(element_id);

            bool identical = false;
            covering_sets.clear();
            for (ElementPos element_pos_prev = element_pos - 1;; --element_pos_prev) {
                if (element_pos_prev < 0)
                    break;
                ElementId element_id_prev = elements_sorted[element_pos_prev];
                const ReductionElement& element_prev = tmp.instance.element(element_id_prev);
                if (tmp.hashes_[element_id] != tmp.hashes_[element_id_prev])
                    break;
                if (element.sets.size() != element_prev.sets.size())
                    break;
                if (covering_sets.empty()) {
                    for (ElementId element_id: element.sets)
                        covering_sets.add(element_id);
                }
                bool identical_cur = true;
                for (ElementId element_id: element_prev.sets) {
                    if (!covering_sets.contains(element_id)) {
                        identical_cur = false;
                        break;
                    }
                }
                if (identical_cur) {
                    identical = true;
                    break;
                }
            }

            if (identical)
                elements_to_remove.add(element_id);
        }
    }

    if (elements_to_remove.size() == 0)
        return false;

    //std::cout << elements_to_remove.size() << std::endl;

    // Update sets.
    for (SetId set_id = 0;
            set_id < tmp.instance.number_of_sets();
            ++set_id) {
        ReductionSet& set = tmp.instance.set(set_id);
        if (set.removed)
            continue;
        for (ElementPos pos = 0;
                pos < (ElementPos)set.elements.size();
                ) {
            ElementId element_id = set.elements[pos];
            if (elements_to_remove.contains(element_id)) {
                set.elements[pos] = set.elements.back();
                set.elements.pop_back();
            } else {
                pos++;
            }
        }
    }
    // Update elements.
    for (ElementId element_id = 0;
            element_id < tmp.instance.number_of_elements();
            ++element_id) {
        ReductionElement& element = tmp.instance.element(element_id);
        if (element.removed)
            continue;
        if (elements_to_remove.contains(element_id))
            element.removed = true;
    }

    //check(tmp.instance);
    if (needs_update(tmp.instance))
        update(tmp.instance, unreduction_operations_);
    return true;
}

bool Reduction::reduce_identical_sets(Tmp& tmp)
{
    //std::cout << "reduce_identical_sets..." << std::endl;

    // Compute hashes.
    for (SetId set_id = 0;
            set_id < tmp.instance.number_of_sets();
            ++set_id) {
        const ReductionSet& set = tmp.instance.set(set_id);
        if (set.removed)
            continue;
        int64_t hash = 0;
        for (SetId element_id: set.elements)
            hash ^= tmp.random_[element_id];
        tmp.hashes_[set_id] = hash;
    }

    std::vector<std::vector<SetId>> sets_by_number_of_elements_covered;
    for (SetId set_id = 0;
            set_id < tmp.instance.number_of_sets();
            ++set_id) {
        const ReductionSet& set = tmp.instance.set(set_id);
        if (set.removed)
            continue;
        while (sets_by_number_of_elements_covered.size()
                <= set.elements.size()) {
            sets_by_number_of_elements_covered.push_back({});
        }
        sets_by_number_of_elements_covered[set.elements.size()].push_back(set_id);
    }

    optimizationtools::IndexedSet& sets_to_remove = tmp.indexed_set_2_;
    sets_to_remove.resize_and_clear(tmp.instance.number_of_sets());
    optimizationtools::IndexedSet& covered_elements = tmp.indexed_set_;
    covered_elements.resize_and_clear(tmp.instance.number_of_elements());
    for (ElementPos m = 0;
            m < (ElementPos)sets_by_number_of_elements_covered.size();
            ++m) {

        // Compute sorted sets.
        std::vector<SetId>& sets_sorted = sets_by_number_of_elements_covered[m];
        if (sets_sorted.empty())
            continue;
        sort(sets_sorted.begin(), sets_sorted.end(),
                [this, &tmp](SetId set_id_1, SetId set_id_2) -> bool
                {
                    if (tmp.hashes_[set_id_1] != tmp.hashes_[set_id_2])
                        return tmp.hashes_[set_id_1] < tmp.hashes_[set_id_2];
                    const ReductionSet& set_1 = tmp.instance.set(set_id_1);
                    const ReductionSet& set_2 = tmp.instance.set(set_id_2);
                    return tmp.instance.set(set_id_1).cost > tmp.instance.set(set_id_2).cost;
                });

        for (SetPos set_pos = 0;
                set_pos < (SetPos)sets_sorted.size();
                ++set_pos) {
            SetId set_id = sets_sorted[set_pos];
            const ReductionSet& set = tmp.instance.set(set_id);

            if (set.elements.size() == 0) {
                sets_to_remove.add(set_id);
                continue;
            }

            bool identical = false;
            covered_elements.clear();
            for (SetPos set_pos_prev = set_pos - 1;; --set_pos_prev) {
                if (set_pos_prev < 0)
                    break;
                SetId set_id_prev = sets_sorted[set_pos_prev];
                const ReductionSet& set_prev = tmp.instance.set(set_id_prev);
                if (tmp.hashes_[set_id] != tmp.hashes_[set_id_prev])
                    break;
                if (set.elements.size() != set_prev.elements.size())
                    break;
                if (set.cost != set_prev.cost)
                    break;
                if (covered_elements.empty()) {
                    for (ElementId element_id: set.elements)
                        covered_elements.add(element_id);
                }
                bool identical_cur = true;
                for (ElementId element_id: set_prev.elements) {
                    if (!covered_elements.contains(element_id)) {
                        identical_cur = false;
                        break;
                    }
                }
                if (identical_cur) {
                    identical = true;
                    break;
                }
            }

            if (identical)
                sets_to_remove.add(set_id);
        }
    }

    if (sets_to_remove.size() == 0)
        return false;

    //std::cout << sets_to_remove.size() << std::endl;

    // Update mandatory_sets.
    for (SetId set_id: sets_to_remove)
        for (SetId orig_set_id: unreduction_operations_[set_id].out)
            mandatory_sets_.push_back(orig_set_id);
    // Update sets.
    for (SetId set_id = 0;
            set_id < tmp.instance.number_of_sets();
            ++set_id) {
        ReductionSet& set = tmp.instance.set(set_id);
        if (set.removed)
            continue;
        if (sets_to_remove.contains(set_id))
            set.removed = true;
    }
    // Update elements.
    for (ElementId element_id = 0;
            element_id < tmp.instance.number_of_elements();
            ++element_id) {
        ReductionElement& element = tmp.instance.element(element_id);
        if (element.removed)
            continue;
        for (ElementPos pos = 0;
                pos < (ElementPos)element.sets.size();
                ) {
            ElementId set_id = element.sets[pos];
            if (sets_to_remove.contains(set_id)) {
                element.sets[pos] = element.sets.back();
                element.sets.pop_back();
            } else {
                pos++;
            }
        }
    }

    //check(tmp.instance);
    if (needs_update(tmp.instance))
        update(tmp.instance, unreduction_operations_);
    return true;
}

bool Reduction::reduce_set_folding(Tmp& tmp)
{
    //std::cout << "reduce_set_folding..." << std::endl;

    optimizationtools::IndexedSet& folded_sets = tmp.indexed_set_2_;
    folded_sets.resize_and_clear(tmp.instance.number_of_sets());
    optimizationtools::IndexedSet& sets_to_remove = tmp.indexed_set_5_;
    sets_to_remove.resize_and_clear(tmp.instance.number_of_sets());
    optimizationtools::IndexedSet& set_neighbors = tmp.indexed_set_;
    set_neighbors.resize_and_clear(tmp.instance.number_of_sets());
    optimizationtools::IndexedSet& covered_elements = tmp.indexed_set_3_;
    covered_elements.resize_and_clear(tmp.instance.number_of_elements());
    optimizationtools::IndexedSet& covered_elements_2 = tmp.indexed_set_4_;
    covered_elements_2.resize_and_clear(tmp.instance.number_of_elements());

    std::vector<std::tuple<SetId, SetId, SetId>> folded_sets_list;
    for (SetId set_id = 0;
            set_id < tmp.instance.number_of_sets();
            ++set_id) {
        const ReductionSet& set = tmp.instance.set(set_id);
        if (set.removed)
            continue;

        if (set.elements.size() == 1)
            continue;

        // Check if 'set_id' has only 2 neighbor sets.
        set_neighbors.clear();
        for (ElementId element_id: set.elements) {
            for (SetId neighbor_id_2: tmp.instance.element(element_id).sets) {
                if (neighbor_id_2 == set_id)
                    continue;
                set_neighbors.add(neighbor_id_2);
                if (set_neighbors.size() > 2)
                    break;
            }
            if (set_neighbors.size() > 2)
                break;
        }
        if (set_neighbors.size() > 2)
            continue;

        SetId neighbor_id_1 = *set_neighbors.begin();
        SetId neighbor_id_2 = *(set_neighbors.begin() + 1);
        if (folded_sets.contains(set_id)
                || folded_sets.contains(neighbor_id_1)
                || folded_sets.contains(neighbor_id_2)
                || sets_to_remove.contains(set_id)
                || sets_to_remove.contains(neighbor_id_1)
                || sets_to_remove.contains(neighbor_id_2))
            continue;
        const ReductionSet& neighbor_1 = tmp.instance.set(neighbor_id_1);
        const ReductionSet& neighbor_2 = tmp.instance.set(neighbor_id_2);
        if (set.cost != neighbor_1.cost
                || set.cost != neighbor_2.cost)
            continue;

        // All elements covered by 'set_id' must be covered by 'neighbor_id_1' U 'neighbor_id_2'.
        // All elements covered by 'neighbor_id_1' and 'neighbor_id_2' must be covered by 'set_id'.
        // 'neighbor_id_1' must cover an element covered by 'set_id' that 'neighbor_id_2' doesn't cover.
        // 'neighbor_id_2' must cover an element covered by 'set_id' that 'neighbor_id_1' doesn't cover.

        // Check if 'neighbor_id_1' and 'neighbor_id_2' cover all elements covered by 'set_id'.
        covered_elements.clear();
        for (ElementId element_id: set.elements)
            covered_elements.add(element_id);

        bool ok_1 = false;
        covered_elements_2.clear();
        ElementPos number_of_covered_elements = 0;
        for (ElementId element_id: neighbor_1.elements) {
            if (!covered_elements.contains(element_id))
                ok_1 = true;
            if (covered_elements.contains(element_id))
                number_of_covered_elements++;
            covered_elements_2.add(element_id);
        }
        if (!ok_1)
            continue;
        if (number_of_covered_elements == covered_elements.size())
            continue;
        bool ok_2 = false;
        ElementPos neighbor_2_number_of_covered_elements = 0;
        for (ElementId element_id: neighbor_2.elements) {
            if (!covered_elements.contains(element_id))
                ok_2 = true;
            if (covered_elements.contains(element_id))
                neighbor_2_number_of_covered_elements++;
            if (covered_elements.contains(element_id)
                    && !covered_elements_2.contains(element_id)) {
                number_of_covered_elements++;
            }
            covered_elements_2.add(element_id);
        }
        if (!ok_2)
            continue;
        if (neighbor_2_number_of_covered_elements == covered_elements.size())
            continue;
        if (number_of_covered_elements != covered_elements.size())
            continue;

        folded_sets.add(set_id);
        sets_to_remove.add(neighbor_id_1);
        sets_to_remove.add(neighbor_id_2);
        folded_sets_list.push_back({set_id, neighbor_id_1, neighbor_id_2});
    }

    if (folded_sets_list.empty())
        return false;

    //std::cout << folded_sets_list.size() << std::endl;

    // Update sets.
    optimizationtools::IndexedSet& elements_to_remove = tmp.indexed_set_;
    elements_to_remove.resize_and_clear(tmp.instance.number_of_elements());
    for (const auto& tuple: folded_sets_list) {
        SetId set_id = std::get<0>(tuple);
        SetId neighbor_id_1 = std::get<1>(tuple);
        SetId neighbor_id_2 = std::get<2>(tuple);
        ReductionSet& set = tmp.instance.set(set_id);
        ReductionSet& neighbor_1 = tmp.instance.set(neighbor_id_1);
        ReductionSet& neighbor_2 = tmp.instance.set(neighbor_id_2);

        for (ElementId element_id: set.elements)
            elements_to_remove.add(element_id);

        covered_elements.clear();
        for (ElementId element_id: neighbor_1.elements)
            covered_elements.add(element_id);
        for (ElementId element_id: neighbor_2.elements)
            covered_elements.add(element_id);
        set.elements.clear();
        for (ElementId element_id: covered_elements) {
            set.elements.push_back(element_id);
            ReductionElement& element = tmp.instance.element(element_id);
            element.sets.push_back(set_id);
        }

        unreduction_operations_[set_id].in.swap(
                unreduction_operations_[set_id].out);

        for (SetId orig_set_id: unreduction_operations_[neighbor_id_1].in)
            unreduction_operations_[set_id].in.push_back(orig_set_id);
        for (SetId orig_set_id: unreduction_operations_[neighbor_id_1].out)
            unreduction_operations_[set_id].out.push_back(orig_set_id);

        for (SetId orig_set_id: unreduction_operations_[neighbor_id_2].in)
            unreduction_operations_[set_id].in.push_back(orig_set_id);
        for (SetId orig_set_id: unreduction_operations_[neighbor_id_2].out)
            unreduction_operations_[set_id].out.push_back(orig_set_id);
    }
    for (SetId set_id = 0;
            set_id < tmp.instance.number_of_sets();
            ++set_id) {
        ReductionSet& set = tmp.instance.set(set_id);
        if (set.removed)
            continue;
        if (sets_to_remove.contains(set_id)) {
            set.removed = true;
        } else {
            for (ElementPos pos = 0;
                    pos < (ElementPos)set.elements.size();
                    ) {
                ElementId element_id = set.elements[pos];
                if (elements_to_remove.contains(element_id)) {
                    set.elements[pos] = set.elements.back();
                    set.elements.pop_back();
                } else {
                    pos++;
                }
            }
        }
    }
    // Update elements.
    for (ElementId element_id = 0;
            element_id < tmp.instance.number_of_elements();
            ++element_id) {
        ReductionElement& element = tmp.instance.element(element_id);
        if (element.removed)
            continue;
        if (elements_to_remove.contains(element_id)) {
            element.removed = true;
        } else {
            for (ElementPos pos = 0;
                    pos < (ElementPos)element.sets.size();
                    ) {
                ElementId set_id = element.sets[pos];
                if (sets_to_remove.contains(set_id)) {
                    element.sets[pos] = element.sets.back();
                    element.sets.pop_back();
                } else {
                    pos++;
                }
            }
        }
    }

    //check(tmp.instance);
    if (needs_update(tmp.instance))
        update(tmp.instance, unreduction_operations_);
    return true;
}

struct ReductionTwinCandidate
{
    SetId set_id;
    std::vector<SetId> neighbor_ids;
};

struct ReductionTwin
{
    SetId set_id_1;
    SetId set_id_2;
    std::vector<SetId> neighbor_ids;
};

bool Reduction::reduce_twin(Tmp& tmp)
{
    //std::cout << "reduce_twin..." << std::endl;

    // Find all sets with exactly 3 neighbors.
    std::vector<ReductionTwinCandidate> twin_candidates;
    optimizationtools::IndexedSet& set_neighbors = tmp.indexed_set_;
    set_neighbors.resize_and_clear(tmp.instance.number_of_sets());
    optimizationtools::IndexedSet& covered_elements = tmp.indexed_set_3_;
    covered_elements.resize_and_clear(tmp.instance.number_of_elements());
    optimizationtools::IndexedSet& neighbors_elements = tmp.indexed_set_2_;
    neighbors_elements.resize_and_clear(tmp.instance.number_of_elements());
    for (SetId set_id = 0;
            set_id < tmp.instance.number_of_sets();
            ++set_id) {
        const ReductionSet& set = tmp.instance.set(set_id);
        if (set.removed)
            continue;
        set_neighbors.clear();
        for (ElementId element_id: set.elements) {
            for (SetId set_id_2: tmp.instance.element(element_id).sets) {
                if (set_id_2 == set_id)
                    continue;
                set_neighbors.add(set_id_2);
                if (set_neighbors.size() > 3)
                    break;
            }
            if (set_neighbors.size() > 3)
                break;
        }
        if (set_neighbors.size() != 3)
            continue;

        // Check that all 3 neighbors are necessary to cover elements of set_1
        // if set_1 is not taken.
        SetId neighbors_id_1 = *(set_neighbors.begin() + 0);
        SetId neighbors_id_2 = *(set_neighbors.begin() + 1);
        SetId neighbors_id_3 = *(set_neighbors.begin() + 2);
        const ReductionSet& neighbor_1 = tmp.instance.set(neighbors_id_1);
        const ReductionSet& neighbor_2 = tmp.instance.set(neighbors_id_2);
        const ReductionSet& neighbor_3 = tmp.instance.set(neighbors_id_3);
        covered_elements.clear();
        for (ElementId element_id: set.elements)
            covered_elements.add(element_id);
        {
            neighbors_elements.clear();
            for (ElementId element_id: neighbor_1.elements)
                if (covered_elements.contains(element_id))
                    neighbors_elements.add(element_id);
            for (ElementId element_id: neighbor_2.elements)
                if (covered_elements.contains(element_id))
                    neighbors_elements.add(element_id);
            if (neighbors_elements.size() == covered_elements.size())
                continue;
        }
        {
            neighbors_elements.clear();
            for (ElementId element_id: neighbor_1.elements)
                if (covered_elements.contains(element_id))
                    neighbors_elements.add(element_id);
            for (ElementId element_id: neighbor_3.elements)
                if (covered_elements.contains(element_id))
                    neighbors_elements.add(element_id);
            if (neighbors_elements.size() == covered_elements.size())
                continue;
        }
        {
            neighbors_elements.clear();
            for (ElementId element_id: neighbor_2.elements)
                if (covered_elements.contains(element_id))
                    neighbors_elements.add(element_id);
            for (ElementId element_id: neighbor_3.elements)
                if (covered_elements.contains(element_id))
                    neighbors_elements.add(element_id);
            if (neighbors_elements.size() == covered_elements.size())
                continue;
        }

        ReductionTwinCandidate twin_candidate;
        twin_candidate.set_id = set_id;
        twin_candidate.neighbor_ids.push_back(*(set_neighbors.begin() + 0));
        twin_candidate.neighbor_ids.push_back(*(set_neighbors.begin() + 1));
        twin_candidate.neighbor_ids.push_back(*(set_neighbors.begin() + 2));
        std::sort(twin_candidate.neighbor_ids.begin(), twin_candidate.neighbor_ids.end());
        twin_candidates.push_back(twin_candidate);
    }
    //std::cout << "twin_candidates.size() " << twin_candidates.size() << std::endl;

    // Sort by neighbors triplets.
    sort(twin_candidates.begin(), twin_candidates.end(),
            [this](
                const ReductionTwinCandidate& twin_candidate_1,
                const ReductionTwinCandidate& twin_candidate_2) -> bool
            {
                if (twin_candidate_1.neighbor_ids[0] != twin_candidate_2.neighbor_ids[0])
                    return (twin_candidate_1.neighbor_ids[0] < twin_candidate_2.neighbor_ids[0]);
                if (twin_candidate_1.neighbor_ids[1] != twin_candidate_2.neighbor_ids[1])
                    return (twin_candidate_1.neighbor_ids[1] < twin_candidate_2.neighbor_ids[1]);
                return (twin_candidate_1.neighbor_ids[2] < twin_candidate_2.neighbor_ids[2]);
            });

    // Fold each pair.
    optimizationtools::IndexedSet& folded_sets = tmp.indexed_set_2_;
    folded_sets.resize_and_clear(tmp.instance.number_of_sets());
    optimizationtools::IndexedSet& sets_to_remove = tmp.indexed_set_5_;
    sets_to_remove.resize_and_clear(tmp.instance.number_of_sets());
    std::vector<ReductionTwin> folded_sets_list;
    for (SetPos pos = 1;
            pos < (SetPos)twin_candidates.size();
            ++pos) {
        const ReductionTwinCandidate& twin_candidate_1 = twin_candidates[pos];
        const ReductionTwinCandidate& twin_candidate_2 = twin_candidates[pos - 1];
        if (twin_candidate_1.neighbor_ids != twin_candidate_2.neighbor_ids)
            continue;

        if (folded_sets.contains(twin_candidate_1.set_id)
                || folded_sets.contains(twin_candidate_2.set_id)
                || folded_sets.contains(twin_candidate_1.neighbor_ids[0])
                || folded_sets.contains(twin_candidate_1.neighbor_ids[1])
                || folded_sets.contains(twin_candidate_1.neighbor_ids[2])
                || sets_to_remove.contains(twin_candidate_1.set_id)
                || sets_to_remove.contains(twin_candidate_2.set_id)
                || sets_to_remove.contains(twin_candidate_1.neighbor_ids[0])
                || sets_to_remove.contains(twin_candidate_1.neighbor_ids[1])
                || sets_to_remove.contains(twin_candidate_1.neighbor_ids[2])) {
            continue;
        }

        const ReductionSet& set_1 = tmp.instance.set(twin_candidate_1.set_id);
        const ReductionSet& set_2 = tmp.instance.set(twin_candidate_2.set_id);
        const ReductionSet& neighbor_1 = tmp.instance.set(twin_candidate_1.neighbor_ids[0]);
        const ReductionSet& neighbor_2 = tmp.instance.set(twin_candidate_1.neighbor_ids[1]);
        const ReductionSet& neighbor_3 = tmp.instance.set(twin_candidate_1.neighbor_ids[2]);
        if (set_2.cost != set_1.cost
                || neighbor_1.cost != set_1.cost
                || neighbor_2.cost != set_1.cost
                || neighbor_3.cost != set_1.cost) {
            continue;
        }

        folded_sets.add(twin_candidate_1.set_id);
        sets_to_remove.add(twin_candidate_2.set_id);
        sets_to_remove.add(twin_candidate_1.neighbor_ids[0]);
        sets_to_remove.add(twin_candidate_1.neighbor_ids[1]);
        sets_to_remove.add(twin_candidate_1.neighbor_ids[2]);
        ReductionTwin twin;
        twin.set_id_1 = twin_candidate_1.set_id;
        twin.set_id_2 = twin_candidate_2.set_id;
        twin.neighbor_ids = twin_candidate_1.neighbor_ids;
        folded_sets_list.push_back(twin);
    }

    if (folded_sets_list.empty())
        return false;

    //std::cout << folded_sets_list.size() << std::endl;

    // Update sets.
    optimizationtools::IndexedSet& elements_to_remove = tmp.indexed_set_;
    elements_to_remove.resize_and_clear(tmp.instance.number_of_elements());
    for (const ReductionTwin& twin: folded_sets_list) {
        ReductionSet& set_1 = tmp.instance.set(twin.set_id_1);
        ReductionSet& set_2 = tmp.instance.set(twin.set_id_2);
        ReductionSet& neighbor_1 = tmp.instance.set(twin.neighbor_ids[0]);
        ReductionSet& neighbor_2 = tmp.instance.set(twin.neighbor_ids[1]);
        ReductionSet& neighbor_3 = tmp.instance.set(twin.neighbor_ids[2]);

        for (ElementId element_id: set_1.elements)
            elements_to_remove.add(element_id);
        for (ElementId element_id: set_2.elements)
            elements_to_remove.add(element_id);

        covered_elements.clear();
        for (ElementId element_id: neighbor_1.elements)
            covered_elements.add(element_id);
        for (ElementId element_id: neighbor_2.elements)
            covered_elements.add(element_id);
        for (ElementId element_id: neighbor_3.elements)
            covered_elements.add(element_id);
        set_1.elements.clear();
        for (ElementId element_id: covered_elements) {
            set_1.elements.push_back(element_id);
            ReductionElement& element = tmp.instance.element(element_id);
            element.sets.push_back(twin.set_id_1);
        }

        unreduction_operations_[twin.set_id_1].in.swap(
                unreduction_operations_[twin.set_id_1].out);

        for (SetId orig_set_id: unreduction_operations_[twin.set_id_2].out)
            unreduction_operations_[twin.set_id_1].in.push_back(orig_set_id);
        for (SetId orig_set_id: unreduction_operations_[twin.set_id_2].in)
            unreduction_operations_[twin.set_id_1].out.push_back(orig_set_id);

        for (SetId orig_set_id: unreduction_operations_[twin.neighbor_ids[0]].in)
            unreduction_operations_[twin.set_id_1].in.push_back(orig_set_id);
        for (SetId orig_set_id: unreduction_operations_[twin.neighbor_ids[0]].out)
            unreduction_operations_[twin.set_id_1].out.push_back(orig_set_id);
        for (SetId orig_set_id: unreduction_operations_[twin.neighbor_ids[1]].in)
            unreduction_operations_[twin.set_id_1].in.push_back(orig_set_id);
        for (SetId orig_set_id: unreduction_operations_[twin.neighbor_ids[1]].out)
            unreduction_operations_[twin.set_id_1].out.push_back(orig_set_id);
        for (SetId orig_set_id: unreduction_operations_[twin.neighbor_ids[2]].in)
            unreduction_operations_[twin.set_id_1].in.push_back(orig_set_id);
        for (SetId orig_set_id: unreduction_operations_[twin.neighbor_ids[2]].out)
            unreduction_operations_[twin.set_id_1].out.push_back(orig_set_id);
    }
    for (SetId set_id = 0;
            set_id < tmp.instance.number_of_sets();
            ++set_id) {
        ReductionSet& set = tmp.instance.set(set_id);
        if (set.removed)
            continue;
        if (sets_to_remove.contains(set_id)) {
            set.removed = true;
        } else {
            for (ElementPos pos = 0;
                    pos < (ElementPos)set.elements.size();
                    ) {
                ElementId element_id = set.elements[pos];
                if (elements_to_remove.contains(element_id)) {
                    set.elements[pos] = set.elements.back();
                    set.elements.pop_back();
                } else {
                    pos++;
                }
            }
        }
    }
    // Update elements.
    for (ElementId element_id = 0;
            element_id < tmp.instance.number_of_elements();
            ++element_id) {
        ReductionElement& element = tmp.instance.element(element_id);
        if (element.removed)
            continue;
        if (elements_to_remove.contains(element_id)) {
            element.removed = true;
        } else {
            for (ElementPos pos = 0;
                    pos < (ElementPos)element.sets.size();
                    ) {
                ElementId set_id = element.sets[pos];
                if (sets_to_remove.contains(set_id)) {
                    element.sets[pos] = element.sets.back();
                    element.sets.pop_back();
                } else {
                    pos++;
                }
            }
        }
    }

    //check(tmp.instance);
    if (needs_update(tmp.instance))
        update(tmp.instance, unreduction_operations_);
    return true;
}

using EdgeId = int64_t;
using VertexId = int64_t;

struct Edge
{
    VertexId vertex_id_1;
    VertexId vertex_id_2;
};

std::vector<uint8_t> bipartite_graph_maximum_matching(
        const std::vector<uint8_t>& vertices_side,
        const std::vector<Edge>& edges)
{
    VertexId number_of_vertices = vertices_side.size();
    std::cout << "bipartite_graph_maximum_matching " << number_of_vertices << " " << edges.size() << std::endl;
    // Find an initial maximal matching.
    std::vector<VertexId> vertices_matched_edge(number_of_vertices, -1);
    std::vector<uint8_t> edges_matched(edges.size(), 0);
    EdgeId matching_size = 0;
    for (EdgeId edge_id = 0;
            edge_id < (EdgeId)edges.size();
            ++edge_id) {
        const Edge& edge = edges[edge_id];
        if (vertices_matched_edge[edge.vertex_id_1] != -1)
            continue;
        if (vertices_matched_edge[edge.vertex_id_2] != -1)
            continue;
        edges_matched[edge_id] = 1;
        vertices_matched_edge[edge.vertex_id_1] = edge_id;
        vertices_matched_edge[edge.vertex_id_2] = edge_id;
        matching_size++;
    }
    std::cout << "matching_size " << matching_size << std::endl;
    if (matching_size > edges.size()) {
        throw std::logic_error("");
    }
    if (matching_size == number_of_vertices / 2)
        return edges_matched;

    std::vector<std::vector<EdgeId>> vertices_edges(number_of_vertices);
    for (EdgeId edge_id = 0;
            edge_id < (EdgeId)edges.size();
            ++edge_id) {
        const Edge& edge = edges[edge_id];
        vertices_edges[edge.vertex_id_1].push_back(edge_id);
        vertices_edges[edge.vertex_id_2].push_back(edge_id);
    }

    std::vector<VertexId> bfs_queue(number_of_vertices, -1);
    std::vector<VertexId> dist(number_of_vertices, number_of_vertices + 1);
    std::vector<EdgeId> pred(number_of_vertices, -1);
    std::vector<VertexId> unmatched_leaves;
    std::vector<VertexId> path;
    std::vector<uint8_t> vertices_processed(number_of_vertices, 0);
    for (;;) {
        std::cout << "it matching_size " << matching_size << std::endl;
        // Breadth first search.
        // Find all unmatched vertices in i.
        std::fill(dist.begin(), dist.end(), number_of_vertices + 1);
        unmatched_leaves.clear();
        VertexId queue_push_pos = 0;
        for (VertexId vertex_id = 0;
                vertex_id < number_of_vertices;
                ++vertex_id) {
            if (vertices_side[vertex_id] == 1)
                continue;
            if (vertices_matched_edge[vertex_id] == -1) {
                dist[vertex_id] = 0;
                bfs_queue[queue_push_pos] = vertex_id;
                queue_push_pos++;
            }
        }
        for (VertexId queue_pop_pos = 0;
                queue_pop_pos < queue_push_pos;
                ++queue_pop_pos) {
            VertexId vertex_id = bfs_queue[queue_pop_pos];
            bool has_child = false;
            for (EdgeId edge_id: vertices_edges[vertex_id]) {
                if (dist[vertex_id] % 2 == 0) {
                    if (edges_matched[edge_id] == 1)
                        continue;
                } else {
                    if (edges_matched[edge_id] == 0)
                        continue;
                }
                const Edge& edge = edges[edge_id];
                VertexId other_vertex_id = (edge.vertex_id_1 == vertex_id)?
                    edge.vertex_id_2:
                    edge.vertex_id_1;
                if (dist[other_vertex_id] <= dist[vertex_id] + 1)
                    continue;
                dist[other_vertex_id] = dist[vertex_id] + 1;
                pred[other_vertex_id] = edge_id;
                bfs_queue[queue_push_pos] = other_vertex_id;
                queue_push_pos++;
                has_child = true;
            }
            if (vertices_matched_edge[vertex_id] == -1
                    && dist[vertex_id] > 0
                    && !has_child) {
                unmatched_leaves.push_back(vertex_id);
            }
        }

        std::cout << "retrieve paths..." << std::endl;
        std::fill(vertices_processed.begin(), vertices_processed.end(), 0);
        bool found = false;
        for (VertexId leaf_vertex_id: unmatched_leaves) {
            bool ok = true;
            path.clear();
            VertexId vertex_id = leaf_vertex_id;
            while (pred[vertex_id] != -1) {
                EdgeId edge_id = pred[vertex_id];
                path.push_back(edge_id);
                const Edge& edge = edges[edge_id];
                vertex_id = (edge.vertex_id_1 == vertex_id)?
                    edge.vertex_id_2:
                    edge.vertex_id_1;
                if (vertices_processed[vertex_id]) {
                    ok = false;
                    break;
                }
            }
            if (!ok)
                continue;

            std::cout << "path";
            for (EdgeId edge_id: path) {
                const Edge& edge = edges[edge_id];
                std::cout << " " << edge_id
                    << "," << edge.vertex_id_1
                    << "," << edge.vertex_id_2;
            }
            std::cout << std::endl;

            // Apply path.
            for (EdgeId edge_id: path) {
                const Edge& edge = edges[edge_id];
                if (edges_matched[edge_id] == 1) {
                    edges_matched[edge_id] = 0;
                } else {
                    edges_matched[edge_id] = 1;
                    vertices_matched_edge[edge.vertex_id_1] = edge_id;
                    vertices_matched_edge[edge.vertex_id_2] = edge_id;
                }
                vertices_processed[edge.vertex_id_1] = 1;
                vertices_processed[edge.vertex_id_2] = 1;
            }
            matching_size++;
            if (matching_size > edges.size()) {
                throw std::logic_error("");
            }
            found = true;
        }
        if (!found)
            break;
    }
    return edges_matched;
}

bool Reduction::reduce_crown(Tmp& tmp)
{
    std::cout << "reduce_crown..." << std::endl;

    optimizationtools::IndexedSet& sets_to_remove = tmp.indexed_set_5_;
    sets_to_remove.resize_and_clear(tmp.instance.number_of_sets());
    optimizationtools::IndexedSet& fixed_sets = tmp.indexed_set_6_;
    fixed_sets.resize_and_clear(tmp.instance.number_of_sets());
    optimizationtools::IndexedSet& elements_to_remove = tmp.indexed_set_7_;
    elements_to_remove.resize_and_clear(tmp.instance.number_of_elements());

    std::vector<ElementId>& shuffled_elements = tmp.set_;
    shuffled_elements.resize(tmp.instance.number_of_elements());
    std::iota(shuffled_elements.begin(), shuffled_elements.end(), 0);

    for (Counter it = 0; it < 16; ++it) {
        std::cout << "it " << it << std::endl;

        std::shuffle(shuffled_elements.begin(), shuffled_elements.end(), tmp.generator_);

        // Find a matching in the hypergraph.
        std::cout << "find matching 1..." << std::endl;
        optimizationtools::IndexedSet& matching_1_sets = tmp.indexed_set_;
        matching_1_sets.resize_and_clear(tmp.instance.number_of_sets());
        ElementId matching_1_number_of_elements = 0;
        // First loop through elements with degree > 2.
        for (ElementId element_id: shuffled_elements) {
            const ReductionElement& element = tmp.instance.element(element_id);
            if (element.removed)
                continue;
            if (element.sets.size() <= 2)
                continue;
            bool valid = true;
            for (SetId set_id: element.sets) {
                if (matching_1_sets.contains(set_id)) {
                    valid = false;
                    break;
                }
            }
            if (!valid)
                continue;
            for (SetId set_id: element.sets)
                matching_1_sets.add(set_id);
            matching_1_number_of_elements++;
        }
        // Then loop through element with degree == 2.
        for (ElementId element_id: shuffled_elements) {
            const ReductionElement& element = tmp.instance.element(element_id);
            if (element.removed)
                continue;
            if (element.sets.size() != 2)
                continue;
            bool valid = true;
            for (SetId set_id: element.sets) {
                if (matching_1_sets.contains(set_id)) {
                    valid = false;
                    break;
                }
            }
            if (!valid)
                continue;
            for (SetId set_id: element.sets)
                matching_1_sets.add(set_id);
            matching_1_number_of_elements++;
        }
        std::cout << "matching_1_number_of_elements " << matching_1_number_of_elements << std::endl;
        std::cout << "matching_1_sets.size() " << matching_1_sets.size() << std::endl;

        // Find the unmatched sets that only cover elements covered by two sets.
        std::cout << "find outsiders" << std::endl;
        optimizationtools::IndexedSet& outsiders = tmp.indexed_set_2_;
        outsiders.resize_and_clear(tmp.instance.number_of_sets());
        for (auto it = matching_1_sets.out_begin();
                it != matching_1_sets.out_end();
                ++it) {
            SetId set_id = *it;
            const ReductionSet& set = tmp.instance.set(set_id);
            if (set.removed)
                continue;
            bool ok = true;
            for (ElementId element_id: set.elements) {
                const ReductionElement& element = tmp.instance.element(element_id);
                if (element.sets.size() != 2) {
                    ok = false;
                    break;
                }
            }
            if (!ok)
                continue;
            outsiders.add(set_id);
        }
        std::cout << "outsiders.size() " << outsiders.size() << std::endl;
        //for (SetId set_id: outsiders) {
        //    std::cout << "set_id " << set_id
        //        << " elts " << tmp.instance.set(set_id).elements.size()
        //        << std::endl;
        //}
        if (outsiders.empty())
            continue;

        // Find the neighbors of the unmatched elements.
        std::cout << "find outsider neighbors..." << std::endl;
        optimizationtools::IndexedSet& outsider_neighbors = tmp.indexed_set_3_;
        outsider_neighbors.resize_and_clear(tmp.instance.number_of_sets());
        for (SetId set_id: outsiders) {
            const ReductionSet& set = tmp.instance.set(set_id);
            for (ElementId element_id: set.elements) {
                const ReductionElement& element = tmp.instance.element(element_id);
                SetId other_set_id = (set_id == element.sets[0])?
                    element.sets[1]:
                    element.sets[0];
                if (outsiders.contains(other_set_id)) {
                    throw std::logic_error(
                            "setcoveringsolver::Reduction::reduce_crown.");
                }
                outsider_neighbors.add(other_set_id);
            }
        }
        std::cout << "outsider_neighbors.size() " << outsider_neighbors.size() << std::endl;
        //for (SetId set_id: outsider_neighbors) {
        //    std::cout << "set_id " << set_id
        //        << " elts " << tmp.instance.set(set_id).elements.size()
        //        << std::endl;
        //}

        // Step 2: Find a maximum auxiliary matching M2 of the edges between O and N (O).

        std::cout << "find second matching" << std::endl;
        optimizationtools::IndexedMap<SetPos>& sets_to_bgmm = tmp.indexed_map_;
        sets_to_bgmm.clear();
        VertexId vertex_id = 0;
        std::vector<uint8_t> bgmm_vertices_sides;
        for (SetId set_id: outsiders) {
            sets_to_bgmm.set(set_id, vertex_id);
            bgmm_vertices_sides.push_back(0);
            vertex_id++;
        }
        for (SetId set_id: outsider_neighbors) {
            sets_to_bgmm.set(set_id, vertex_id);
            bgmm_vertices_sides.push_back(1);
            vertex_id++;
        }
        std::vector<Edge> bgmm_edges;
        std::vector<ElementId> bgmm_edges_to_elements;
        for (SetId set_id: outsiders) {
            const ReductionSet& set = tmp.instance.set(set_id);
            for (ElementId element_id: set.elements) {
                const ReductionElement& element = tmp.instance.element(element_id);
                Edge bgmm_edge;
                SetId other_set_id = (set_id == element.sets[0])?
                    element.sets[1]:
                    element.sets[0];
                bgmm_edge.vertex_id_1 = sets_to_bgmm[set_id];
                bgmm_edge.vertex_id_2 = sets_to_bgmm[other_set_id];
                bgmm_edges.push_back(bgmm_edge);
                bgmm_edges_to_elements.push_back(element_id);
            }
        }
        std::vector<uint8_t> bgmm_output = bipartite_graph_maximum_matching(
                bgmm_vertices_sides,
                bgmm_edges);
        optimizationtools::IndexedSet& matching_2 = tmp.indexed_set_4_;
        matching_2.resize_and_clear(tmp.instance.number_of_elements());
        for (EdgeId edge_id = 0;
                edge_id < (EdgeId)bgmm_edges.size();
                ++edge_id) {
            if (bgmm_output[edge_id] == 0)
                continue;
            ElementId element_id = bgmm_edges_to_elements[edge_id];
            matching_2.add(element_id);
        }
        std::cout << "matching_2.size() " << matching_2.size() << std::endl;

        // Step 3: If every vertex in N (O) is matched by M2, then H = N (O) and I = O form a  crown, and we are done.
        if (matching_2.size() == outsider_neighbors.size()) {
            std::cout << "every vertex is matched" << std::endl;
            for (SetId set_id: outsiders)
                sets_to_remove.add(set_id);
            for (SetId set_id: outsider_neighbors) {
                const ReductionSet& set = tmp.instance.set(set_id);
                sets_to_remove.add(set_id);
                fixed_sets.add(set_id);
                for (ElementId element_id: set.elements)
                    elements_to_remove.add(element_id);
            }

        } else {

            // Step 4: Let I0 be the set of vertices in O that are unmatched by M2.
            std::cout << "build i0..." << std::endl;
            optimizationtools::IndexedSet& i = tmp.indexed_set_;
            i.resize_and_clear(tmp.instance.number_of_elements());

            for (SetId set_id: outsiders) {
                const ReductionSet& set = tmp.instance.set(set_id);
                bool inside_matching = false;
                for (ElementId element_id: set.elements) {
                    if (matching_2.contains(element_id)) {
                        inside_matching = true;
                        break;
                    }
                }
                if (!inside_matching) {
                    if (i.contains(set_id)) {
                        throw std::logic_error("");
                    }
                    i.add(set_id);
                }
            }
            std::cout << "i0.size() " << i.size() << std::endl;
            if (i.empty())
                continue;

            optimizationtools::IndexedSet& h = tmp.indexed_set_2_;
            h.resize_and_clear(tmp.instance.number_of_elements());

            // Step 5: Repeat steps 5a and 5b until n = N so that IN−1 = IN .
            for (;;) {
                // 5a. Let Hn = N (In). 
                std::cout << "build h..." << std::endl;
                h.clear();
                for (SetId set_id: i) {
                    const ReductionSet& set = tmp.instance.set(set_id);
                    for (ElementId element_id: set.elements) {
                        const ReductionElement& element = tmp.instance.element(element_id);
                        SetId other_set_id = (set_id == element.sets[0])?
                            element.sets[1]:
                            element.sets[0];
                        if (i.contains(other_set_id)) {
                            throw std::logic_error(
                                    "setcoveringsolver::Reduction::reduce_crown.");
                        }
                        h.add(other_set_id);
                    }
                }
                std::cout << "h.size() " << h.size() << std::endl;
                // 5b. Let In+1 = In ∪ NM2(Hn).
                std::cout << "build i..." << std::endl;
                bool added = false;
                for (SetId set_id: h) {
                    const ReductionSet& set = tmp.instance.set(set_id);
                    for (ElementId element_id: set.elements) {
                        if (matching_2.contains(element_id)) {
                            const ReductionElement& element = tmp.instance.element(element_id);
                            SetId other_set_id = (set_id == element.sets[0])?
                                element.sets[1]:
                                element.sets[0];
                            if (!i.contains(other_set_id)) {
                                i.add(other_set_id);
                                added = true;
                            }
                        }
                    }
                }
                std::cout << "i.size() " << i.size() << std::endl;
                if (!added)
                    break;
            }

            // Step 6: I = IN and H = HN form a flared crown.
            std::cout << "crown found..." << std::endl;
            for (SetId set_id: i)
                sets_to_remove.add(set_id);
            for (SetId set_id: h) {
                const ReductionSet& set = tmp.instance.set(set_id);
                sets_to_remove.add(set_id);
                fixed_sets.add(set_id);
                for (ElementId element_id: set.elements)
                    elements_to_remove.add(element_id);
            }

            std::cout << "i" << std::endl;
            for (SetId set_id: i) {
                std::cout << "set_id " << set_id
                    << " elts " << tmp.instance.set(set_id).elements.size()
                    << std::endl;
            }

            std::cout << "h" << std::endl;
            for (SetId set_id: h) {
                std::cout << "set_id " << set_id
                    << " elts " << tmp.instance.set(set_id).elements.size()
                    << std::endl;
            }

        }

        if (!sets_to_remove.empty())
            break;
    }

    if (sets_to_remove.size() == 0)
        return false;

    std::cout << sets_to_remove.size() << " " << fixed_sets.size() << " " << elements_to_remove.size() << std::endl;

    // Update mandatory_sets.
    for (SetId set_id: sets_to_remove) {
        if (fixed_sets.contains(set_id)) {
            for (SetId orig_set_id: unreduction_operations_[set_id].in)
                mandatory_sets_.push_back(orig_set_id);
        } else {
            for (SetId orig_set_id: unreduction_operations_[set_id].out)
                mandatory_sets_.push_back(orig_set_id);
        }
    }
    // Update sets.
    for (SetId set_id = 0;
            set_id < tmp.instance.number_of_sets();
            ++set_id) {
        ReductionSet& set = tmp.instance.set(set_id);
        if (set.removed)
            continue;
        if (sets_to_remove.contains(set_id)) {
            set.removed = true;
        } else {
            for (ElementPos pos = 0;
                    pos < (ElementPos)set.elements.size();
                    ) {
                ElementId element_id = set.elements[pos];
                if (elements_to_remove.contains(element_id)) {
                    set.elements[pos] = set.elements.back();
                    set.elements.pop_back();
                } else {
                    pos++;
                }
            }
        }
    }
    // Update elements.
    for (ElementId element_id = 0;
            element_id < tmp.instance.number_of_elements();
            ++element_id) {
        ReductionElement& element = tmp.instance.element(element_id);
        if (element.removed)
            continue;
        if (elements_to_remove.contains(element_id)) {
            element.removed = true;
        } else {
            for (SetPos pos = 0;
                    pos < (SetPos)element.sets.size();
                    ) {
                SetId set_id = element.sets[pos];
                if (sets_to_remove.contains(set_id)) {
                    element.sets[pos] = element.sets.back();
                    element.sets.pop_back();
                } else {
                    pos++;
                }
            }
        }
    }

    //check(tmp.instance);
    if (needs_update(tmp.instance))
        update(tmp.instance, unreduction_operations_);
    return true;
}

bool Reduction::reduce_unconfined_sets(Tmp& tmp)
{
    //std::cout << "reduce_unconfined_sets..." << std::endl;

    optimizationtools::IndexedSet& fixed_sets = tmp.indexed_set_;
    fixed_sets.resize_and_clear(tmp.instance.number_of_sets());
    optimizationtools::IndexedSet& elements_to_remove = tmp.indexed_set_2_;
    elements_to_remove.resize_and_clear(tmp.instance.number_of_elements());
    // Set of sets S containing sets that are assumed to be in no optimal
    // solution.
    optimizationtools::IndexedSet& s = tmp.indexed_set_3_;
    s.resize_and_clear(tmp.instance.number_of_sets());
    // Set of sets neighbors of S which are mandatory because one of the
    // elements they cover is only covered by sets of S.
    optimizationtools::IndexedSet& ns_mandatory = tmp.indexed_set_5_;
    ns_mandatory.resize_and_clear(tmp.instance.number_of_sets());
    // Sets of ns_mandatory such that, for a set of ns_candidates, if we
    // consider all the elements that it covers which are also covered by S,
    // there is a single set of S that covers them.
    std::vector<SetId> ns_candidates = tmp.set_;
    ns_candidates.clear();
    // Elements covered by S.
    optimizationtools::IndexedSet& s_covered_elements = tmp.indexed_set_6_;
    s_covered_elements.resize_and_clear(tmp.instance.number_of_elements());
    // Elements covered by S or ns_mandatory.
    optimizationtools::IndexedMap<SetPos>& s_ns_mandatory_covered_elements = tmp.indexed_map_;
    s_ns_mandatory_covered_elements.resize_and_clear(tmp.instance.number_of_elements());
    optimizationtools::IndexedSet& covered_elements = tmp.indexed_set_7_;
    covered_elements.resize_and_clear(tmp.instance.number_of_elements());

    for (SetId set_id = 0;
            set_id < tmp.instance.number_of_sets();
            ++set_id) {
        const ReductionSet& set = tmp.instance.set(set_id);
        if (set.removed)
            continue;

        //std::cout << "set_id " << set_id << " elts";
        //for (ElementId element_id: set.elements)
        //    std::cout << " " << element_id;
        //std::cout << std::endl;

        s.clear();
        s_covered_elements.clear();

        bool fixed = false;
        SetId set_v_id = set_id;

        for (;;) {
            const ReductionSet& set_v = tmp.instance.sets[set_v_id];

            // Update S.
            s.add(set_v_id);

            // Update s_covered_elements.
            for (ElementId element_id: set_v.elements) {
                if (elements_to_remove.contains(element_id))
                    continue;
                s_covered_elements.add(element_id);
            }

            //std::cout << "set_v_id " << set_v_id << std::endl;

            // Compute ns_mandatory and ns_candidates.
            ns_mandatory.clear();
            s_ns_mandatory_covered_elements.clear();
            ns_candidates.clear();
            for (SetId set_v0_id: s) {
                const ReductionSet& set_v0 = tmp.instance.sets[set_v0_id];

                for (ElementId element_id: set_v0.elements) {
                    if (elements_to_remove.contains(element_id))
                        continue;
                    const ReductionElement& element = tmp.instance.elements[element_id];

                    s_ns_mandatory_covered_elements.set(
                            element_id,
                            s_ns_mandatory_covered_elements[element_id] + 1);

                    // For ns_mandatory, look for elements which are covered by a
                    // single set outside of S.
                    SetPos n = 0;
                    SetId set_u_id = -1;
                    for (SetId set_id_cur: element.sets) {
                        if (s.contains(set_id_cur))
                            continue;
                        n++;
                        if (n > 1)
                            break;
                        set_u_id = set_id_cur;
                    }
                    if (n == 0 || n > 1)
                        continue;
                    // The set is already in ns_mandatory.
                    if (ns_mandatory.contains(set_u_id))
                        continue;
                    if (fixed_sets.contains(set_u_id)) {
                        throw std::logic_error(
                                "setcoveringsolver::Reduction::reduce_unconfined_sets: "
                                "'fixed_sets' should not contain 'set_u_id'.");
                    }
                    if (s.contains(set_u_id)) {
                        throw std::logic_error(
                                "setcoveringsolver::Reduction::reduce_unconfined_sets: "
                                "'s' should not contain 'set_u_id'.");
                    }
                    ns_mandatory.add(set_u_id);

                    // Update s_ns_mandatory_covered_elements.
                    const ReductionSet& set_u = tmp.instance.sets[set_u_id];
                    for (ElementId element_id_3: set_u.elements) {
                        if (elements_to_remove.contains(element_id_3))
                            continue;
                        s_ns_mandatory_covered_elements.set(
                                element_id_3,
                                s_ns_mandatory_covered_elements[element_id_3] + 1);

                        //std::cout << " s_ns_elts";
                        //for (const auto& p: s_ns_mandatory_covered_elements)
                        //    std::cout << " " << p.first << "," << p.second;
                        //std::cout << std::endl;

                        if (s_ns_mandatory_covered_elements.size() > 128)
                            break;
                    }
                    // if s_ns_mandatory_covered_elements is too large, skip.
                    if (s_ns_mandatory_covered_elements.size() > 128)
                        break;

                    // Check if set_u_id may be added to ns_candidates.
                    // Check if all elements covered by u and by S are covered by
                    // a single set of S.

                    // Find all elements covered by u and by S.
                    covered_elements.clear();
                    for (ElementId element_id_2: set_u.elements) {
                        if (elements_to_remove.contains(element_id_2))
                            continue;
                        if (s_covered_elements.contains(element_id_2))
                            covered_elements.add(element_id_2);
                    }

                    // Check if there is a set set_id_3 from S that covers all these
                    // elements.
                    bool ok = false;
                    for (SetId set_v2_id: s) {
                        if (fixed_sets.contains(set_v2_id)) {
                            throw std::logic_error(
                                    "setcoveringsolver::Reduction::reduce_unconfined_sets.");
                        }
                        // We hope to show that u may be replace by v2 in an
                        // optimal solution. Therefore, we need
                        // cost(u) >= cost(v2).
                        const ReductionSet& set_v2 = tmp.instance.sets[set_v2_id];
                        if (set_u.cost < set_v2.cost)
                            continue;
                        ElementPos m = 0;
                        for (ElementId element_id_3: set_v2.elements) {
                            if (elements_to_remove.contains(element_id_3))
                                continue;
                            if (covered_elements.contains(element_id_3))
                                m++;
                        }
                        if (m == covered_elements.size()) {
                            ok = true;
                            break;
                        }
                    }
                    if (ok)
                        ns_candidates.push_back(set_u_id);
                }
            }
            if (s_ns_mandatory_covered_elements.size() > 128)
                break;

            //std::cout << " s_ns_elts";
            //for (const auto& p: s_ns_mandatory_covered_elements)
            //    std::cout << " " << p.first << "," << p.second;
            //std::cout << std::endl;

            // Find the candidate set with the least number of elements outside
            // of s_ns_mandatory_covered_elements.
            SetId set_u_id_best = -1;
            SetId set_w_id = -1;
            for (SetId set_u_id: ns_candidates) {
                if (fixed_sets.contains(set_u_id)) {
                    throw std::logic_error(
                        "setcoveringsolver::Reduction::reduce_unconfined_sets.");
                }
                const ReductionSet& set_u = tmp.instance.sets[set_u_id];
                ElementPos m = 0;
                ElementId element_id_cur = -1;
                for (ElementId element_id: set_u.elements) {
                    if (elements_to_remove.contains(element_id))
                        continue;
                    if (s_ns_mandatory_covered_elements[element_id] <= 1) {
                        m++;
                        element_id_cur = element_id;
                        if (m > 2)
                            break;
                    }
                }

                // If all elements of set_u_id are in
                // s_ns_mandatory_covered_elements.
                if (m == 0) {
                    fixed = true;

                    //std::cout << "set_id " << set_id << " fixed" << std::endl;
                    //std::cout << " s";
                    //for (SetId set_v_id: s)
                    //    std::cout << " " << set_v_id;
                    //std::cout << std::endl;

                    //for (SetId set_v_id: s) {
                    //    std::cout << "  " << set_v_id << ":";
                    //    for (SetId e: tmp.instance.sets[set_v_id].elements)
                    //        std::cout << " " << e << "," << tmp.instance.elements[e].sets.size();
                    //    std::cout << std::endl;
                    //}

                    //std::cout << " ns_mandatory";
                    //for (SetId set_u_id: ns_mandatory)
                    //    std::cout << " " << set_u_id;
                    //std::cout << std::endl;

                    //for (SetId set_u_id: ns_mandatory) {
                    //    std::cout << "  " << set_u_id << ":";
                    //    for (SetId e: tmp.instance.sets[set_u_id].elements)
                    //        std::cout << " " << e;
                    //    std::cout << std::endl;
                    //}

                    //std::cout << " ns_candidates";
                    //for (SetId set_u_id: ns_candidates)
                    //    std::cout << " " << set_u_id;
                    //std::cout << std::endl;
                    //std::cout << " s_elts";
                    //for (ElementId element_id: s_covered_elements)
                    //    std::cout << " " << element_id;
                    //std::cout << std::endl;
                    //std::cout << " s_ns_elts";
                    //for (const auto& p: s_ns_mandatory_covered_elements)
                    //    std::cout << " " << p.first << "," << p.second;
                    //std::cout << std::endl;

                    break;
                }

                // If there is a single element of set_u_id in
                // s_ns_mandatory_covered_elements, if this element is only
                // covered by a single other set, it can be added to S.
                if (m == 1) {
                    const ReductionElement& element_cur
                        = tmp.instance.elements[element_id_cur];
                    SetPos n = 0;
                    SetId other_set_id = -1;
                    for (SetId set_id_cur: element_cur.sets) {
                        if (s.contains(set_id_cur)) {
                            throw std::logic_error(
                                    "setcoveringsolver::Reduction::reduce_unconfined_sets.");
                        }
                        if (fixed_sets.contains(set_id_cur)) {
                            throw std::logic_error(
                                    "setcoveringsolver::Reduction::reduce_unconfined_sets.");
                        }
                        n++;
                        if (set_id_cur != set_u_id)
                            other_set_id = set_id_cur;
                    }
                    if (n == 2)
                        set_w_id = other_set_id;
                }
            }

            if (fixed)
                break;
            if (set_w_id == -1)
                break;
            set_v_id = set_w_id;

            if (ns_mandatory.contains(set_v_id)) {
                throw std::logic_error(
                        "setcoveringsolver::Reduction::reduce_unconfined_sets: "
                        "'ns_mandatory' should not contain 'set_v_id'.");
            }
        }

        if (fixed) {
            fixed_sets.add(set_id);
            for (ElementId element_id: set.elements)
                elements_to_remove.add(element_id);
        }
    }

    if (fixed_sets.size() == 0)
        return false;

    //std::cout << fixed_sets.size() << std::endl;

    // Update mandatory_sets.
    for (SetId set_id: fixed_sets)
        for (SetId orig_set_id: unreduction_operations_[set_id].in)
            mandatory_sets_.push_back(orig_set_id);
    // Update sets.
    for (SetId set_id = 0;
            set_id < tmp.instance.number_of_sets();
            ++set_id) {
        ReductionSet& set = tmp.instance.set(set_id);
        if (set.removed)
            continue;
        if (fixed_sets.contains(set_id)) {
            set.removed = true;
        } else {
            for (ElementPos pos = 0;
                    pos < (ElementPos)set.elements.size();
                    ) {
                ElementId element_id = set.elements[pos];
                if (elements_to_remove.contains(element_id)) {
                    set.elements[pos] = set.elements.back();
                    set.elements.pop_back();
                } else {
                    pos++;
                }
            }
        }
    }
    // Update elements.
    for (ElementId element_id = 0;
            element_id < tmp.instance.number_of_elements();
            ++element_id) {
        ReductionElement& element = tmp.instance.element(element_id);
        if (element.removed)
            continue;
        if (elements_to_remove.contains(element_id)) {
            element.removed = true;
        } else {
            for (ElementPos pos = 0;
                    pos < (ElementPos)element.sets.size();
                    ) {
                ElementId set_id = element.sets[pos];
                if (fixed_sets.contains(set_id)) {
                    element.sets[pos] = element.sets.back();
                    element.sets.pop_back();
                } else {
                    pos++;
                }
            }
        }
    }

    //check(tmp.instance);
    if (needs_update(tmp.instance))
        update(tmp.instance, unreduction_operations_);
    return true;
}

bool Reduction::reduce_dominated_sets_2(
        Tmp& tmp,
        const ReductionParameters& parameters)
{
    //std::cout << "reduce_dominated_sets_trivial..." << std::endl;

    optimizationtools::IndexedSet& covered_elements = tmp.indexed_set_;
    covered_elements.resize_and_clear(tmp.instance.number_of_elements());
    optimizationtools::IndexedSet& set_neighbors = tmp.indexed_set_3_;
    set_neighbors.resize_and_clear(tmp.instance.number_of_sets());
    optimizationtools::IndexedSet& sets_to_remove = tmp.indexed_set_2_;
    sets_to_remove.resize_and_clear(tmp.instance.number_of_sets());

    // For each element, get the list of 2-covering sets covering it.
    std::vector<std::vector<SetId>> sets(tmp.instance.number_of_elements());
    SetPos nb_sets = 0;
    for (ElementId element_id = 0;
            element_id < tmp.instance.number_of_elements();
            ++element_id) {
        const ReductionElement& element = tmp.instance.element(element_id);
        if (element.removed)
            continue;

        for (SetId set_id: element.sets) {
            const ReductionSet& set = tmp.instance.set(set_id);
            if (set.elements.size() == 2) {
                sets[element_id].push_back(set_id);
                nb_sets++;
            }
        }
    }
    if (nb_sets == 0)
        return false;

    std::vector<SetId>& shuffled_sets = tmp.set_;
    shuffled_sets.resize(tmp.instance.number_of_sets());
    std::iota(shuffled_sets.begin(), shuffled_sets.end(), 0);
    std::shuffle(shuffled_sets.begin(), shuffled_sets.end(), tmp.generator_);
    for (auto it = shuffled_sets.begin();
            it != shuffled_sets.begin() + (std::min)(tmp.instance.number_of_sets(), tmp.instance.number_of_elements());
            ++it) {
        // Check timer.
        if (parameters.timer.needs_to_end())
            break;

        SetId set_id = *it;
        if (sets_to_remove.contains(set_id))
            continue;
        const ReductionSet& set = tmp.instance.set(set_id);
        if (set.removed)
            continue;

        if (set.elements.size() != 2)
            continue;

        ElementId element_id_1 = set.elements[0];
        ElementId element_id_2 = set.elements[1];
        const ReductionElement& element_1 = tmp.instance.element(element_id_1);
        const ReductionElement& element_2 = tmp.instance.element(element_id_2);

        ElementId element_id = element_id_1;
        if (element_1.sets.size() - sets[element_id_1].size()
                > element_2.sets.size() - sets[element_id_2].size()) {
            element_id = element_id_2;
        }

        for (SetId set_id_2 : tmp.instance.element(element_id).sets) {
            if (set_id_2 == set_id)
                continue;
            if (sets_to_remove.contains(set_id_2))
                continue;
            const ReductionSet& set_2 = tmp.instance.set(set_id_2);
            if (set_2.cost > set.cost)
                continue;
            if (set_2.elements.size() <= 2)
                continue;

            bool found_1 = false;
            bool found_2 = false;
            for (ElementId element_id_3: set_2.elements) {
                if (element_id_3 == element_id_1) {
                    found_1 = true;
                    if (found_2)
                        break;
                }
                if (element_id_3 == element_id_2) {
                    found_2 = true;
                    if (found_1)
                        break;
                }
            }
            if (found_1 && found_2) {
                sets_to_remove.add(set_id);
                break;
            }
        }
    }

    if (sets_to_remove.size() == 0)
        return false;

    //std::cout << sets_to_remove.size() << std::endl;

    // Update mandatory_sets.
    for (SetId set_id: sets_to_remove)
        for (SetId orig_set_id: unreduction_operations_[set_id].out)
            mandatory_sets_.push_back(orig_set_id);
    // Update sets.
    for (SetId set_id = 0;
            set_id < tmp.instance.number_of_sets();
            ++set_id) {
        ReductionSet& set = tmp.instance.set(set_id);
        if (set.removed)
            continue;
        if (sets_to_remove.contains(set_id))
            set.removed = true;
    }
    // Update elements.
    for (ElementId element_id = 0;
            element_id < tmp.instance.number_of_elements();
            ++element_id) {
        ReductionElement& element = tmp.instance.element(element_id);
        if (element.removed)
            continue;
        for (ElementPos pos = 0;
                pos < (ElementPos)element.sets.size();
                ) {
            ElementId set_id = element.sets[pos];
            if (sets_to_remove.contains(set_id)) {
                element.sets[pos] = element.sets.back();
                element.sets.pop_back();
            } else {
                pos++;
            }
        }
    }

    //check(tmp.instance);
    if (needs_update(tmp.instance))
        update(tmp.instance, unreduction_operations_);
    return true;
}

bool Reduction::reduce_dominated_elements_2(
        Tmp& tmp,
        const ReductionParameters& parameters)
{
    //std::cout << "reduce_dominated_elements_trivial..." << std::endl;

    optimizationtools::IndexedSet& covering_sets = tmp.indexed_set_;
    covering_sets.resize_and_clear(tmp.instance.number_of_sets());
    optimizationtools::IndexedSet& element_neighbors = tmp.indexed_set_3_;
    element_neighbors.resize_and_clear(tmp.instance.number_of_elements());
    optimizationtools::IndexedSet& elements_to_remove = tmp.indexed_set_2_;
    elements_to_remove.resize_and_clear(tmp.instance.number_of_elements());

    // For each set, get the list of 2-covered elements it covers.
    std::vector<std::vector<ElementId>> elements(tmp.instance.number_of_sets());
    SetPos nb_elements = 0;
    for (SetId set_id = 0;
            set_id < tmp.instance.number_of_sets();
            ++set_id) {
        const ReductionSet& set = tmp.instance.set(set_id);
        if (set.removed)
            continue;
        for (ElementId element_id: set.elements) {
            const ReductionElement& element = tmp.instance.element(element_id);
            if (element.sets.size() == 2) {
                elements[set_id].push_back(element_id);
                nb_elements++;
            }
        }
    }
    if (nb_elements == 0)
        return false;

    std::vector<ElementId>& shuffled_elements = tmp.set_;
    shuffled_elements.resize(tmp.instance.number_of_elements());
    std::iota(shuffled_elements.begin(), shuffled_elements.end(), 0);
    std::shuffle(shuffled_elements.begin(), shuffled_elements.end(), tmp.generator_);
    for (auto it = shuffled_elements.begin();
            it != shuffled_elements.begin() + (std::min)(tmp.instance.number_of_sets(), tmp.instance.number_of_elements());
            ++it) {
        // Check timer.
        if (parameters.timer.needs_to_end())
            break;

        ElementId element_id = *it;
        const ReductionElement& element = tmp.instance.element(element_id);
        if (element.removed)
            continue;

        if (element.sets.size() != 2)
            continue;

        SetId set_id_1 = element.sets[0];
        SetId set_id_2 = element.sets[1];
        const ReductionSet& set_1 = tmp.instance.set(set_id_1);
        const ReductionSet& set_2 = tmp.instance.set(set_id_2);

        SetId set_id = set_id_1;
        if (set_1.elements.size() - elements[set_id_1].size()
                > set_2.elements.size() - elements[set_id_2].size()) {
            set_id = set_id_2;
        }

        for (ElementId element_id_2: tmp.instance.set(set_id).elements) {
            if (element_id_2 == element_id)
                continue;
            if (elements_to_remove.contains(element_id_2))
                continue;

            const ReductionElement& element_2 = tmp.instance.element(element_id_2);

            if (element_2.sets.size() <= 2)
                continue;

            bool found_1 = false;
            bool found_2 = false;
            for (SetId set_id_3: element_2.sets) {
                if (set_id_3 == set_id_1) {
                    found_1 = true;
                    if (found_2)
                        break;
                }
                if (set_id_3 == set_id_2) {
                    found_2 = true;
                    if (found_1)
                        break;
                }
            }
            if (found_1 && found_2)
                elements_to_remove.add(element_id_2);
        }
    }

    if (elements_to_remove.size() == 0)
        return false;

    //std::cout << elements_to_remove.size() << std::endl;

    // Update sets.
    for (SetId set_id = 0;
            set_id < tmp.instance.number_of_sets();
            ++set_id) {
        ReductionSet& set = tmp.instance.set(set_id);
        if (set.removed)
            continue;
        for (ElementPos pos = 0;
                pos < (ElementPos)set.elements.size();
                ) {
            ElementId element_id = set.elements[pos];
            if (elements_to_remove.contains(element_id)) {
                set.elements[pos] = set.elements.back();
                set.elements.pop_back();
            } else {
                pos++;
            }
        }
    }
    // Update elements.
    for (ElementId element_id = 0;
            element_id < tmp.instance.number_of_elements();
            ++element_id) {
        ReductionElement& element = tmp.instance.element(element_id);
        if (element.removed)
            continue;
        if (elements_to_remove.contains(element_id))
            element.removed = true;
    }

    //check(tmp.instance);
    if (needs_update(tmp.instance))
        update(tmp.instance, unreduction_operations_);
    return true;
}

bool Reduction::reduce_dominated_elements(
        Tmp& tmp,
        const ReductionParameters& parameters)
{
    //std::cout << "reduce_dominated_elements..." << std::endl;

    optimizationtools::IndexedSet& covered_sets = tmp.indexed_set_;
    covered_sets.resize_and_clear(tmp.instance.number_of_sets());
    optimizationtools::IndexedSet& elements_to_remove = tmp.indexed_set_2_;
    elements_to_remove.resize_and_clear(tmp.instance.number_of_elements());

    std::vector<ElementId>& shuffled_elements = tmp.set_;
    shuffled_elements.resize(tmp.instance.number_of_elements());
    std::iota(shuffled_elements.begin(), shuffled_elements.end(), 0);
    std::shuffle(shuffled_elements.begin(), shuffled_elements.end(), tmp.generator_);
    for (auto it = shuffled_elements.begin();
            it != shuffled_elements.begin() + (std::min)(tmp.instance.number_of_sets(), tmp.instance.number_of_elements());
            ++it) {
        // Check timer.
        if (parameters.timer.needs_to_end())
            break;

        if (elements_to_remove.size() > 0.01 * tmp.instance.number_of_elements())
            break;

        ElementId element_id_1 = *it;
        if (elements_to_remove.contains(element_id_1)) {
            continue;
        }

        const ReductionElement& element_1 = tmp.instance.element(element_id_1);
        if (element_1.removed)
            continue;
        if (element_1.sets.size() <= 2)
            continue;

        SetId set_id_1 = -1;
        ElementPos size_min = tmp.instance.number_of_elements() + 1;
        for (SetPos pos = 0;
                pos < (SetPos)element_1.sets.size();
                ++pos) {
            SetId set_id = element_1.sets[pos];
            ElementPos size = tmp.instance.set(set_id).elements.size();
            if (size < size_min) {
                size_min = size;
                set_id_1 = set_id;
                if (size_min == 2)
                    break;
                if (pos > 16 && size_min < 8)
                    break;
            }
        }
        const ReductionSet& set_1 = tmp.instance.set(set_id_1);

        for (ElementId element_id_2: tmp.instance.set(set_id_1).elements) {
            if (element_id_2 == element_id_1)
                continue;
            if (elements_to_remove.contains(element_id_2)) {
                continue;
            }
            const ReductionElement& element_2 = tmp.instance.element(element_id_2);
            if (element_2.sets.size() <= element_1.sets.size())
                continue;
            covered_sets.clear();
            for (SetId set_id: element_2.sets)
                covered_sets.add(set_id);
            // Check if element_id_1 dominates element_id_2
            bool dominates = true;
            for (SetId set_id: element_1.sets) {
                if (!covered_sets.contains(set_id)) {
                    dominates = false;
                    break;
                }
            }
            if (dominates) {
                elements_to_remove.add(element_id_2);
            }
        }
    }

    if (elements_to_remove.size() == 0)
        return false;

    //std::cout << elements_to_remove.size() << std::endl;

    // Update sets.
    for (SetId set_id = 0;
            set_id < tmp.instance.number_of_sets();
            ++set_id) {
        ReductionSet& set = tmp.instance.set(set_id);
        if (set.removed)
            continue;
        for (ElementPos pos = 0;
                pos < (ElementPos)set.elements.size();
                ) {
            ElementId element_id = set.elements[pos];
            if (elements_to_remove.contains(element_id)) {
                set.elements[pos] = set.elements.back();
                set.elements.pop_back();
            } else {
                pos++;
            }
        }
    }
    // Update elements.
    for (ElementId element_id = 0;
            element_id < tmp.instance.number_of_elements();
            ++element_id) {
        ReductionElement& element = tmp.instance.element(element_id);
        if (element.removed)
            continue;
        if (elements_to_remove.contains(element_id))
            element.removed = true;
    }

    //check(tmp.instance);
    if (needs_update(tmp.instance))
        update(tmp.instance, unreduction_operations_);
    return true;
}

bool Reduction::reduce_dominated_sets(
        Tmp& tmp,
        const ReductionParameters& parameters)
{
    //std::cout << "reduce_dominated_sets..." << std::endl;

    optimizationtools::IndexedSet& covered_elements = tmp.indexed_set_;
    covered_elements.resize_and_clear(tmp.instance.number_of_elements());
    optimizationtools::IndexedSet& sets_to_remove = tmp.indexed_set_2_;
    sets_to_remove.resize_and_clear(tmp.instance.number_of_sets());

    std::vector<SetId>& shuffled_sets = tmp.set_;
    shuffled_sets.resize(tmp.instance.number_of_sets());
    std::iota(shuffled_sets.begin(), shuffled_sets.end(), 0);
    std::shuffle(shuffled_sets.begin(), shuffled_sets.end(), tmp.generator_);
    for (auto it = shuffled_sets.begin();
            it != shuffled_sets.begin() + (std::min)(tmp.instance.number_of_sets(), tmp.instance.number_of_elements());
            ++it) {
        // Check timer.
        if (parameters.timer.needs_to_end())
            break;

        if (sets_to_remove.size() > 0.01 * tmp.instance.number_of_sets())
            break;

        SetId set_id_1 = *it;
        const ReductionSet& set_1 = tmp.instance.set(set_id_1);
        if (set_1.removed)
            continue;
        if (set_1.elements.size() <= 2)
            continue;

        ElementId element_id_1 = -1;
        SetPos size_min = tmp.instance.number_of_sets() + 1;
        for (ElementPos pos = 0;
                pos < (ElementPos)set_1.elements.size();
                ++pos) {
            ElementId element_id = set_1.elements[pos];
            SetPos size = tmp.instance.element(element_id).sets.size();
            if (size < size_min) {
                size_min = size;
                element_id_1 = element_id;
                if (size_min == 2)
                    break;
                if (pos > 16 && size_min < 8)
                    break;
            }
        }
        // set_1 will be dominated only by another set that contains at least element_id_1
        // no need to loop through others elements
        const ReductionElement& element_1 = tmp.instance.element(element_id_1);
        for (SetId set_id_2: element_1.sets) {
            if (set_id_2 == set_id_1)
                continue;
            if (sets_to_remove.contains(set_id_2))
                continue;
            const ReductionSet& set_2 = tmp.instance.set(set_id_2);
            if (set_2.elements.size() <= 2)
                continue;
            if (set_2.elements.size() < set_1.elements.size())
                continue;
            if (set_2.cost > set_1.cost)
                continue;
            if (set_2.elements.size() == set_1.elements.size()
                    && set_2.cost >= set_1.cost)
                continue;
            if (set_2.elements.size() <= set_1.elements.size()
                    && set_1.cost == set_2.cost)
                continue;
            covered_elements.clear();
            for (ElementId element_id: set_2.elements)
                covered_elements.add(element_id);
            // Check if set_id_2 dominates set_id_1
            bool dominates = true;
            for (ElementId element_id: set_1.elements) {
                if (!covered_elements.contains(element_id)) {
                    dominates = false;
                    break;
                }
            }
            if (dominates) {
                sets_to_remove.add(set_id_1);
                break;
            }
        }
    }

    if (sets_to_remove.size() == 0)
        return false;

    //std::cout << sets_to_remove.size() << std::endl;

    // Update mandatory_sets.
    for (SetId set_id: sets_to_remove)
        for (SetId orig_set_id: unreduction_operations_[set_id].out)
            mandatory_sets_.push_back(orig_set_id);
    // Update sets.
    for (SetId set_id = 0;
            set_id < tmp.instance.number_of_sets();
            ++set_id) {
        ReductionSet& set = tmp.instance.set(set_id);
        if (set.removed)
            continue;
        if (sets_to_remove.contains(set_id))
            set.removed = true;
    }
    // Update elements.
    for (ElementId element_id = 0;
            element_id < tmp.instance.number_of_elements();
            ++element_id) {
        ReductionElement& element = tmp.instance.element(element_id);
        if (element.removed)
            continue;
        for (ElementPos pos = 0;
                pos < (ElementPos)element.sets.size();
                ) {
            ElementId set_id = element.sets[pos];
            if (sets_to_remove.contains(set_id)) {
                element.sets[pos] = element.sets.back();
                element.sets.pop_back();
            } else {
                pos++;
            }
        }
    }

    //check(tmp.instance);
    if (needs_update(tmp.instance))
        update(tmp.instance, unreduction_operations_);
    return true;
}

void Reduction::reduce_small_components(Tmp& tmp)
{
    if (instance().number_of_components() == 1)
        return;
    //std::cout << "reduce_small_components..." << std::endl;

    optimizationtools::IndexedSet& covered_elements = tmp.indexed_set_;
    covered_elements.resize_and_clear(instance().number_of_elements());
    optimizationtools::IndexedSet& sets_to_remove = tmp.indexed_set_2_;
    sets_to_remove.resize_and_clear(instance().number_of_sets());
    optimizationtools::IndexedSet& fixed_sets = tmp.indexed_set_5_;
    fixed_sets.resize_and_clear(instance().number_of_sets());
    optimizationtools::IndexedSet& elements_to_remove = tmp.indexed_set_4_;
    elements_to_remove.resize_and_clear(instance().number_of_elements());

    std::vector<SetId> sets_original2component(instance().number_of_sets(), -1);
    std::vector<ElementId> elements_original2component(instance().number_of_elements(), -1);
    for (ComponentId component_id = 0;
            component_id < instance().number_of_components();
            ++component_id) {
        const Component& component = instance().component(component_id);
        bool solved = false;

        // Build component instance.
        InstanceBuilder component_instance_builder;
        component_instance_builder.add_sets(component.sets.size());
        component_instance_builder.add_elements(component.elements.size());
        ElementId new_element_id = 0;
        for (ElementId element_id: component.elements) {
            elements_original2component[element_id] = new_element_id;
            new_element_id++;
        }
        SetId new_set_id = 0;
        for (SetId set_id: component.sets) {
            sets_original2component[set_id] = new_set_id;
            component_instance_builder.set_cost(new_set_id, instance().set(set_id).cost);
            for (ElementId element_id: instance().set(set_id).elements) {
                ElementId new_element_id = elements_original2component[element_id];
                component_instance_builder.add_arc(
                        new_set_id,
                        new_element_id);
            }
            new_set_id++;
        }
        Instance component_instance = component_instance_builder.build();

        // Compute bound.
        Cost bound = 0;
        Parameters component_trivial_bound_parameters;
        component_trivial_bound_parameters.verbosity_level = 0;
        component_trivial_bound_parameters.reduction_parameters.reduce = false;
        auto component_trivial_bound_output = trivial_bound(component_instance, component_trivial_bound_parameters);
        bound = (std::max)(bound, component_trivial_bound_output.bound);

        //ElementPos number_of_2_elements = 0;
        //for (ElementId element_id = 0;
        //        element_id < component_instance.number_of_elements();
        //        ++element_id) {
        //    const Element& element = component_instance.element(element_id);
        //    if (element.sets.size() == 2)
        //        number_of_2_elements++;
        //}
        //ElementPos number_of_edges_complementary
        //    = component_instance.number_of_sets() * (component_instance.number_of_sets() - 1) / 2
        //    - number_of_2_elements;
        //if (number_of_edges_complementary <= 1e7) {
        //    Parameters clique_cover_bound_parameters;
        //    clique_cover_bound_parameters.verbosity_level = 0;
        //    clique_cover_bound_parameters.reduction_parameters.reduce = false;
        //    auto clique_cover_output = clique_cover_bound(
        //            component_instance,
        //            clique_cover_bound_parameters);
        //    bound = (std::max)(bound, clique_cover_output.bound);
        //}

        if (bound == 2) {

            // Sort sets by number of covered elements.
            std::vector<SetId> sorted_set_ids = component.sets;
            std::sort(sorted_set_ids.begin(), sorted_set_ids.end(),
                    [this](SetId set_id_1, SetId set_id_2) -> bool
                    {
                        return instance().set(set_id_1).elements.size() > instance().set(set_id_2).elements.size();
                    });

            // Search for a solution containing 2 sets.
            for (SetPos pos_1 = 0;
                    pos_1 < (SetPos)sorted_set_ids.size();
                    ++pos_1) {
                SetId set_id_1 = sorted_set_ids[pos_1];
                const Set& set_1 = instance().set(set_id_1);

                if (set_1.elements.size() * 2 < component.elements.size())
                    break;

                covered_elements.clear();
                for (ElementId element_id: set_1.elements)
                    covered_elements.add(element_id);

                for (SetPos pos_2 = pos_1 + 1;
                        pos_2 < (SetPos)sorted_set_ids.size();
                        ++pos_2) {
                    SetId set_id_2 = sorted_set_ids[pos_2];
                    const Set& set_2 = instance().set(set_id_2);

                    if (set_1.elements.size() + set_2.elements.size() < component.elements.size())
                        break;

                    ElementPos number_of_covered_elements = covered_elements.size();
                    for (ElementId element_id: set_2.elements)
                        if (!covered_elements.contains(element_id))
                            number_of_covered_elements++;

                    if (number_of_covered_elements == component.elements.size()) {
                        fixed_sets.add(set_id_1);
                        fixed_sets.add(set_id_2);
                        for (SetId set_id: component.sets)
                            sets_to_remove.add(set_id);
                        for (ElementId element_id: component.elements)
                            elements_to_remove.add(element_id);
                        solved = true;
                        //std::cout << "solve component " << component_id << std::endl;
                        break;
                    }
                }

                if (solved)
                    break;
            }
        } else {

            Parameters component_greedy_parameters;
            component_greedy_parameters.verbosity_level = 0;
            component_greedy_parameters.reduction_parameters.reduce = false;
            auto component_greedy_output = greedy(component_instance, component_greedy_parameters);
            if (component_greedy_output.solution.cost() == bound) {
                for (SetId set_id: component.sets) {
                    SetId new_set_id = sets_original2component[set_id];
                    if (component_greedy_output.solution.contains(new_set_id))
                        fixed_sets.add(set_id);
                    sets_to_remove.add(set_id);
                }
                for (ElementId element_id: component.elements)
                    elements_to_remove.add(element_id);
                //std::cout << "solve component " << component_id << " cost " << bound << std::endl;
            }

        }
    }

    if (sets_to_remove.size() == 0)
        return;

    //std::cout << sets_to_remove.size() << " " << fixed_sets.size() << " " << elements_to_remove.size() << std::endl;

    // Update mandatory_sets.
    for (SetId set_id: sets_to_remove) {
        if (fixed_sets.contains(set_id)) {
            for (SetId orig_set_id: unreduction_operations_[set_id].in)
                mandatory_sets_.push_back(orig_set_id);
        } else {
            for (SetId orig_set_id: unreduction_operations_[set_id].out)
                mandatory_sets_.push_back(orig_set_id);
        }
    }
    // Create new instance and compute unreduction_operations.
    SetId new_number_of_sets = instance().number_of_sets() - sets_to_remove.size();
    ElementId new_number_of_elements = instance().number_of_elements() - elements_to_remove.size();
    InstanceBuilder new_instance_builder;
    new_instance_builder.add_elements(new_number_of_elements);
    new_instance_builder.add_sets(new_number_of_sets);
    //new_instance_builder.move(
    //        std::move(tmp.instance_),
    //        new_number_of_sets,
    //        new_number_of_elements);
    for (SetId set_id = 0; set_id < new_number_of_sets; ++set_id) {
        tmp.unreduction_operations[set_id].in.clear();
        tmp.unreduction_operations[set_id].out.clear();
    }
    // Add sets.
    std::vector<SetId> sets_original2reduced(instance().number_of_sets(), -1);
    std::vector<ElementId> elements_original2reduced(instance().number_of_elements(), -1);
    ElementId new_element_id = 0;
    for (auto it = elements_to_remove.out_begin();
            it != elements_to_remove.out_end();
            ++it) {
        ElementId element_id = *it;
        elements_original2reduced[element_id] = new_element_id;
        new_element_id++;
    }
    SetId new_set_id = 0;
    for (auto it = sets_to_remove.out_begin(); it != sets_to_remove.out_end(); ++it) {
        SetId set_id = *it;
        sets_original2reduced[set_id] = new_set_id;
        new_instance_builder.set_cost(new_set_id, instance().set(set_id).cost);
        tmp.unreduction_operations[new_set_id]
            = unreduction_operations_[set_id];
        new_set_id++;
    }
    // Add arcs.
    for (auto it = sets_to_remove.out_begin(); it != sets_to_remove.out_end(); ++it) {
        SetId set_id = *it;
        SetId new_set_id = sets_original2reduced[set_id];
        if (new_set_id == -1)
            continue;
        for (ElementId element_id: instance().set(set_id).elements) {
            ElementId new_element_id = elements_original2reduced[element_id];
            if (new_element_id == -1)
                continue;
            new_instance_builder.add_arc(
                    new_set_id,
                    new_element_id);
        }
    }

    unreduction_operations_.swap(tmp.unreduction_operations);
    //tmp.instance_ = std::move(instance_);
    instance_ = new_instance_builder.build();
}

Reduction::Reduction(
        const Instance& instance,
        const ReductionParameters& parameters):
    original_instance_(&instance),
    instance_(instance)
{
    Tmp tmp(instance);

    std::uniform_int_distribution<uint64_t> distribution(
        std::numeric_limits<uint64_t>::min(),
        std::numeric_limits<uint64_t>::max()
    );
    for (SetId pos = 0; pos < (SetId)tmp.random_.size(); ++pos)
        tmp.random_[pos] = distribution(tmp.generator_);
    tmp.instance = instance_to_reduction(instance);

    // Initialize reduced instance.
    unreduction_operations_ = std::vector<UnreductionOperations>(instance.number_of_sets());
    for (SetId set_id = 0;
            set_id < instance.number_of_sets();
            ++set_id) {
        unreduction_operations_[set_id].in.push_back(set_id);
    }

    for (Counter round_number = 0;
            round_number < parameters.maximum_number_of_rounds;
            ++round_number) {
        // Check timer.
        if (parameters.timer.needs_to_end())
            break;
        //std::cout << "round_number " << round_number
        //    << " number_of_elements " << this->instance().number_of_elements()
        //    << " number_of_sets " << this->instance().number_of_sets()
        //    << " number_of_arcs " << this->instance().number_of_arcs()
        //    << std::endl;
        bool found = false;
        found |= reduce_mandatory_sets(tmp);
        found |= reduce_dominated_sets_2(tmp, parameters);
        if (parameters.timer.needs_to_end())
            break;
        found |= reduce_dominated_elements_2(tmp, parameters);
        if (parameters.timer.needs_to_end())
            break;
        if (parameters.set_folding) {
            for (;;) {
                bool found_cur = reduce_set_folding(tmp);
                if (!found_cur)
                    break;
                found |= found_cur;
            }
        }
        // Twin reduction fails if some elements are covered by only one vertex.
        // So, run the mandatory set reduction right before.
        if (parameters.twin) {
            found |= reduce_mandatory_sets(tmp);
            found |= reduce_twin(tmp);
        }
        found |= reduce_identical_sets(tmp);
        found |= reduce_identical_elements(tmp);
        if (!found || round_number >= 4) {
            if (parameters.unconfined_sets)
                found |= reduce_unconfined_sets(tmp);
            if (parameters.dominated_sets_removal) {
                found |= reduce_dominated_sets(tmp, parameters);
                if (parameters.timer.needs_to_end())
                    break;
            }
            if (parameters.dominated_elements_removal) {
                found |= reduce_dominated_elements(tmp, parameters);
                if (parameters.timer.needs_to_end())
                    break;
            }
        }
        if (found)
            continue;
        break;
    }

    update(tmp.instance, unreduction_operations_);
    instance_ = reduction_to_instance(tmp.instance);
    if (!parameters.timer.needs_to_end())
        reduce_small_components(tmp);

    extra_cost_ = 0;
    for (SetId orig_set_id: mandatory_sets_)
        extra_cost_ += instance.set(orig_set_id).cost;
}

void Reduction::unreduce_solution(
        Solution& new_solution,
        const Solution& solution) const
{
    for (SetId set_id: mandatory_sets_)
        if (!new_solution.contains(set_id))
            new_solution.add(set_id);

    for (SetId set_id = 0;
            set_id < instance().number_of_sets();
            ++set_id) {
        if (solution.contains(set_id)) {
            for (SetId set_id_2: unreduction_operations_[set_id].in)
                if (!new_solution.contains(set_id_2))
                    new_solution.add(set_id_2);
            for (SetId set_id_2: unreduction_operations_[set_id].out)
                if (new_solution.contains(set_id_2))
                    new_solution.remove(set_id_2);
        } else {
            for (SetId set_id_2: unreduction_operations_[set_id].in)
                if (new_solution.contains(set_id_2))
                    new_solution.remove(set_id_2);
            for (SetId set_id_2: unreduction_operations_[set_id].out)
                if (!new_solution.contains(set_id_2))
                    new_solution.add(set_id_2);
        }
    }
}

Solution Reduction::unreduce_solution(
        const Solution& solution) const
{
    Solution new_solution(*original_instance_);

    for (SetId set_id: mandatory_sets_)
        new_solution.add(set_id);

    for (SetId set_id = 0;
            set_id < instance().number_of_sets();
            ++set_id) {
        if (solution.contains(set_id)) {
            for (SetId set_id_2: unreduction_operations_[set_id].in)
                new_solution.add(set_id_2);
        } else {
            for (SetId set_id_2: unreduction_operations_[set_id].out)
                new_solution.add(set_id_2);
        }
    }

    return new_solution;
}

Cost Reduction::unreduce_bound(
        Cost bound) const
{
    return extra_cost_ + bound;
}
