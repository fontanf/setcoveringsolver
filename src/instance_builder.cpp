#include "setcoveringsolver/instance_builder.hpp"

#include <fstream>

using namespace setcoveringsolver;

void InstanceBuilder::add_sets(SetId number_of_sets)
{
    instance_.sets_.insert(instance_.sets_.end(), number_of_sets, Set());
}

void InstanceBuilder::add_elements(ElementId number_of_elements)
{
    instance_.elements_.insert(instance_.elements_.end(), number_of_elements, Element());
}

void InstanceBuilder::set_cost(
        SetId set_id,
        Cost cost)
{
    instance_.sets_[set_id].cost = cost;
}

void InstanceBuilder::add_arc(
        SetId set_id,
        ElementId element_id)
{
    instance_.check_set_index(set_id);
    instance_.check_element_index(element_id);

    instance_.elements_[element_id].sets.push_back(set_id);
    instance_.sets_[set_id].elements.push_back(element_id);
}

void InstanceBuilder::set_unicost()
{
    for (SetId set_id = 0; set_id < instance_.number_of_sets(); ++set_id)
        set_cost(set_id, 1);
}

void InstanceBuilder::read(
        const std::string& instance_path,
        const std::string& format)
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
}

void InstanceBuilder::read_geccod2020(std::ifstream& file)
{
    ElementId number_of_elements;
    SetId number_of_sets;
    file >> number_of_elements >> number_of_sets;

    add_elements(number_of_elements);
    add_sets(number_of_sets);

    for (SetId set_id = 0; set_id < number_of_sets; ++set_id)
        set_cost(set_id, 1);

    ElementId element_id_tmp;
    SetId element_number_of_sets;
    SetId set_id;
    for (ElementId element_id = 0;
            element_id < number_of_elements;
            ++element_id) {
        file >> element_id_tmp >> element_number_of_sets;
        for (SetPos set_pos = 0; set_pos < element_number_of_sets; ++set_pos) {
            file >> set_id;
            add_arc(set_id, element_id);
        }
    }
}

void InstanceBuilder::read_fulkerson1974(std::ifstream& file)
{
    SetId number_of_sets;
    ElementId number_of_elements;
    file >> number_of_sets >> number_of_elements;

    add_elements(number_of_elements);
    add_sets(number_of_sets);

    for (SetId set_id = 0; set_id < number_of_sets; ++set_id)
        set_cost(set_id, 1);

    SetId set_id;
    for (ElementId element_id = 0;
            element_id < number_of_elements;
            ++element_id) {
        for (SetPos set_pos = 0; set_pos < 3; ++set_pos) {
            file >> set_id;
            add_arc(set_id - 1, element_id);
        }
    }
}

void InstanceBuilder::read_balas1980(std::ifstream& file)
{
    ElementId number_of_elements;
    SetId number_of_sets;
    file >> number_of_elements >> number_of_sets;

    add_elements(number_of_elements);
    add_sets(number_of_sets);

    Cost cost;
    for (SetId set_id = 0; set_id < number_of_sets; ++set_id) {
        file >> cost;
        set_cost(set_id, cost);
    }

    SetId set_id;
    SetId element_number_of_sets;
    for (ElementId element_id = 0;
            element_id < number_of_elements;
            ++element_id) {
        file >> element_number_of_sets;
        for (SetPos set_pos = 0; set_pos < element_number_of_sets; ++set_pos) {
            file >> set_id;
            add_arc(set_id - 1, element_id);
        }
    }
}

void InstanceBuilder::read_balas1996(std::ifstream& file)
{
    SetId number_of_sets;
    ElementId number_of_elements;
    file >> number_of_sets >> number_of_elements;

    add_elements(number_of_elements);
    add_sets(number_of_sets);

    Cost cost;
    for (SetId set_id = 0; set_id < number_of_sets; ++set_id) {
        file >> cost;
        set_cost(set_id, cost);
    }

    ElementId element_id;
    ElementId set_number_of_elements;
    for (SetId set_id = 0; set_id < number_of_sets; ++set_id) {
        file >> set_number_of_elements;
        for (ElementPos element_pos = 0;
                element_pos < set_number_of_elements;
                ++element_pos) {
            file >> element_id;
            element_id--;
            add_arc(set_id, element_id - 1);
        }
    }
}

void InstanceBuilder::read_faster1994(std::ifstream& file)
{
    ElementId number_of_elements;
    SetId number_of_sets;
    file >> number_of_elements >> number_of_sets;

    add_elements(number_of_elements);
    add_sets(number_of_sets);

    Cost cost;
    ElementId element_id;
    ElementId set_number_of_elements;
    for (SetId set_id = 0; set_id < number_of_sets; ++set_id) {
        file >> cost >> set_number_of_elements;
        set_cost(set_id, cost);
        for (ElementPos element_pos = 0;
                element_pos < set_number_of_elements;
                ++element_pos) {
            file >> element_id;
            add_arc(set_id, element_id - 1);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// Build /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void InstanceBuilder::compute_number_of_arcs()
{
    instance_.total_cost_ = 0;
    for (SetId set_id = 0; set_id < instance_.number_of_sets(); ++set_id)
        instance_.total_cost_ += instance_.set(set_id).cost;
}

void InstanceBuilder::compute_total_cost()
{
    instance_.number_of_arcs_ = 0;
    for (SetId set_id = 0; set_id < instance_.number_of_sets(); ++set_id)
        instance_.number_of_arcs_ += instance_.set(set_id).elements.size();
}

void InstanceBuilder::compute_components()
{
    instance_.components_.clear();
    for (ElementId element_id = 0;
            element_id < instance_.number_of_elements();
            ++element_id)
        instance_.elements_[element_id].component = -1;
    for (SetId set_id = 0; set_id < instance_.number_of_sets(); ++set_id)
        instance_.sets_[set_id].component = -1;

    for (ComponentId component_id = 0;; ++component_id) {
        ElementId element_id = 0;
        while (element_id < instance_.number_of_elements()
                && (instance_.element(element_id).component != -1))
            element_id++;
        if (element_id == instance_.number_of_elements())
            break;
        instance_.components_.push_back(Component());
        std::vector<ElementId> stack {element_id};
        instance_.elements_[element_id].component = component_id;
        while (!stack.empty()) {
            element_id = stack.back();
            stack.pop_back();
            for (SetId set_id: instance_.element(element_id).sets) {
                if (instance_.set(set_id).component != -1)
                    continue;
                instance_.sets_[set_id].component = component_id;
                for (ElementId element_id_next: instance_.set(set_id).elements) {
                    if (instance_.element(element_id_next).component != -1)
                        continue;
                    instance_.elements_[element_id_next].component = component_id;
                    stack.push_back(element_id_next);
                }
            }
        }
    }

    for (ElementId element_id = 0;
            element_id < instance_.number_of_elements();
            ++element_id) {
        instance_.components_[instance_.element(element_id).component].elements.push_back(element_id);
    }
    for (SetId set_id = 0; set_id < instance_.number_of_sets(); ++set_id) {
        if (instance_.set(set_id).component != -1)
            instance_.components_[instance_.set(set_id).component].sets.push_back(set_id);
    }
}

Instance InstanceBuilder::build()
{
    compute_total_cost();
    compute_number_of_arcs();
    compute_components();
    return std::move(instance_);
}
