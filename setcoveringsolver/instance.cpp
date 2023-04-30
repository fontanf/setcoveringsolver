#include "setcoveringsolver/instance.hpp"

#include <sstream>
#include <iomanip>
#include <thread>

using namespace setcoveringsolver;

Instance::Instance(
        std::string instance_path,
        std::string format)
{
    std::ifstream file(instance_path);
    if (!file.good()) {
        throw std::runtime_error(
                "Unable to open file \"" + instance_path + "\".");
    }

    if (format == "gecco2020" || format == "gecco") {
        read_geccod2020(file);
    } else if (format == "fulkerson1974" || format == "sts") {
        read_fulkerson1974(file);
    } else if (format == "balaset_id_1980" || format == "orlibrary") {
        read_balas1980(file);
    } else if (format == "balaset_id_1996") {
        read_balas1996(file);
    } else if (format == "faster1994" || format == "faster" || format == "wedelin1995" || format == "wedelin") {
        read_faster1994(file);
    } else {
        throw std::invalid_argument(
                "Unknown instance format \"" + format + "\".");
    }

    compute_components();
}

Instance::Instance(
        SetId number_of_sets,
        ElementId number_of_elements):
    elements_(number_of_elements),
    sets_(number_of_sets),
    total_cost_(number_of_sets)
{
}

void Instance::set_cost(
        SetId set_id,
        Cost cost)
{
    total_cost_ -= sets_[set_id].cost;
    sets_[set_id].cost = cost;
    total_cost_ += sets_[set_id].cost;
}

void Instance::add_arc(
        SetId set_id,
        ElementId element_id)
{
    elements_[element_id].sets.push_back(set_id);
    sets_[set_id].elements.push_back(element_id);
    number_of_arcs_++;
}

void Instance::compute_set_neighbors(
        Counter number_of_threads,
        optimizationtools::Info& info)
{
    info.os() << "Compute set neighbors..." << std::endl << std::endl;
    std::vector<std::thread> threads;
    for (Counter thread_id = 0; thread_id < number_of_threads; ++thread_id)
        threads.push_back(std::thread(&Instance::compute_set_neighbors_worker,
                    this,
                    thread_id       * number_of_sets() / number_of_threads,
                    (thread_id + 1) * number_of_sets() / number_of_threads));
    for (Counter thread_id = 0; thread_id < number_of_threads; ++thread_id)
        threads[thread_id].join();
}

void Instance::compute_element_neighbor_sets(
        optimizationtools::Info& info)
{
    info.os() << "Compute element neighbor sets..." << std::endl << std::endl;
    optimizationtools::IndexedSet neighbors(number_of_sets());
    for (ElementId element_id = 0;
            element_id < number_of_elements();
            ++element_id) {
        neighbors.clear();
        for (SetId set_id: element(element_id).sets) {
            neighbors.add(set_id);
            for (SetId set_id_2: set(set_id).neighbors)
                neighbors.add(set_id_2);
        }
        for (SetId set_id: neighbors)
            elements_[element_id].neighbor_sets.push_back(set_id);
    }
}

void Instance::compute_set_neighbors_worker(
        SetId s_start,
        SetId s_end)
{
    optimizationtools::IndexedSet neighbors(number_of_sets());
    for (SetId set_id_1 = s_start; set_id_1 < s_end; ++set_id_1) {
        neighbors.clear();
        for (ElementId element_id: set(set_id_1).elements)
            for (SetId set_id_2: element(element_id).sets)
                neighbors.add(set_id_2);
        if (neighbors.contains(set_id_1))
            neighbors.remove(set_id_1);
        for (SetId set_id_2: neighbors)
            sets_[set_id_1].neighbors.push_back(set_id_2);
    }
}

void Instance::compute_element_neighbors(optimizationtools::Info& info)
{
    info.os() << "Compute element neighbors..." << std::endl;
    optimizationtools::IndexedSet neighbors(number_of_elements());
    for (ElementId element_id_1 = 0; element_id_1 < number_of_elements(); ++element_id_1) {
        neighbors.clear();
        for (SetId set_id: element(element_id_1).sets)
            for (ElementId element_id_2: set(set_id).elements)
                if (element_id_2 != element_id_1)
                    neighbors.add(element_id_2);
        for (ElementId element_id_2: neighbors)
            elements_[element_id_1].neighbors.push_back(element_id_2);
    }
}

void Instance::compute_components()
{
    components_.clear();
    for (ElementId element_id = 0;
            element_id < number_of_elements();
            ++element_id)
        elements_[element_id].component = -1;
    for (SetId set_id = 0; set_id < number_of_sets(); ++set_id)
        sets_[set_id].component = -1;

    for (ComponentId component_id = 0;; ++component_id) {
        ElementId element_id = 0;
        while (element_id < number_of_elements()
                && (element(element_id).component != -1))
            element_id++;
        if (element_id == number_of_elements())
            break;
        components_.push_back(Component());
        std::vector<ElementId> stack {element_id};
        elements_[element_id].component = component_id;
        while (!stack.empty()) {
            element_id = stack.back();
            stack.pop_back();
            for (SetId set_id: element(element_id).sets) {
                if (set(set_id).component != -1)
                    continue;
                sets_[set_id].component = component_id;
                for (ElementId element_id_next: set(set_id).elements) {
                    if (element(element_id_next).component != -1)
                        continue;
                    elements_[element_id_next].component = component_id;
                    stack.push_back(element_id_next);
                }
            }
        }
    }

    for (ElementId element_id = 0;
            element_id < number_of_elements();
            ++element_id) {
        components_[element(element_id).component].elements.push_back(element_id);
    }
    for (SetId set_id = 0; set_id < number_of_sets(); ++set_id) {
        if (set(set_id).component != -1)
            components_[set(set_id).component].sets.push_back(set_id);
    }
}

void Instance::read_geccod2020(std::ifstream& file)
{
    ElementId number_of_elements;
    SetId number_of_sets;
    file >> number_of_elements >> number_of_sets;

    elements_.resize(number_of_elements);
    sets_.resize(number_of_sets);

    for (SetId set_id = 0; set_id < number_of_sets; ++set_id) {
        sets_[set_id].cost = 1;
    }

    ElementId element_id_tmp;
    SetId element_number_of_sets;
    SetId set_id;
    for (ElementId element_id = 0;
            element_id < number_of_elements;
            ++element_id) {
        file >> element_id_tmp >> element_number_of_sets;
        number_of_arcs_ += element_number_of_sets;
        for (SetPos set_pos = 0; set_pos < element_number_of_sets; ++set_pos) {
            file >> set_id;
            elements_[element_id].sets.push_back(set_id);
            sets_[set_id].elements.push_back(element_id);
        }
    }
    total_cost_ = number_of_sets;
}

void Instance::read_fulkerson1974(std::ifstream& file)
{
    SetId number_of_sets;
    ElementId number_of_elements;
    file >> number_of_sets >> number_of_elements;

    sets_.resize(number_of_sets);
    elements_.resize(number_of_elements);

    for (SetId set_id = 0; set_id < number_of_sets; ++set_id) {
        sets_[set_id].cost = 1;
    }

    SetId set_id;
    for (ElementId element_id = 0;
            element_id < number_of_elements;
            ++element_id) {
        number_of_arcs_ += 3;
        for (SetPos set_pos = 0; set_pos < 3; ++set_pos) {
            file >> set_id;
            set_id--;
            elements_[element_id].sets.push_back(set_id);
            sets_[set_id].elements.push_back(element_id);
        }
    }
    total_cost_ = number_of_sets;
}

void Instance::read_balas1980(std::ifstream& file)
{
    ElementId number_of_elements;
    SetId number_of_sets;
    file >> number_of_elements >> number_of_sets;

    elements_.resize(number_of_elements);
    sets_.resize(number_of_sets);

    Cost cost;
    for (SetId set_id = 0; set_id < number_of_sets; ++set_id) {
        file >> cost;
        sets_[set_id].cost = cost;
        total_cost_ += cost;
    }

    SetId set_id;
    SetId element_number_of_sets;
    for (ElementId element_id = 0; element_id < number_of_elements; ++element_id) {
        file >> element_number_of_sets;
        number_of_arcs_ += element_number_of_sets;
        for (SetPos set_pos = 0; set_pos < element_number_of_sets; ++set_pos) {
            file >> set_id;
            set_id--;
            elements_[element_id].sets.push_back(set_id);
            sets_[set_id].elements.push_back(element_id);
        }
    }
}

void Instance::read_balas1996(std::ifstream& file)
{
    SetId number_of_sets;
    ElementId number_of_elements;
    file >> number_of_sets >> number_of_elements;

    sets_.resize(number_of_sets);
    elements_.resize(number_of_elements);

    Cost cost;
    for (SetId set_id = 0; set_id < number_of_sets; ++set_id) {
        file >> cost;
        sets_[set_id].cost = cost;
        total_cost_ += cost;
    }

    ElementId element_id;
    ElementId set_number_of_elements;
    for (SetId set_id = 0; set_id < number_of_sets; ++set_id) {
        file >> set_number_of_elements;
        number_of_arcs_ += set_number_of_elements;
        for (ElementPos element_pos = 0; element_pos < set_number_of_elements; ++element_pos) {
            file >> element_id;
            element_id--;
            assert(element_id >= 0);
            assert(element_id < number_of_elements);
            sets_[set_id].elements.push_back(element_id);
            elements_[element_id].sets.push_back(set_id);
        }
    }
}

void Instance::read_faster1994(std::ifstream& file)
{
    ElementId number_of_elements;
    SetId number_of_sets;
    file >> number_of_elements >> number_of_sets;

    elements_.resize(number_of_elements);
    sets_.resize(number_of_sets);

    Cost cost;
    ElementId element_id;
    ElementId set_number_of_elements;
    for (SetId set_id = 0; set_id < number_of_sets; ++set_id) {
        file >> cost >> set_number_of_elements;
        set_cost(set_id, cost);
        for (ElementPos element_pos = 0; element_pos < set_number_of_elements; ++element_pos) {
            file >> element_id;
            element_id--;
            assert(element_id >= 0);
            assert(element_id < number_of_elements);
            add_arc(set_id, element_id);
        }
    }
}

void Instance::set_unicost()
{
    for (SetId set_id = 0; set_id < number_of_sets(); ++set_id) {
        set_cost(set_id, 1);
    }
}

std::ostream& Instance::print(
        std::ostream& os,
        int verbose) const
{
    if (verbose >= 1) {
        os
            << "Number of elements:                           " << number_of_elements() << std::endl
            << "Number of sets:                               " << number_of_sets() << std::endl
            << "Number of arcs:                               " << number_of_arcs() << std::endl
            << "Average number of sets covering an element:   " << (double)number_of_arcs() / number_of_elements() << std::endl
            << "Average number of elements covered by a set:  " << (double)number_of_arcs() / number_of_sets() << std::endl
            << "Total cost:                                   " << total_cost() << std::endl
            << "Number of connected components:               " << number_of_components() << std::endl
            ;
    }

    if (verbose >= 2) {
        os << std::endl
            << std::setw(12) << "SetId"
            << std::setw(12) << "Cost"
            << std::setw(12) << "# elem."
            << std::endl
            << std::setw(12) << "-----"
            << std::setw(12) << "----"
            << std::setw(12) << "-------"
            << std::endl;
        for (SetId set_id = 0;
                set_id < number_of_sets();
                ++set_id) {
            const Set& set = this->set(set_id);
            os
                << std::setw(12) << set_id
                << std::setw(12) << set.cost
                << std::setw(12) << set.elements.size()
                << std::endl;
        }
    }

    if (verbose >= 3) {
        os << std::endl
            << std::setw(12) << "Set"
            << std::setw(12) << "Element"
            << std::endl
            << std::setw(12) << "---"
            << std::setw(12) << "-------"
            << std::endl;
        for (SetId set_id = 0;
                set_id < number_of_sets();
                ++set_id) {
            const Set& set = this->set(set_id);
            for (ElementId element_id: set.elements) {
                os
                    << std::setw(12) << set_id
                    << std::setw(12) << element_id
                    << std::endl;
            }
        }
    }

    return os;
}

void Instance::write(std::string instance_path, std::string format)
{
    std::ofstream file(instance_path);
    if (!file.good()) {
        throw std::runtime_error(
                "Unable to open file \"" + instance_path + "\".");
    }

    if (format == "gecco2020" || format == "gecco") {
        //write_geccod2020(file);
    } else if (format == "fulkerson1974" || format == "sts") {
        //write_fulkerson1974(file);
    } else if (format == "balas1980" || format == "orlibrary") {
        write_balas1980(file);
    } else if (format == "balas1996") {
        //write_balas1996(file);
    } else if (format == "faster1994" || format == "faster" || format == "wedelin1995" || format == "wedelin") {
        //write_faster1994(file);
    } else {
        throw std::invalid_argument(
                "Unknown instance format \"" + format + "\".");
    }

    compute_components();
}

void Instance::write_balas1980(std::ofstream& file)
{
    file << number_of_elements() << " " << number_of_sets() << std::endl;
    for (SetId set_id = 0; set_id < number_of_sets(); ++set_id)
        file << " " << set(set_id).cost;
    file << std::endl;

    for (ElementId element_id = 0;
            element_id < number_of_elements();
            ++element_id) {
        file << element(element_id).sets.size();
        for (SetId s: element(element_id).sets)
            file << " " << (s + 1);
        file << std::endl;
    }
}

bool Instance::reduce_mandatory_sets()
{
    //std::cout << "reduce_mandatory_sets..." << std::endl;
    optimizationtools::IndexedSet fixed_sets(number_of_sets());
    optimizationtools::IndexedSet elements_to_remove(number_of_elements());
    for (ElementId element_id = 0;
            element_id < number_of_elements();
            ++element_id) {
        const Element& element = this->element(element_id);
        if (element.sets.size() == 1) {
            SetId set_id = element.sets.front();
            fixed_sets.add(set_id);
            for (ElementId element_id_2: set(set_id).elements) {
                elements_to_remove.add(element_id_2);
            }
        }
    }
    //std::cout << fixed_sets.size() << std::endl;

    if (fixed_sets.size() == 0)
        return false;

    UnreductionInfo new_unreduction_info;
    new_unreduction_info.original_instance = unreduction_info_.original_instance;

    // Update mandatory_sets.
    new_unreduction_info.mandatory_sets = unreduction_info_.mandatory_sets;
    for (SetId set_id: fixed_sets) {
        SetId orig_item_id = unreduction_info_.unreduction_operations[set_id];
        new_unreduction_info.mandatory_sets.push_back(orig_item_id);
    }
    // Create new instance and compute unreduction_operations.
    SetId new_number_of_sets = number_of_sets() - fixed_sets.size();
    SetId new_number_of_elements = number_of_elements() - elements_to_remove.size();
    Instance new_instance(
            new_number_of_sets,
            new_number_of_elements);
    new_unreduction_info.unreduction_operations = std::vector<SetId>(new_number_of_sets);
    // Add sets.
    std::vector<SetId> sets_original2reduced(number_of_sets(), -1);
    std::vector<ElementId> elements_original2reduced(number_of_elements(), -1);
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
        new_instance.set_cost(new_set_id, set(set_id).cost);
        new_unreduction_info.unreduction_operations[new_set_id]
            = unreduction_info_.unreduction_operations[set_id];
        new_set_id++;
    }
    // Add arcs.
    for (auto it = fixed_sets.out_begin(); it != fixed_sets.out_end(); ++it) {
        SetId set_id = *it;
        SetId new_set_id = sets_original2reduced[set_id];
        if (new_set_id == -1)
            continue;
        for (ElementId element_id: set(set_id).elements) {
            ElementId new_element_id = elements_original2reduced[element_id];
            if (new_element_id == -1)
                continue;
            new_instance.add_arc(
                    new_set_id,
                    new_element_id);
        }
    }

    new_instance.unreduction_info_ = new_unreduction_info;
    new_instance.compute_components();
    *this = new_instance;
    //print(std::cout, 1);
    return true;
}

bool Instance::reduce_identical_elements()
{
    //std::cout << "reduce_identical_elements..." << std::endl;
    optimizationtools::IndexedSet elements_to_remove(number_of_elements());
    std::vector<ElementId> elements_sorted(number_of_elements());
    std::iota(elements_sorted.begin(), elements_sorted.end(), 0);
    sort(elements_sorted.begin(), elements_sorted.end(),
            [this](ElementId element_id_1, ElementId element_id_2) -> bool
    {
        if (element(element_id_1).sets.size() != element(element_id_2).sets.size())
            return element(element_id_1).sets.size() < element(element_id_2).sets.size();
        for (SetPos set_pos = 0;
                set_pos < (SetPos)element(element_id_1).sets.size();
                ++set_pos) {
            if (element(element_id_1).sets[set_pos]
                    != element(element_id_2).sets[set_pos]) {
                return (element(element_id_1).sets[set_pos] < element(element_id_2).sets[set_pos]);
            }
        }
        return element_id_2 < element_id_1;
    });
    ElementId element_id_pred = -1;
    for (ElementId element_id: elements_sorted) {
        if (element_id_pred != -1) {
            if (element(element_id_pred).sets == element(element_id).sets)
                elements_to_remove.add(element_id_pred);
        }
        element_id_pred = element_id;
    }
    //std::cout << elements_to_remove.size() << std::endl;

    if (elements_to_remove.size() == 0)
        return false;

    UnreductionInfo new_unreduction_info;
    new_unreduction_info.original_instance = unreduction_info_.original_instance;

    // Update mandatory_sets.
    new_unreduction_info.mandatory_sets = unreduction_info_.mandatory_sets;
    // Create new instance and compute unreduction_operations.
    ElementId new_number_of_elements = number_of_elements() - elements_to_remove.size();
    Instance new_instance(
            number_of_sets(),
            new_number_of_elements);
    new_unreduction_info.unreduction_operations = std::vector<SetId>(number_of_sets());
    std::vector<ElementId> original2reduced(number_of_elements(), -1);
    ElementId new_element_id = 0;
    for (auto it = elements_to_remove.out_begin();
            it != elements_to_remove.out_end();
            ++it) {
        ElementId element_id = *it;
        original2reduced[element_id] = new_element_id;
        new_element_id++;
    }
    // Add sets and arcs.
    for (SetId set_id = 0; set_id < number_of_sets(); ++set_id) {
        new_instance.set_cost(set_id, set(set_id).cost);
        new_unreduction_info.unreduction_operations[set_id]
            = unreduction_info_.unreduction_operations[set_id];
        for (ElementId element_id: set(set_id).elements) {
            ElementId new_element_id = original2reduced[element_id];
            if (new_element_id == -1)
                continue;
            new_instance.add_arc(
                    set_id,
                    new_element_id);
        }
    }

    new_instance.unreduction_info_ = new_unreduction_info;
    new_instance.compute_components();
    *this = new_instance;
    //print(std::cout, 1);
    return true;
}

bool Instance::reduce_identical_sets()
{
    //std::cout << "reduce_identical_sets..." << std::endl;
    optimizationtools::IndexedSet sets_to_remove(number_of_sets());
    std::vector<ElementId> sets_sorted(number_of_sets());
    std::iota(sets_sorted.begin(), sets_sorted.end(), 0);
    sort(sets_sorted.begin(), sets_sorted.end(),
            [this](SetId set_id_1, SetId set_id_2) -> bool
    {
        if (set(set_id_1).elements.size() != set(set_id_2).elements.size())
            return set(set_id_1).elements.size() < set(set_id_2).elements.size();
        for (ElementPos element_pos = 0; element_pos < (ElementPos)set(set_id_1).elements.size(); ++element_pos)
            if (set(set_id_1).elements[element_pos] != set(set_id_2).elements[element_pos])
                return (set(set_id_1).elements[element_pos] < set(set_id_2).elements[element_pos]);
        return set(set_id_1).cost > set(set_id_2).cost;
    });
    SetId set_id_pred = -1;
    for (SetId set_id: sets_sorted) {
        if (set(set_id).elements.size() == 0) {
            sets_to_remove.add(set_id);
            continue;
        }
        if (set_id_pred != -1) {
            if (set(set_id_pred).cost >= set(set_id).cost
                    && set(set_id_pred).elements == set(set_id).elements)
                sets_to_remove.add(set_id_pred);
        }
        set_id_pred = set_id;
    }
    //std::cout << sets_to_remove.size() << std::endl;

    if (sets_to_remove.size() == 0)
        return false;

    UnreductionInfo new_unreduction_info;
    new_unreduction_info.original_instance = unreduction_info_.original_instance;

    // Update mandatory_sets.
    new_unreduction_info.mandatory_sets = unreduction_info_.mandatory_sets;
    // Create new instance and compute unreduction_operations.
    SetId new_number_of_sets = number_of_sets() - sets_to_remove.size();
    Instance new_instance(new_number_of_sets, number_of_elements());
    new_unreduction_info.unreduction_operations = std::vector<SetId>(new_number_of_sets);
    // Add sets.
    std::vector<SetId> original2reduced(number_of_sets(), -1);
    SetId new_set_id = 0;
    for (auto it = sets_to_remove.out_begin();
            it != sets_to_remove.out_end(); ++it) {
        SetId set_id = *it;
        original2reduced[set_id] = new_set_id;
        new_instance.set_cost(new_set_id, set(set_id).cost);
        new_unreduction_info.unreduction_operations[new_set_id]
            = unreduction_info_.unreduction_operations[set_id];
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
        for (ElementId element_id: set(set_id).elements) {
            new_instance.add_arc(
                    new_set_id,
                    element_id);
        }
    }

    new_instance.unreduction_info_ = new_unreduction_info;
    new_instance.compute_components();
    *this = new_instance;
    //print(std::cout, 1);
    return true;
}

bool Instance::reduce_domianted_elements()
{
    //std::cout << "reduce_domianted_elements..." << std::endl;
    optimizationtools::IndexedSet elements_to_remove(number_of_elements());
    optimizationtools::IndexedSet covered_sets(number_of_sets());
    for (ElementId element_id_1 = 0;
            element_id_1 < number_of_elements();
            ++element_id_1) {
        covered_sets.clear();
        for (SetId set_id: element(element_id_1).sets)
            covered_sets.add(set_id);
        for (ElementId element_id_2 = 0;
                element_id_2 < number_of_elements();
                ++element_id_2) {
            if (element_id_2 == element_id_1
                    || element(element_id_2).sets.size()
                    > element(element_id_1).sets.size())
                continue;
            // Check if element_id_2 dominates element_id_1
            bool dominates = true;
            for (SetId set_id: element(element_id_2).sets) {
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

    UnreductionInfo new_unreduction_info;
    new_unreduction_info.original_instance = unreduction_info_.original_instance;

    // Update mandatory_sets.
    new_unreduction_info.mandatory_sets = unreduction_info_.mandatory_sets;
    // Create new instance and compute unreduction_operations.
    Instance new_instance(
            number_of_sets(),
            number_of_elements() - elements_to_remove.size());
    new_unreduction_info.unreduction_operations = std::vector<SetId>(number_of_sets());
    std::vector<ElementId> original2reduced(number_of_elements(), -1);
    ElementId new_element_id = 0;
    for (auto it = elements_to_remove.out_begin();
            it != elements_to_remove.out_end();
            ++it) {
        ElementId element_id = *it;
        original2reduced[element_id] = new_element_id;
        new_element_id++;
    }
    // Add sets and arcs.
    for (SetId set_id = 0; set_id < number_of_sets(); ++set_id) {
        new_instance.set_cost(set_id, set(set_id).cost);
        new_unreduction_info.unreduction_operations[set_id]
            = unreduction_info_.unreduction_operations[set_id];
        for (ElementId element_id: set(set_id).elements) {
            ElementId new_element_id = original2reduced[element_id];
            if (new_element_id == -1)
                continue;
            new_instance.add_arc(
                    set_id,
                    new_element_id);
        }
    }

    new_instance.unreduction_info_ = new_unreduction_info;
    new_instance.compute_components();
    *this = new_instance;
    return true;
}

bool Instance::reduce_domianted_sets()
{
    //std::cout << "reduce_domianted_sets..." << std::endl;
    optimizationtools::IndexedSet sets_to_remove(number_of_sets());
    optimizationtools::IndexedSet covered_elements(number_of_elements());
    for (SetId set_id_1 = 0;
            set_id_1 < number_of_sets();
            ++set_id_1) {
        covered_elements.clear();
        for (ElementId element_id: set(set_id_1).elements)
            covered_elements.add(element_id);
        for (SetId set_id_2 = 0;
                set_id_2 < number_of_sets();
                ++set_id_2) {
            if (set_id_2 == set_id_1
                    || set(set_id_1).elements.size()
                    < set(set_id_2).elements.size()
                    || set(set_id_1).cost > set(set_id_2).cost)
                continue;
            // Check if set_id_1 dominates set_id_2
            bool dominates = true;
            for (ElementId element_id: set(set_id_2).elements) {
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

    UnreductionInfo new_unreduction_info;
    new_unreduction_info.original_instance = unreduction_info_.original_instance;

    // Update mandatory_sets.
    new_unreduction_info.mandatory_sets = unreduction_info_.mandatory_sets;
    // Create new instance and compute unreduction_operations.
    SetId new_number_of_sets = number_of_sets() - sets_to_remove.size();
    Instance new_instance(new_number_of_sets, number_of_elements());
    new_unreduction_info.unreduction_operations = std::vector<SetId>(new_number_of_sets);
    // Add sets.
    std::vector<SetId> original2reduced(number_of_sets(), -1);
    SetId new_set_id = 0;
    for (auto it = sets_to_remove.out_begin();
            it != sets_to_remove.out_end(); ++it) {
        SetId set_id = *it;
        original2reduced[set_id] = new_set_id;
        new_instance.set_cost(new_set_id, set(set_id).cost);
        new_unreduction_info.unreduction_operations[new_set_id]
            = unreduction_info_.unreduction_operations[set_id];
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
        for (ElementId element_id: set(set_id).elements) {
            new_instance.add_arc(
                    new_set_id,
                    element_id);
        }
    }

    new_instance.unreduction_info_ = new_unreduction_info;
    new_instance.compute_components();
    *this = new_instance;
    return true;
}

Instance Instance::reduce(ReductionParameters parameters) const
{
    // Initialize reduced instance.
    Instance new_instance = *this;
    new_instance.unreduction_info_ = UnreductionInfo();
    new_instance.unreduction_info_.original_instance = this;
    new_instance.unreduction_info_.unreduction_operations = std::vector<SetId>(number_of_sets());
    for (SetId set_id = 0;
            set_id < number_of_sets();
            ++set_id) {
        new_instance.unreduction_info_.unreduction_operations[set_id] = set_id;
    }

    for (Counter round_number = 0;
            round_number < parameters.maximum_number_of_rounds;
            ++round_number) {
        bool found = false;
        found |= new_instance.reduce_mandatory_sets();
        found |= new_instance.reduce_identical_elements();
        found |= new_instance.reduce_identical_sets();
        if (found)
            continue;
        if (parameters.remove_domianted) {
            found |= new_instance.reduce_domianted_elements();
            found |= new_instance.reduce_domianted_sets();
        }
        if (!found)
            break;
    }

    new_instance.unreduction_info_.extra_cost = 0;
    for (SetId orig_set_id: new_instance.unreduction_info_.mandatory_sets)
        new_instance.unreduction_info_.extra_cost += set(orig_set_id).cost;

    return new_instance;
}

void setcoveringsolver::init_display(
        const Instance& instance,
        optimizationtools::Info& info)
{
    info.os()
        << "=====================================" << std::endl
        << "         Set Covering Solver         " << std::endl
        << "=====================================" << std::endl
        << std::endl
        << "Instance" << std::endl
        << "--------" << std::endl;
    instance.print(info.os(), info.verbosity_level());
    info.os() << std::endl;
}

