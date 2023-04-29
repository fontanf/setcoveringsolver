#include "setcoveringsolver/instance.hpp"

#include <sstream>
#include <iomanip>
#include <thread>

using namespace setcoveringsolver;

Instance::Instance(
        std::string instance_path,
        std::string format):
    fixed_sets_(0),
    fixed_elements_(0)
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

    fixed_sets_ = optimizationtools::IndexedSet(number_of_sets());
    fixed_elements_ = optimizationtools::IndexedSet(number_of_elements());

    compute_components();
}

Instance::Instance(
        SetId number_of_sets,
        ElementId number_of_elements):
    elements_(number_of_elements),
    sets_(number_of_sets),
    total_cost_(number_of_sets),
    fixed_sets_(number_of_sets),
    fixed_elements_(number_of_elements)
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

void Instance::fix_identical(optimizationtools::Info& info)
{
    info.os()
            << "Reduction" << std::endl
            << "---------" << std::endl;

    optimizationtools::IndexedSet elements_to_remove(number_of_elements());
    optimizationtools::IndexedSet sets_to_remove(number_of_sets());

    std::vector<ElementId> elements_sorted(
            fixed_elements_.out_begin(),
            fixed_elements_.out_end());
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
    remove_elements(elements_to_remove);
    info.os() << "Number of unfixed elements:  " << number_of_unfixed_elements() << "/" << number_of_elements()
            << " (" << elements_to_remove.size() << " fixed)"
            << std::endl;

    sets_to_remove.clear();
    std::vector<SetId> sets_sorted(fixed_sets_.out_begin(), fixed_sets_.out_end());
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
    remove_sets(sets_to_remove, info);
    info.os()
            << "Number of unfixed sets:      " << number_of_unfixed_sets() << "/" << number_of_sets()
            << " (" << sets_to_remove.size() << " fixed)"
            << std::endl
            << std::endl;
}

void Instance::fix_dominated(optimizationtools::Info& info)
{
    info.os() << "Fix dominated elements and sets..." << std::endl;
    optimizationtools::IndexedSet elements_to_remove(number_of_elements());
    optimizationtools::IndexedSet sets_to_remove(number_of_sets());

    elements_to_remove.clear();
    optimizationtools::IndexedSet covered_sets(number_of_sets());
    for (auto it1 = fixed_elements_.out_begin(); it1 != fixed_elements_.out_end(); ++it1) {
        ElementId element_id_1 = *it1;
        covered_sets.clear();
        for (SetId set_id: element(element_id_1).sets)
            covered_sets.add(set_id);
        for (auto it2 = fixed_elements_.out_begin(); it2 != fixed_elements_.out_end(); ++it2) {
            ElementId element_id_2 = *it2;
            if (element_id_2 == element_id_1 || element(element_id_2).sets.size() > element(element_id_1).sets.size())
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
    remove_elements(elements_to_remove);
    info.os() << "* Element number: " << number_of_unfixed_elements() << "/" << number_of_elements()
            << " (" << elements_to_remove.size() << " fixed)"
            << std::endl;

    sets_to_remove.clear();
    optimizationtools::IndexedSet covered_elements(number_of_elements());
    for (auto it1 = fixed_sets_.out_begin(); it1 != fixed_sets_.out_end(); ++it1) {
        SetId set_id_1 = *it1;
        covered_elements.clear();
        for (ElementId element_id: set(set_id_1).elements)
            covered_elements.add(element_id);
        for (auto it2 = fixed_sets_.out_begin(); it2 != fixed_sets_.out_end(); ++it2) {
            SetId set_id_2 = *it2;
            if (set_id_2 == set_id_1
                    || set(set_id_1).elements.size() < set(set_id_2).elements.size()
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
    remove_sets(sets_to_remove, info);
    info.os() << "* Set number: " << number_of_unfixed_sets() << "/" << number_of_sets()
            << " (" << sets_to_remove.size() << " fixed)"
            << std::endl;
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
                && (element(element_id).component != -1 || fixed_elements_.contains(element_id)))
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

    for (auto it = fixed_elements_.out_begin(); it != fixed_elements_.out_end(); ++it)
        components_[element(*it).component].elements.push_back(*it);
    for (auto it = fixed_sets_.out_begin(); it != fixed_sets_.out_end(); ++it)
        if (set(*it).component != -1)
            components_[set(*it).component].sets.push_back(*it);
}

void Instance::remove_elements(const optimizationtools::IndexedSet& elements)
{
    optimizationtools::IndexedSet modified_sets(number_of_sets());
    for (ElementId element_id: elements) {
        fixed_elements_.add(element_id);
        for (SetId set_id: element(element_id).sets)
            modified_sets.add(set_id);
        elements_[element_id].sets.clear();
    }
    for (SetId set_id: modified_sets) {
        std::vector<ElementId> s_elements_new;
        for (ElementId element_id: set(set_id).elements)
            if (!elements.contains(element_id))
                s_elements_new.push_back(element_id);
        sets_[set_id].elements.swap(s_elements_new);
    }
}

void Instance::remove_sets(
        const optimizationtools::IndexedSet& sets,
        optimizationtools::Info& info)
{
    optimizationtools::IndexedSet modified_elements(number_of_elements());
    for (SetId set_id: sets) {
        fixed_sets_.add(set_id);
        for (ElementId element_id: set(set_id).elements)
            modified_elements.add(element_id);
        sets_[set_id].elements.clear();
    }
    for (ElementId element_id: modified_elements) {
        std::vector<SetId> e_sets_new;
        for (SetId set_id: element(element_id).sets)
            if (!sets.contains(set_id))
                e_sets_new.push_back(set_id);
        elements_[element_id].sets.swap(e_sets_new);
    }

    SetPos number_of_mandatory_sets = 0;
    for (ElementId element_id = 0;
            element_id < number_of_elements();
            ++element_id) {
        if (element(element_id).sets.size() == 1) {
            if (!sets_[element(element_id).sets.front()].mandatory) {
                sets_[element(element_id).sets.front()].mandatory = true;
                number_of_mandatory_sets++;
            }
        }
    }
    info.os() << "Number of mandatory sets:    " << number_of_mandatory_sets << "/" << number_of_sets() << std::endl;
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
        sets_[set_id].cost = cost;
        total_cost_ += cost;
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

void Instance::set_unicost()
{
    total_cost_ = 0;
    for (SetId set_id = 0; set_id < number_of_sets(); ++set_id) {
        sets_[set_id].cost = 1;
        total_cost_++;
    }
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

    fixed_sets_ = optimizationtools::IndexedSet(number_of_sets());
    fixed_elements_ = optimizationtools::IndexedSet(number_of_elements());

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
            << "--------" << std::endl
            << "Number of elements:                           " << instance.number_of_elements() << std::endl
            << "Number of sets:                               " << instance.number_of_sets() << std::endl
            << "Number of arcs:                               " << instance.number_of_arcs() << std::endl
            << "Average number of sets covering an element:   " << (double)instance.number_of_arcs() / instance.number_of_elements() << std::endl
            << "Average number of elements covered by a set:  " << (double)instance.number_of_arcs() / instance.number_of_sets() << std::endl
            << "Number of connected components:               " << instance.number_of_components() << std::endl
            << "Average cost:                                 " << (double)instance.total_cost() / instance.number_of_unfixed_sets() << std::endl
            << std::endl;
}

