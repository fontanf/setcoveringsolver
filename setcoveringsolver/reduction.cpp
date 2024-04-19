#include "setcoveringsolver/reduction.hpp"

#include "setcoveringsolver/instance_builder.hpp"

#include "optimizationtools/containers/indexed_set.hpp"

using namespace setcoveringsolver;

bool Reduction::reduce_mandatory_sets()
{
    //std::cout << "reduce_mandatory_sets..." << std::endl;
    optimizationtools::IndexedSet fixed_sets(instance().number_of_sets());
    optimizationtools::IndexedSet elements_to_remove(instance().number_of_elements());
    for (ElementId element_id = 0;
            element_id < instance().number_of_elements();
            ++element_id) {
        const Element& element = instance().element(element_id);
        if (element.sets.size() == 1) {
            SetId set_id = element.sets.front();
            fixed_sets.add(set_id);
            for (ElementId element_id_2: instance().set(set_id).elements) {
                elements_to_remove.add(element_id_2);
            }
        }
    }
    //std::cout << fixed_sets.size() << std::endl;

    if (fixed_sets.size() == 0)
        return false;

    std::vector<SetId> new_unreduction_operations;
    std::vector<SetId> new_mandatory_sets;

    // Update mandatory_sets.
    new_mandatory_sets = mandatory_sets_;
    for (SetId set_id: fixed_sets) {
        SetId orig_item_id = unreduction_operations_[set_id];
        new_mandatory_sets.push_back(orig_item_id);
    }
    // Create new instance and compute unreduction_operations.
    SetId new_number_of_sets = instance().number_of_sets() - fixed_sets.size();
    SetId new_number_of_elements = instance().number_of_elements() - elements_to_remove.size();
    InstanceBuilder new_instance_builder;
    new_instance_builder.add_sets(new_number_of_sets);
    new_instance_builder.add_elements(new_number_of_elements);
    new_unreduction_operations = std::vector<SetId>(new_number_of_sets);
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
    for (auto it = fixed_sets.out_begin(); it != fixed_sets.out_end(); ++it) {
        SetId set_id = *it;
        sets_original2reduced[set_id] = new_set_id;
        new_instance_builder.set_cost(new_set_id, instance().set(set_id).cost);
        new_unreduction_operations[new_set_id]
            = unreduction_operations_[set_id];
        new_set_id++;
    }
    // Add arcs.
    for (auto it = fixed_sets.out_begin(); it != fixed_sets.out_end(); ++it) {
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

    unreduction_operations_ = new_unreduction_operations;
    mandatory_sets_ = new_mandatory_sets;
    instance_ = new_instance_builder.build();
    return true;
}

bool Reduction::reduce_identical_elements()
{
    //std::cout << "reduce_identical_elements..." << std::endl;
    optimizationtools::IndexedSet elements_to_remove(instance().number_of_elements());
    std::vector<ElementId> elements_sorted(instance().number_of_elements());
    std::iota(elements_sorted.begin(), elements_sorted.end(), 0);
    sort(elements_sorted.begin(), elements_sorted.end(),
            [this](ElementId element_id_1, ElementId element_id_2) -> bool
    {
        if (instance().element(element_id_1).sets.size() != instance().element(element_id_2).sets.size())
            return instance().element(element_id_1).sets.size() < instance().element(element_id_2).sets.size();
        for (SetPos set_pos = 0;
                set_pos < (SetPos)instance().element(element_id_1).sets.size();
                ++set_pos) {
            if (instance().element(element_id_1).sets[set_pos]
                    != instance().element(element_id_2).sets[set_pos]) {
                return (instance().element(element_id_1).sets[set_pos] < instance().element(element_id_2).sets[set_pos]);
            }
        }
        return element_id_2 < element_id_1;
    });
    ElementId element_id_pred = -1;
    for (ElementId element_id: elements_sorted) {
        if (element_id_pred != -1) {
            if (instance().element(element_id_pred).sets == instance().element(element_id).sets)
                elements_to_remove.add(element_id_pred);
        }
        element_id_pred = element_id;
    }
    //std::cout << elements_to_remove.size() << std::endl;

    if (elements_to_remove.size() == 0)
        return false;

    std::vector<SetId> new_unreduction_operations;
    std::vector<SetId> new_mandatory_sets;

    // Update mandatory_sets.
    new_mandatory_sets = mandatory_sets_;
    // Create new instance and compute unreduction_operations.
    ElementId new_number_of_elements = instance().number_of_elements() - elements_to_remove.size();
    InstanceBuilder new_instance_builder;
    new_instance_builder.add_sets(instance().number_of_sets());
    new_instance_builder.add_elements(new_number_of_elements);
    new_unreduction_operations = std::vector<SetId>(instance().number_of_sets());
    std::vector<ElementId> original2reduced(instance().number_of_elements(), -1);
    ElementId new_element_id = 0;
    for (auto it = elements_to_remove.out_begin();
            it != elements_to_remove.out_end();
            ++it) {
        ElementId element_id = *it;
        original2reduced[element_id] = new_element_id;
        new_element_id++;
    }
    // Add sets and arcs.
    for (SetId set_id = 0; set_id < instance().number_of_sets(); ++set_id) {
        new_instance_builder.set_cost(set_id, instance().set(set_id).cost);
        new_unreduction_operations[set_id]
            = unreduction_operations_[set_id];
        for (ElementId element_id: instance().set(set_id).elements) {
            ElementId new_element_id = original2reduced[element_id];
            if (new_element_id == -1)
                continue;
            new_instance_builder.add_arc(
                    set_id,
                    new_element_id);
        }
    }

    unreduction_operations_ = new_unreduction_operations;
    mandatory_sets_ = new_mandatory_sets;
    instance_ = new_instance_builder.build();
    return true;
}

bool Reduction::reduce_identical_sets()
{
    //std::cout << "reduce_identical_sets..." << std::endl;
    optimizationtools::IndexedSet sets_to_remove(instance().number_of_sets());
    std::vector<ElementId> sets_sorted(instance().number_of_sets());
    std::iota(sets_sorted.begin(), sets_sorted.end(), 0);
    sort(sets_sorted.begin(), sets_sorted.end(),
            [this](SetId set_id_1, SetId set_id_2) -> bool
    {
        if (instance().set(set_id_1).elements.size() != instance().set(set_id_2).elements.size())
            return instance().set(set_id_1).elements.size() < instance().set(set_id_2).elements.size();
        for (ElementPos element_pos = 0; element_pos < (ElementPos)instance().set(set_id_1).elements.size(); ++element_pos)
            if (instance().set(set_id_1).elements[element_pos] != instance().set(set_id_2).elements[element_pos])
                return (instance().set(set_id_1).elements[element_pos] < instance().set(set_id_2).elements[element_pos]);
        return instance().set(set_id_1).cost > instance().set(set_id_2).cost;
    });
    SetId set_id_pred = -1;
    for (SetId set_id: sets_sorted) {
        if (instance().set(set_id).elements.size() == 0) {
            sets_to_remove.add(set_id);
            continue;
        }
        if (set_id_pred != -1) {
            if (instance().set(set_id_pred).cost >= instance().set(set_id).cost
                    && instance().set(set_id_pred).elements == instance().set(set_id).elements)
                sets_to_remove.add(set_id_pred);
        }
        set_id_pred = set_id;
    }
    //std::cout << sets_to_remove.size() << std::endl;

    if (sets_to_remove.size() == 0)
        return false;

    std::vector<SetId> new_unreduction_operations;
    std::vector<SetId> new_mandatory_sets;

    // Update mandatory_sets.
    new_mandatory_sets = mandatory_sets_;
    // Create new instance and compute unreduction_operations.
    SetId new_number_of_sets = instance().number_of_sets() - sets_to_remove.size();
    InstanceBuilder new_instance_builder;
    new_instance_builder.add_sets(new_number_of_sets);
    new_instance_builder.add_elements(instance().number_of_elements());
    new_unreduction_operations = std::vector<SetId>(new_number_of_sets);
    // Add sets.
    std::vector<SetId> original2reduced(instance().number_of_sets(), -1);
    SetId new_set_id = 0;
    for (auto it = sets_to_remove.out_begin();
            it != sets_to_remove.out_end(); ++it) {
        SetId set_id = *it;
        original2reduced[set_id] = new_set_id;
        new_instance_builder.set_cost(new_set_id, instance().set(set_id).cost);
        new_unreduction_operations[new_set_id]
            = unreduction_operations_[set_id];
        new_set_id++;
    }
    // Add arcs.
    for (auto it = sets_to_remove.out_begin();
            it != sets_to_remove.out_end();
            ++it) {
        SetId set_id = *it;
        SetId new_set_id = original2reduced[set_id];
        if (new_set_id == -1)
            continue;
        for (ElementId element_id: instance().set(set_id).elements) {
            new_instance_builder.add_arc(
                    new_set_id,
                    element_id);
        }
    }

    unreduction_operations_ = new_unreduction_operations;
    mandatory_sets_ = new_mandatory_sets;
    instance_ = new_instance_builder.build();
    return true;
}

bool Reduction::reduce_domianted_elements()
{
    //std::cout << "reduce_domianted_elements..." << std::endl;
    optimizationtools::IndexedSet elements_to_remove(instance().number_of_elements());
    optimizationtools::IndexedSet covered_sets(instance().number_of_sets());
    for (ElementId element_id_1 = 0;
            element_id_1 < instance().number_of_elements();
            ++element_id_1) {
        covered_sets.clear();
        for (SetId set_id: instance().element(element_id_1).sets)
            covered_sets.add(set_id);
        for (ElementId element_id_2 = 0;
                element_id_2 < instance().number_of_elements();
                ++element_id_2) {
            if (element_id_2 == element_id_1
                    || instance().element(element_id_2).sets.size()
                    > instance().element(element_id_1).sets.size())
                continue;
            // Check if element_id_2 dominates element_id_1
            bool dominates = true;
            for (SetId set_id: instance().element(element_id_2).sets) {
                if (!covered_sets.contains(set_id)) {
                    dominates = false;
                    break;
                }
            }
            if (dominates) {
                elements_to_remove.add(element_id_1);
                break;
            }
        }
    }
    //std::cout << elements_to_remove.size() << std::endl;

    if (elements_to_remove.size() == 0)
        return false;

    std::vector<SetId> new_unreduction_operations;
    std::vector<SetId> new_mandatory_sets;

    // Update mandatory_sets.
    new_mandatory_sets = mandatory_sets_;
    // Create new instance and compute unreduction_operations.
    ElementId new_number_of_elements = instance().number_of_elements() - elements_to_remove.size();
    InstanceBuilder new_instance_builder;
    new_instance_builder.add_sets(instance().number_of_sets());
    new_instance_builder.add_elements(new_number_of_elements);
    new_unreduction_operations = std::vector<SetId>(instance().number_of_sets());
    std::vector<ElementId> original2reduced(instance().number_of_elements(), -1);
    ElementId new_element_id = 0;
    for (auto it = elements_to_remove.out_begin();
            it != elements_to_remove.out_end();
            ++it) {
        ElementId element_id = *it;
        original2reduced[element_id] = new_element_id;
        new_element_id++;
    }
    // Add sets and arcs.
    for (SetId set_id = 0; set_id < instance().number_of_sets(); ++set_id) {
        new_instance_builder.set_cost(set_id, instance().set(set_id).cost);
        new_unreduction_operations[set_id]
            = unreduction_operations_[set_id];
        for (ElementId element_id: instance().set(set_id).elements) {
            ElementId new_element_id = original2reduced[element_id];
            if (new_element_id == -1)
                continue;
            new_instance_builder.add_arc(
                    set_id,
                    new_element_id);
        }
    }

    unreduction_operations_ = new_unreduction_operations;
    mandatory_sets_ = new_mandatory_sets;
    instance_ = new_instance_builder.build();
    return true;
}

bool Reduction::reduce_domianted_sets()
{
    //std::cout << "reduce_domianted_sets..." << std::endl;
    optimizationtools::IndexedSet sets_to_remove(instance().number_of_sets());
    optimizationtools::IndexedSet covered_elements(instance().number_of_elements());
    for (SetId set_id_1 = 0;
            set_id_1 < instance().number_of_sets();
            ++set_id_1) {
        covered_elements.clear();
        for (ElementId element_id: instance().set(set_id_1).elements)
            covered_elements.add(element_id);
        for (SetId set_id_2 = 0;
                set_id_2 < instance().number_of_sets();
                ++set_id_2) {
            if (set_id_2 == set_id_1
                    || instance().set(set_id_1).elements.size()
                    < instance().set(set_id_2).elements.size()
                    || instance().set(set_id_1).cost
                    > instance().set(set_id_2).cost)
                continue;
            // Check if set_id_1 dominates set_id_2
            bool dominates = true;
            for (ElementId element_id: instance().set(set_id_2).elements) {
                if (!covered_elements.contains(element_id)) {
                    dominates = false;
                    break;
                }
            }
            if (dominates)
                sets_to_remove.add(set_id_2);
        }
    }
    //std::cout << sets_to_remove.size() << std::endl;

    if (sets_to_remove.size() == 0)
        return false;

    std::vector<SetId> new_unreduction_operations;
    std::vector<SetId> new_mandatory_sets;

    // Update mandatory_sets.
    new_mandatory_sets = mandatory_sets_;
    // Create new instance and compute unreduction_operations.
    SetId new_number_of_sets = instance().number_of_sets() - sets_to_remove.size();
    InstanceBuilder new_instance_builder;
    new_instance_builder.add_sets(new_number_of_sets);
    new_instance_builder.add_elements(instance().number_of_elements());
    new_unreduction_operations = std::vector<SetId>(new_number_of_sets);
    // Add sets.
    std::vector<SetId> original2reduced(instance().number_of_sets(), -1);
    SetId new_set_id = 0;
    for (auto it = sets_to_remove.out_begin();
            it != sets_to_remove.out_end(); ++it) {
        SetId set_id = *it;
        original2reduced[set_id] = new_set_id;
        new_instance_builder.set_cost(new_set_id, instance().set(set_id).cost);
        new_unreduction_operations[new_set_id]
            = unreduction_operations_[set_id];
        new_set_id++;
    }
    // Add arcs.
    for (auto it = sets_to_remove.out_begin();
            it != sets_to_remove.out_end();
            ++it) {
        SetId set_id = *it;
        SetId new_set_id = original2reduced[set_id];
        if (new_set_id == -1)
            continue;
        for (ElementId element_id: instance().set(set_id).elements) {
            new_instance_builder.add_arc(
                    new_set_id,
                    element_id);
        }
    }

    unreduction_operations_ = new_unreduction_operations;
    mandatory_sets_ = new_mandatory_sets;
    instance_ = new_instance_builder.build();
    return true;
}

Reduction::Reduction(
        const Instance& instance,
        const ReductionParameters& parameters):
    original_instance_(&instance),
    instance_(instance)
{
    // Initialize reduced instance.
    unreduction_operations_ = std::vector<SetId>(instance.number_of_sets());
    for (SetId set_id = 0;
            set_id < instance.number_of_sets();
            ++set_id) {
        unreduction_operations_[set_id] = set_id;
    }

    for (Counter round_number = 0;
            round_number < parameters.maximum_number_of_rounds;
            ++round_number) {
        bool found = false;
        found |= reduce_mandatory_sets();
        found |= reduce_identical_elements();
        found |= reduce_identical_sets();
        if (found)
            continue;
        if (parameters.remove_domianted) {
            found |= reduce_domianted_elements();
            found |= reduce_domianted_sets();
        }
        if (!found)
            break;
    }

    extra_cost_ = 0;
    for (SetId orig_set_id: mandatory_sets_)
        extra_cost_ += instance.set(orig_set_id).cost;
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
            SetId set_id_2 = unreduction_operations_[set_id];
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
