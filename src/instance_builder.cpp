#include "setcoveringsolver/instance_builder.hpp"

#include "optimizationtools/utils/utils.hpp"

#include <fstream>

using namespace setcoveringsolver;

void InstanceBuilder::move(
        Instance&& instance,
        SetId new_number_of_sets,
        ElementId new_number_of_elements)
{
    instance_ = std::move(instance);
    for (SetId set_id = 0;
            set_id < instance_.number_of_sets();
            ++set_id) {
        instance_.sets_[set_id].elements.clear();
        instance_.sets_[set_id].cost = 1;
        instance_.sets_[set_id].component = -1;
    }
    instance_.sets_.resize(new_number_of_sets);
    for (ElementId element_id = 0;
            element_id < instance_.number_of_elements();
            ++element_id) {
        instance_.elements_[element_id].sets.clear();
        instance_.elements_[element_id].component = -1;
    }
    instance_.elements_.resize(new_number_of_elements);
    instance_.total_cost_ = 0;
    instance_.number_of_arcs_ = 0;
    instance_.components_.clear();
    instance_.element_neighbors_.clear();
    instance_.set_neighbors_.clear();
}

void InstanceBuilder::add_sets(SetId number_of_sets)
{
    instance_.sets_.insert(instance_.sets_.end(), number_of_sets, Set());
}

void InstanceBuilder::add_elements(ElementId number_of_elements)
{
    instance_.elements_.insert(
            instance_.elements_.end(),
            number_of_elements,
            Element());
}

void InstanceBuilder::set_cost(
        SetId set_id,
        Cost cost)
{
    instance_.sets_[set_id].cost = cost;
}

void InstanceBuilder::set_unicost()
{
    for (SetId set_id = 0; set_id < instance_.number_of_sets(); ++set_id)
        set_cost(set_id, 1);
}

namespace
{

/**
 * Check that a set index read in an instance file belongs to
 * [first_set_id, first_set_id + number_of_sets - 1].
 */
void check_set_index(
        SetId set_id,
        SetId first_set_id,
        SetId number_of_sets)
{
    if (set_id < first_set_id || set_id >= first_set_id + number_of_sets) {
        throw std::runtime_error(
                "invalid set index " + std::to_string(set_id)
                + "; set indices should belong to ["
                + std::to_string(first_set_id) + ", "
                + std::to_string(first_set_id + number_of_sets - 1) + "].");
    }
}

/**
 * Check that an element index read in an instance file belongs to
 * [first_element_id, first_element_id + number_of_elements - 1].
 */
void check_element_index(
        ElementId element_id,
        ElementId first_element_id,
        ElementId number_of_elements)
{
    if (element_id < first_element_id
            || element_id >= first_element_id + number_of_elements) {
        throw std::runtime_error(
                "invalid element index " + std::to_string(element_id)
                + "; element indices should belong to ["
                + std::to_string(first_element_id) + ", "
                + std::to_string(first_element_id + number_of_elements - 1) + "].");
    }
}

/**
 * Check that a vertex index read in a graph instance file belongs to
 * [first_vertex_id, first_vertex_id + number_of_vertices - 1].
 *
 * In graph formats, vertices are sets.
 */
void check_vertex_index(
        SetId vertex_id,
        SetId first_vertex_id,
        SetId number_of_vertices)
{
    if (vertex_id < first_vertex_id
            || vertex_id >= first_vertex_id + number_of_vertices) {
        throw std::runtime_error(
                "invalid vertex index " + std::to_string(vertex_id)
                + "; vertex indices should belong to ["
                + std::to_string(first_vertex_id) + ", "
                + std::to_string(first_vertex_id + number_of_vertices - 1) + "].");
    }
}

/**
 * Check that the number of elements read in an instance file doesn't exceed
 * the number declared in its header.
 */
void check_number_of_elements(
        ElementId number_of_elements,
        ElementId declared_number_of_elements)
{
    if (number_of_elements > declared_number_of_elements) {
        throw std::runtime_error(
                "the file contains more elements than the "
                + std::to_string(declared_number_of_elements)
                + " declared in its header.");
    }
}

/**
 * Check that the number of edges read in a graph instance file doesn't exceed
 * the number declared in its header.
 *
 * In graph formats where edges are elements, edges are counted as elements.
 */
void check_number_of_edges(
        ElementId number_of_edges,
        ElementId declared_number_of_edges)
{
    if (number_of_edges > declared_number_of_edges) {
        throw std::runtime_error(
                "the file contains more edges than the "
                + std::to_string(declared_number_of_edges)
                + " declared in its header.");
    }
}

/**
 * Check that the number of vertex lines read in a graph instance file doesn't
 * exceed the number of vertices declared in its header.
 */
void check_number_of_vertex_lines(
        SetId number_of_vertex_lines,
        SetId declared_number_of_vertices)
{
    if (number_of_vertex_lines > declared_number_of_vertices) {
        throw std::runtime_error(
                "the file contains more vertex lines than the "
                + std::to_string(declared_number_of_vertices)
                + " declared in its header.");
    }
}

/**
 * Read a value from an instance file, and check that it has been read.
 *
 * 'name' describes the value in the error message.
 */
template <typename T>
void read_value(
        std::ifstream& file,
        T& value,
        const char* name)
{
    if (!(file >> value)) {
        throw std::runtime_error(
                std::string("unable to read ") + name
                + "; the file is truncated or contains an invalid value.");
    }
}

/**
 * Read a value from an instance file, and check that it has been read.
 *
 * 'name' followed by 'id' describes the value in the error message. The
 * message is only built if the value can't be read.
 */
template <typename T, typename Id>
void read_value(
        std::ifstream& file,
        T& value,
        const char* name,
        Id id)
{
    if (!(file >> value)) {
        throw std::runtime_error(
                std::string("unable to read ") + name + " " + std::to_string(id)
                + "; the file is truncated or contains an invalid value.");
    }
}

/** Check that the number of sets declared in the header is not negative. */
void check_declared_number_of_sets(
        SetId number_of_sets)
{
    if (number_of_sets < 0) {
        throw std::runtime_error(
                "invalid number of sets " + std::to_string(number_of_sets) + ".");
    }
}

/** Check that the number of elements declared in the header is not negative. */
void check_declared_number_of_elements(
        ElementId number_of_elements)
{
    if (number_of_elements < 0) {
        throw std::runtime_error(
                "invalid number of elements " + std::to_string(number_of_elements) + ".");
    }
}

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

    try {
        read(instance_path, format, file);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(
                "Invalid instance file \"" + instance_path + "\": " + e.what());
    }
}

void InstanceBuilder::read(
        const std::string& instance_path,
        const std::string& format,
        std::ifstream& file)
{
    if (format == "gecco2020" || format == "gecco") {
        read_geccod2020(file);
    } else if (format == "fulkerson1974" || format == "sts") {
        read_fulkerson1974(file);
    } else if (format == "balas1980" || format == "orlibrary") {
        read_balas1980(file);
    } else if (format == "balas1996") {
        read_balas1996(file);
    } else if (format == "faster1994"
            || format == "faster"
            || format == "wedelin1995"
            || format == "wedelin") {
        read_faster1994(file);
    } else if (format == "pace2019_vc") {
        FILE* file = fopen(instance_path.c_str(), "r");
        read_pace2019_vc(file);
    } else if (format == "pace2025") {
        FILE* file = fopen(instance_path.c_str(), "r");
        read_pace2025(file);
    } else if (format == "pace2025_ds") {
        FILE* file = fopen(instance_path.c_str(), "r");
        read_pace2025_ds(file);
    } else if (format == "dimacs2010_vc") {
        FILE* file = fopen(instance_path.c_str(), "r");
        read_dimacs2010_vc(file);
    } else {
        throw std::invalid_argument(
                "Unknown instance format \"" + format + "\".");
    }
}

void InstanceBuilder::read_geccod2020(std::ifstream& file)
{
    ElementId number_of_elements;
    SetId number_of_sets;
    read_value(file, number_of_elements, "the number of elements");
    read_value(file, number_of_sets, "the number of sets");
    check_declared_number_of_elements(number_of_elements);
    check_declared_number_of_sets(number_of_sets);

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
        read_value(file, element_id_tmp, "the index of element", element_id);
        read_value(file, element_number_of_sets, "the number of sets covering element", element_id);
        for (SetPos set_pos = 0; set_pos < element_number_of_sets; ++set_pos) {
            read_value(file, set_id, "a set covering element", element_id);
            check_set_index(set_id, 0, number_of_sets);
            add_arc(set_id, element_id);
        }
    }
}

void InstanceBuilder::read_fulkerson1974(std::ifstream& file)
{
    SetId number_of_sets;
    ElementId number_of_elements;
    read_value(file, number_of_sets, "the number of sets");
    read_value(file, number_of_elements, "the number of elements");
    check_declared_number_of_sets(number_of_sets);
    check_declared_number_of_elements(number_of_elements);

    add_elements(number_of_elements);
    add_sets(number_of_sets);

    for (SetId set_id = 0; set_id < number_of_sets; ++set_id)
        set_cost(set_id, 1);

    SetId set_id;
    for (ElementId element_id = 0;
            element_id < number_of_elements;
            ++element_id) {
        for (SetPos set_pos = 0; set_pos < 3; ++set_pos) {
            read_value(file, set_id, "a set covering element", element_id + 1);
            check_set_index(set_id, 1, number_of_sets);
            add_arc(set_id - 1, element_id);
        }
    }
}

void InstanceBuilder::read_balas1980(std::ifstream& file)
{
    ElementId number_of_elements;
    SetId number_of_sets;
    read_value(file, number_of_elements, "the number of elements");
    read_value(file, number_of_sets, "the number of sets");
    check_declared_number_of_elements(number_of_elements);
    check_declared_number_of_sets(number_of_sets);

    add_elements(number_of_elements);
    add_sets(number_of_sets);

    Cost cost;
    for (SetId set_id = 0; set_id < number_of_sets; ++set_id) {
        read_value(file, cost, "the cost of set", set_id + 1);
        set_cost(set_id, cost);
    }

    SetId set_id;
    SetId element_number_of_sets;
    for (ElementId element_id = 0;
            element_id < number_of_elements;
            ++element_id) {
        read_value(file, element_number_of_sets, "the number of sets covering element", element_id + 1);
        for (SetPos set_pos = 0; set_pos < element_number_of_sets; ++set_pos) {
            read_value(file, set_id, "a set covering element", element_id + 1);
            check_set_index(set_id, 1, number_of_sets);
            add_arc(set_id - 1, element_id);
        }
    }
}

void InstanceBuilder::read_balas1996(std::ifstream& file)
{
    SetId number_of_sets;
    ElementId number_of_elements;
    read_value(file, number_of_sets, "the number of sets");
    read_value(file, number_of_elements, "the number of elements");
    check_declared_number_of_sets(number_of_sets);
    check_declared_number_of_elements(number_of_elements);

    add_elements(number_of_elements);
    add_sets(number_of_sets);

    Cost cost;
    for (SetId set_id = 0; set_id < number_of_sets; ++set_id) {
        read_value(file, cost, "the cost of set", set_id + 1);
        set_cost(set_id, cost);
    }

    ElementId element_id;
    ElementId set_number_of_elements;
    for (SetId set_id = 0; set_id < number_of_sets; ++set_id) {
        read_value(file, set_number_of_elements, "the number of elements covered by set", set_id + 1);
        for (ElementPos element_pos = 0;
                element_pos < set_number_of_elements;
                ++element_pos) {
            read_value(file, element_id, "an element covered by set", set_id + 1);
            check_element_index(element_id, 1, number_of_elements);
            add_arc(set_id, element_id - 1);
        }
    }
}

void InstanceBuilder::read_faster1994(std::ifstream& file)
{
    ElementId number_of_elements;
    SetId number_of_sets;
    read_value(file, number_of_elements, "the number of elements");
    read_value(file, number_of_sets, "the number of sets");
    check_declared_number_of_elements(number_of_elements);
    check_declared_number_of_sets(number_of_sets);

    add_elements(number_of_elements);
    add_sets(number_of_sets);

    Cost cost;
    ElementId element_id;
    ElementId set_number_of_elements;
    for (SetId set_id = 0; set_id < number_of_sets; ++set_id) {
        read_value(file, cost, "the cost of set", set_id + 1);
        read_value(file, set_number_of_elements, "the number of elements covered by set", set_id + 1);
        set_cost(set_id, cost);
        for (ElementPos element_pos = 0;
                element_pos < set_number_of_elements;
                ++element_pos) {
            read_value(file, element_id, "an element covered by set", set_id + 1);
            check_element_index(element_id, 1, number_of_elements);
            add_arc(set_id, element_id - 1);
        }
    }
}

inline char peek_char(
        FILE* file,
        char* buf,
        size_t BUF_SIZE,
        size_t& buf_pos,
        size_t& buf_len)
{
    if (buf_pos >= buf_len) {
        buf_len = fread(buf, 1, BUF_SIZE, file);
        buf_pos = 0;
        if (buf_len == 0) return EOF;
    }
    return buf[buf_pos];
}

inline char get_char(
        FILE* file,
        char* buf,
        size_t BUF_SIZE,
        size_t& buf_pos,
        size_t& buf_len)
{
    if (buf_pos >= buf_len) {
        buf_len = fread(buf, 1, BUF_SIZE, file);
        buf_pos = 0;
        if (buf_len == 0) return EOF;
    }
    return buf[buf_pos++];
}

inline void skip_line(
        FILE* file,
        char* buf,
        size_t BUF_SIZE,
        size_t& buf_pos,
        size_t& buf_len)
{
    char c;
    while ((c = get_char(file, buf, BUF_SIZE, buf_pos, buf_len)) != '\n' && c != EOF) {}
}

inline bool read_next_int_on_line(
        FILE* file,
        char* buf,
        size_t BUF_SIZE,
        size_t& buf_pos,
        size_t& buf_len,
        int64_t &x)
{
    x = 0;
    char c;

    // Skip non-digit characters
    do {
        c = get_char(file, buf, BUF_SIZE, buf_pos, buf_len);
        if (c == '\n' || c == EOF) return false;
    } while (c < '0' || c > '9');

    // Parse integer
    do {
        x = x * 10 + (c - '0');
        c = get_char(file, buf, BUF_SIZE, buf_pos, buf_len);
    } while (c >= '0' && c <= '9');

    // If we hit newline, we’ve reached the end of the line
    if (c == '\n') buf_pos--; // allow outer loop to see the newline
    return true;
}

void InstanceBuilder::read_pace2019_vc(FILE* file)
{
    const size_t BUF_SIZE = 1 << 21;
    char buf[BUF_SIZE];
    size_t buf_pos = 0;
    size_t buf_len = 0;

    // Skip lines until we find the problem line: p ds <vertices> <edges>
    char c;
    SetId set_id_1 = -1;
    SetId set_id_2 = -1;
    ElementId element_id = 0;
    bool header_read = false;
    while ((c = peek_char(file, buf, BUF_SIZE, buf_pos, buf_len)) != EOF) {
        if (c == 'c') {
            skip_line(file, buf, BUF_SIZE, buf_pos, buf_len);
        } else if (c == 'p') {
            SetId number_of_vertices = -1;
            SetId number_of_edges = -1;
            read_next_int_on_line(file, buf, BUF_SIZE, buf_pos, buf_len, number_of_vertices);
            read_next_int_on_line(file, buf, BUF_SIZE, buf_pos, buf_len, number_of_edges);
            add_elements(number_of_edges);
            add_sets(number_of_vertices);
            skip_line(file, buf, BUF_SIZE, buf_pos, buf_len);
            header_read = true;
        } else if (c == '\n') {
            skip_line(file, buf, BUF_SIZE, buf_pos, buf_len);
        } else {
            if (!header_read)
                throw std::runtime_error("missing problem line 'p'.");
            if (!read_next_int_on_line(file, buf, BUF_SIZE, buf_pos, buf_len, set_id_1)
                    || !read_next_int_on_line(file, buf, BUF_SIZE, buf_pos, buf_len, set_id_2)) {
                throw std::runtime_error(
                        "edge " + std::to_string(element_id + 1)
                        + " doesn't have two vertices.");
            }
            skip_line(file, buf, BUF_SIZE, buf_pos, buf_len);
            check_number_of_edges(element_id + 1, instance_.number_of_elements());
            check_vertex_index(set_id_1, 1, instance_.number_of_sets());
            check_vertex_index(set_id_2, 1, instance_.number_of_sets());
            add_arc(set_id_1 - 1, element_id);
            add_arc(set_id_2 - 1, element_id);
            element_id++;
        }
    }
}

void InstanceBuilder::read_pace2025(FILE* file)
{
    const size_t BUF_SIZE = 1 << 21;
    char buf[BUF_SIZE];
    size_t buf_pos = 0;
    size_t buf_len = 0;

    // Skip lines until we find the problem line: p ds <vertices> <edges>
    char c;
    SetId set_id = -1;
    ElementId element_id = 0;
    bool header_read = false;
    while ((c = peek_char(file, buf, BUF_SIZE, buf_pos, buf_len)) != EOF) {
        if (c == 'c') {
            skip_line(file, buf, BUF_SIZE, buf_pos, buf_len);
        } else if (c == 'p') {
            SetId number_of_sets = -1;
            SetId number_of_elements = -1;
            read_next_int_on_line(file, buf, BUF_SIZE, buf_pos, buf_len, number_of_sets);
            read_next_int_on_line(file, buf, BUF_SIZE, buf_pos, buf_len, number_of_elements);
            add_elements(number_of_elements);
            add_sets(number_of_sets);
            skip_line(file, buf, BUF_SIZE, buf_pos, buf_len);
            header_read = true;
        } else {
            // Lines without any set (e.g. empty lines) are ignored.
            bool empty = true;
            while (read_next_int_on_line(file, buf, BUF_SIZE, buf_pos, buf_len, set_id)) {
                if (!header_read)
                    throw std::runtime_error("missing problem line 'p'.");
                if (empty)
                    check_number_of_elements(element_id + 1, instance_.number_of_elements());
                empty = false;
                check_set_index(set_id, 1, instance_.number_of_sets());
                add_arc(set_id - 1, element_id);
            }
            if (!empty)
                element_id++;
        }
    }
}

void InstanceBuilder::read_pace2025_ds(FILE* file)
{
    const size_t BUF_SIZE = 1 << 21;
    char buf[BUF_SIZE];
    size_t buf_pos = 0;
    size_t buf_len = 0;

    // Skip lines until we find the problem line: p ds <vertices> <edges>
    char c;
    SetId set_id_1 = -1;
    SetId set_id_2 = -1;
    ElementId edge_id = 0;
    bool header_read = false;
    while ((c = peek_char(file, buf, BUF_SIZE, buf_pos, buf_len)) != EOF) {
        if (c == 'c') {
            skip_line(file, buf, BUF_SIZE, buf_pos, buf_len);
        } else if (c == 'p') {
            SetId number_of_vertices = -1;
            SetId number_of_edges = -1;
            read_next_int_on_line(file, buf, BUF_SIZE, buf_pos, buf_len, number_of_vertices);
            read_next_int_on_line(file, buf, BUF_SIZE, buf_pos, buf_len, number_of_edges);
            add_elements(number_of_vertices);
            add_sets(number_of_vertices);
            for (SetId set_id = 0; set_id < number_of_vertices; ++set_id)
                add_arc(set_id, set_id);
            skip_line(file, buf, BUF_SIZE, buf_pos, buf_len);
            header_read = true;
        } else if (c == '\n') {
            skip_line(file, buf, BUF_SIZE, buf_pos, buf_len);
        } else {
            if (!header_read)
                throw std::runtime_error("missing problem line 'p'.");
            if (!read_next_int_on_line(file, buf, BUF_SIZE, buf_pos, buf_len, set_id_1)
                    || !read_next_int_on_line(file, buf, BUF_SIZE, buf_pos, buf_len, set_id_2)) {
                throw std::runtime_error(
                        "edge " + std::to_string(edge_id + 1)
                        + " doesn't have two vertices.");
            }
            skip_line(file, buf, BUF_SIZE, buf_pos, buf_len);
            check_vertex_index(set_id_1, 1, instance_.number_of_sets());
            check_vertex_index(set_id_2, 1, instance_.number_of_sets());
            add_arc(set_id_1 - 1, set_id_2 - 1);
            add_arc(set_id_2 - 1, set_id_1 - 1);
            edge_id++;
        }
    }
}

void InstanceBuilder::read_dimacs2010_vc(FILE* file)
{
    const size_t BUF_SIZE = 1 << 21;
    char buf[BUF_SIZE];
    size_t buf_pos = 0;
    size_t buf_len = 0;

    // Skip lines until we find the problem line: p ds <vertices> <edges>
    char c;
    SetId set_id_1 = 1;
    SetId set_id_2 = -1;
    ElementId element_id = 0;
    bool first = true;
    while ((c = peek_char(file, buf, BUF_SIZE, buf_pos, buf_len)) != EOF) {
        if (first) {
            first = false;
            SetId number_of_sets = -1;
            ElementId number_of_elements = -1;
            read_next_int_on_line(file, buf, BUF_SIZE, buf_pos, buf_len, number_of_sets);
            read_next_int_on_line(file, buf, BUF_SIZE, buf_pos, buf_len, number_of_elements);
            add_elements(number_of_elements);
            add_sets(number_of_sets);
            // Skip the rest of the line (optional format field).
            // 'read_next_int_on_line' returns 'false' once it has read the
            // end of the line.
            SetId tmp;
            while (read_next_int_on_line(file, buf, BUF_SIZE, buf_pos, buf_len, tmp)) {}
        } else {
            while (read_next_int_on_line(file, buf, BUF_SIZE, buf_pos, buf_len, set_id_2)) {
                check_number_of_vertex_lines(set_id_1, instance_.number_of_sets());
                check_vertex_index(set_id_2, 1, instance_.number_of_sets());
                if (set_id_2 > set_id_1) {
                    check_number_of_edges(element_id + 1, instance_.number_of_elements());
                    add_arc(set_id_1 - 1, element_id);
                    add_arc(set_id_2 - 1, element_id);
                    element_id++;
                }
            }
            set_id_1++;
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
    if (!instance_.components_.empty())
        return;
    for (ElementId element_id = 0;
            element_id < instance_.number_of_elements();
            ++element_id)
        instance_.elements_[element_id].component = -1;
    for (SetId set_id = 0; set_id < instance_.number_of_sets(); ++set_id)
        instance_.sets_[set_id].component = -1;

    ElementId element_id_0 = 0;
    for (ComponentId component_id = 0;; ++component_id) {
        while (element_id_0 < instance_.number_of_elements()
                && (instance_.element(element_id_0).component != -1))
            element_id_0++;
        if (element_id_0 == instance_.number_of_elements())
            break;
        instance_.components_.push_back(Component());
        std::vector<ElementId> stack {element_id_0};
        instance_.elements_[element_id_0].component = component_id;
        while (!stack.empty()) {
            ElementId element_id = stack.back();
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

    for (ElementId element_id = 0;
            element_id < instance_.number_of_elements();
            ++element_id) {
        const Element& element = instance_.element(element_id);
        if (element.sets.empty()) {
            throw std::logic_error(
                    "setcoveringsolver::InstanceBuilder::build: "
                    "uncoverable element; "
                    "element_id: " + std::to_string(element_id) + ".");
        }
    }

    return std::move(instance_);
}
