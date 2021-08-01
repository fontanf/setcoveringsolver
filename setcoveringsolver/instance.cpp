#include "setcoveringsolver/instance.hpp"

#include <sstream>
#include <iomanip>
#include <thread>

using namespace setcoveringsolver;

Instance::Instance(std::string instance_path, std::string format):
    fixed_sets_(0),
    fixed_elements_(0)
{
    std::ifstream file(instance_path);
    if (!file.good())
        throw std::runtime_error(
                "Unable to open file \"" + instance_path + "\".");

    if (format == "gecco2020" || format == "gecco") {
        read_geccod2020(file);
    } else if (format == "fulkerson1974" || format == "sts") {
        read_fulkerson1974(file);
    } else if (format == "balas1980" || format == "orlibrary") {
        read_balas1980(file);
    } else if (format == "balas1996") {
        read_balas1996(file);
    } else if (format == "faster1994" || format == "faster" || format == "wedelin1995" || format == "wedelin") {
        read_faster1994(file);
    } else {
        throw std::invalid_argument(
                "Unknown instance format \"" + format + "\".");
    }

    fixed_sets_     = optimizationtools::IndexedSet(number_of_sets());
    fixed_elements_ = optimizationtools::IndexedSet(number_of_elements());

    compute_components();
}

Instance::Instance(SetId number_of_sets, ElementId number_of_elements):
    elements_(number_of_elements),
    sets_(number_of_sets),
    total_cost_(number_of_sets),
    fixed_sets_(number_of_sets),
    fixed_elements_(number_of_elements)
{
    for (ElementId e = 0; e < number_of_elements; ++e)
        elements_[e].id = e;
    for (SetId s = 0; s < number_of_sets; ++s)
        sets_[s].id = s;
}

void Instance::add_arc(SetId s, ElementId e)
{
    elements_[e].sets.push_back(s);
    sets_[s].elements.push_back(e);
    number_of_arcs_++;
}

void Instance::fix_identical(Info& info)
{
    VER(info, "Fix redundant elements and sets..." << std::endl);
    optimizationtools::IndexedSet elements_to_remove(number_of_elements());
    optimizationtools::IndexedSet sets_to_remove(number_of_sets());

    std::vector<ElementId> elements_sorted(fixed_elements_.out_begin(), fixed_elements_.out_end());
    sort(elements_sorted.begin(), elements_sorted.end(),
            [this](ElementId e1, ElementId e2) -> bool
    {
        if (element(e1).sets.size() != element(e2).sets.size())
            return element(e1).sets.size() < element(e2).sets.size();
        for (SetPos s_pos = 0; s_pos < (SetPos)element(e1).sets.size(); ++s_pos)
            if (element(e1).sets[s_pos] != element(e2).sets[s_pos])
                return (element(e1).sets[s_pos] < element(e2).sets[s_pos]);
        return e2 < e1;
    });
    ElementId e_prec = -1;
    for (ElementId e: elements_sorted) {
        if (e_prec != -1) {
            if (element(e_prec).sets == element(e).sets)
                elements_to_remove.add(e_prec);
        }
        e_prec = e;
    }
    remove_elements(elements_to_remove);
    VER(info, "* Element number: " << number_of_unfixed_elements() << "/" << number_of_elements()
            << " (" << elements_to_remove.size() << " fixed)"
            << std::endl);

    sets_to_remove.clear();
    std::vector<SetId> sets_sorted(fixed_sets_.out_begin(), fixed_sets_.out_end());
    sort(sets_sorted.begin(), sets_sorted.end(),
            [this](SetId s1, SetId s2) -> bool
    {
        if (set(s1).elements.size() != set(s2).elements.size())
            return set(s1).elements.size() < set(s2).elements.size();
        for (ElementPos e_pos = 0; e_pos < (ElementPos)set(s1).elements.size(); ++e_pos)
            if (set(s1).elements[e_pos] != set(s2).elements[e_pos])
                return (set(s1).elements[e_pos] < set(s2).elements[e_pos]);
        return set(s1).cost > set(s2).cost;
    });
    SetId s_prec = -1;
    for (SetId s: sets_sorted) {
        if (set(s).elements.size() == 0) {
            sets_to_remove.add(s);
            continue;
        }
        if (s_prec != -1) {
            if (set(s_prec).cost >= set(s).cost
                    && set(s_prec).elements == set(s).elements)
                sets_to_remove.add(s_prec);
        }
        s_prec = s;
    }
    remove_sets(sets_to_remove);
    VER(info, "* Set number: " << number_of_unfixed_sets() << "/" << number_of_sets()
            << " (" << sets_to_remove.size() << " fixed)"
            << std::endl);
}

void Instance::fix_dominated(Info& info)
{
    VER(info, "Fix dominated elements and sets..." << std::endl);
    optimizationtools::IndexedSet elements_to_remove(number_of_elements());
    optimizationtools::IndexedSet sets_to_remove(number_of_sets());

    elements_to_remove.clear();
    optimizationtools::IndexedSet covered_sets(number_of_sets());
    for (auto it1 = fixed_elements_.out_begin(); it1 != fixed_elements_.out_end(); ++it1) {
        ElementId e1 = *it1;
        covered_sets.clear();
        for (SetId s: element(e1).sets)
            covered_sets.add(s);
        for (auto it2 = fixed_elements_.out_begin(); it2 != fixed_elements_.out_end(); ++it2) {
            ElementId e2 = *it2;
            if (e2 == e1 || element(e2).sets.size() > element(e1).sets.size())
                continue;
            // Check if e2 dominates e1
            bool dominates = true;
            for (SetId s: element(e2).sets) {
                if (!covered_sets.contains(s)) {
                    dominates = false;
                    break;
                }
            }
            if (dominates) {
                elements_to_remove.add(e1);
                break;
            }
        }
    }
    remove_elements(elements_to_remove);
    VER(info, "* Element number: " << number_of_unfixed_elements() << "/" << number_of_elements()
            << " (" << elements_to_remove.size() << " fixed)"
            << std::endl);

    sets_to_remove.clear();
    optimizationtools::IndexedSet covered_elements(number_of_elements());
    for (auto it1 = fixed_sets_.out_begin(); it1 != fixed_sets_.out_end(); ++it1) {
        SetId s1 = *it1;
        covered_elements.clear();
        for (ElementId e: set(s1).elements)
            covered_elements.add(e);
        for (auto it2 = fixed_sets_.out_begin(); it2 != fixed_sets_.out_end(); ++it2) {
            SetId s2 = *it2;
            if (s2 == s1
                    || set(s1).elements.size() < set(s2).elements.size()
                    || set(s1).cost > set(s2).cost)
                continue;
            // Check if s1 dominates s2
            bool dominates = true;
            for (ElementId e: set(s2).elements) {
                if (!covered_elements.contains(e)) {
                    dominates = false;
                    break;
                }
            }
            if (dominates)
                sets_to_remove.add(s2);
        }
    }
    remove_sets(sets_to_remove);
    VER(info, "* Set number: " << number_of_unfixed_sets() << "/" << number_of_sets()
            << " (" << sets_to_remove.size() << " fixed)"
            << std::endl);
}

void Instance::compute_set_neighbors(Counter number_of_threads, Info& info)
{
    VER(info, "Compute set neighbors..." << std::endl);
    std::vector<std::thread> threads;
    for (Counter thread_id = 0; thread_id < number_of_threads; ++thread_id)
        threads.push_back(std::thread(&Instance::compute_set_neighbors_worker,
                    this,
                    thread_id       * number_of_sets() / number_of_threads,
                    (thread_id + 1) * number_of_sets() / number_of_threads));
    for (Counter thread_id = 0; thread_id < number_of_threads; ++thread_id)
        threads[thread_id].join();
}

void Instance::compute_set_neighbors_worker(SetId s_start, SetId s_end)
{
    optimizationtools::IndexedSet neighbors(number_of_sets());
    for (SetId s1 = s_start; s1 < s_end; ++s1) {
        neighbors.clear();
        for (ElementId e: set(s1).elements)
            for (SetId s2: element(e).sets)
                if (s2 != s1)
                    neighbors.add(s2);
        for (SetId s2: neighbors)
            sets_[s1].neighbors.push_back(s2);
    }
}

void Instance::compute_element_neighbors(Info& info)
{
    VER(info, "Compute element neighbors..." << std::endl);
    optimizationtools::IndexedSet neighbors(number_of_elements());
    for (ElementId e1 = 0; e1 < number_of_elements(); ++e1) {
        neighbors.clear();
        for (SetId s: element(e1).sets)
            for (ElementId e2: set(s).elements)
                if (e2 != e1)
                    neighbors.add(e2);
        for (ElementId e2: neighbors)
            elements_[e1].neighbors.push_back(e2);
    }
}

void Instance::compute_components()
{
    components_.clear();
    for (ElementId e = 0; e < number_of_elements(); ++e)
        elements_[e].component = -1;
    for (SetId s = 0; s < number_of_sets(); ++s)
        sets_[s].component = -1;

    for (ComponentId c = 0;; ++c) {
        ElementId e = 0;
        while (e < number_of_elements()
                && (element(e).component != -1 || fixed_elements_.contains(e)))
            e++;
        if (e == number_of_elements())
            break;
        components_.push_back(Component());
        components_.back().id = c;
        std::vector<ElementId> stack {e};
        elements_[e].component = c;
        while (!stack.empty()) {
            e = stack.back();
            stack.pop_back();
            for (SetId s: element(e).sets) {
                if (set(s).component != -1)
                    continue;
                sets_[s].component = c;
                for (ElementId e_suiv: set(s).elements) {
                    if (element(e_suiv).component != -1)
                        continue;
                    elements_[e_suiv].component = c;
                    stack.push_back(e_suiv);
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
    for (ElementId e: elements) {
        fixed_elements_.add(e);
        for (SetId s: element(e).sets)
            modified_sets.add(s);
        elements_[e].sets.clear();
    }
    for (SetId s: modified_sets) {
        std::vector<ElementId> s_elements_new;
        for (ElementId e: set(s).elements)
            if (!elements.contains(e))
                s_elements_new.push_back(e);
        sets_[s].elements.swap(s_elements_new);
    }
}

void Instance::remove_sets(const optimizationtools::IndexedSet& sets)
{
    optimizationtools::IndexedSet modified_elements(number_of_elements());
    for (SetId s: sets) {
        fixed_sets_.add(s);
        for (ElementId e: set(s).elements)
            modified_elements.add(e);
        sets_[s].elements.clear();
    }
    for (ElementId e: modified_elements) {
        std::vector<SetId> e_sets_new;
        for (SetId s: element(e).sets)
            if (!sets.contains(s))
                e_sets_new.push_back(s);
        elements_[e].sets.swap(e_sets_new);
    }

    for (ElementId e = 0; e < number_of_elements(); ++e)
        if (element(e).sets.size() == 1)
            sets_[element(e).sets.front()].mandatory = true;
}

void Instance::read_geccod2020(std::ifstream& file)
{
    ElementId number_of_elements;
    SetId number_of_sets;
    file >> number_of_elements >> number_of_sets;

    elements_.resize(number_of_elements);
    sets_.resize(number_of_sets);

    for (SetId s = 0; s < number_of_sets; ++s) {
        sets_[s].id = s;
        sets_[s].cost = 1;
    }

    ElementId e_tmp;
    SetId s_number;
    SetId s;
    for (ElementId e = 0; e < number_of_elements; ++e) {
        elements_[e].id = e;
        file >> e_tmp >> s_number;
        number_of_arcs_ += s_number;
        for (SetPos s_pos = 0; s_pos < s_number; ++s_pos) {
            file >> s;
            elements_[e].sets.push_back(s);
            sets_[s].elements.push_back(e);
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

    for (SetId s = 0; s < number_of_sets; ++s) {
        sets_[s].id = s;
        sets_[s].cost = 1;
    }

    SetId s;
    for (ElementId e = 0; e < number_of_elements; ++e) {
        elements_[e].id = e;
        number_of_arcs_ += 3;
        for (SetPos s_pos = 0; s_pos < 3; ++s_pos) {
            file >> s;
            s--;
            elements_[e].sets.push_back(s);
            sets_[s].elements.push_back(e);
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
    for (SetId s = 0; s < number_of_sets; ++s) {
        sets_[s].id = s;
        file >> cost;
        sets_[s].cost = cost;
        total_cost_ += cost;
    }

    SetId s;
    SetId s_number;
    for (ElementId e = 0; e < number_of_elements; ++e) {
        elements_[e].id = e;
        file >> s_number;
        number_of_arcs_ += s_number;
        for (SetPos s_pos = 0; s_pos < s_number; ++s_pos) {
            file >> s;
            s--;
            elements_[e].sets.push_back(s);
            sets_[s].elements.push_back(e);
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
    for (SetId s = 0; s < number_of_sets; ++s) {
        sets_[s].id = s;
        file >> cost;
        sets_[s].cost = cost;
        total_cost_ += cost;
    }

    ElementId e;
    ElementId e_number;
    for (SetId s = 0; s < number_of_sets; ++s) {
        file >> e_number;
        number_of_arcs_ += e_number;
        for (ElementPos e_pos = 0; e_pos < e_number; ++e_pos) {
            file >> e;
            e--;
            assert(e >= 0);
            assert(e < number_of_elements);
            sets_[s].elements.push_back(e);
            elements_[e].sets.push_back(s);
        }
    }

    for (ElementId e = 0; e < number_of_elements; ++e)
        elements_[e].id = e;
}

void Instance::read_faster1994(std::ifstream& file)
{
    ElementId number_of_elements;
    SetId number_of_sets;
    file >> number_of_elements >> number_of_sets;

    elements_.resize(number_of_elements);
    sets_.resize(number_of_sets);

    Cost cost;
    ElementId e;
    ElementId e_number;
    for (SetId s = 0; s < number_of_sets; ++s) {
        file >> cost >> e_number;
        sets_[s].cost = cost;
        total_cost_ += cost;
        number_of_arcs_ += e_number;
        for (ElementPos e_pos = 0; e_pos < e_number; ++e_pos) {
            file >> e;
            e--;
            assert(e >= 0);
            assert(e < number_of_elements);
            sets_[s].elements.push_back(e);
            elements_[e].sets.push_back(s);
        }
    }

    for (ElementId e = 0; e < number_of_elements; ++e)
        elements_[e].id = e;
}

void Instance::set_unicost()
{
    for (SetId s = 0; s < number_of_sets(); ++s)
        sets_[s].cost = 1;
}

