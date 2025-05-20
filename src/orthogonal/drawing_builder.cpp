#include "drawing_builder.hpp"

#include <list>
#include <math.h>
#include <ranges>
#include <functional>
#include <unordered_map>
#include <unordered_set>

class EquivalenceClasses {
private:
    std::unordered_map<int,int> m_elem_to_class;
    std::unordered_map<int,std::vector<int>> m_class_to_elems;
public:
    void set_class(int elem, int class_id) {
        if (has_elem_a_class(elem))
            throw std::runtime_error(
                "EquivalenceClasses::set_class elem already has an assigned class"
            );
        m_elem_to_class[elem] = class_id;
        if (!m_class_to_elems.contains(class_id))
            m_class_to_elems[class_id] = std::vector<int>();
        m_class_to_elems[class_id].push_back(elem);
    }
    bool has_elem_a_class(int elem) const {
        return m_elem_to_class.contains(elem);
    }
    bool has_class(int class_id) const {
        return m_class_to_elems.contains(class_id);
    }
    int get_class_of_elem(int elem) const  {
        if (!has_elem_a_class(elem))
            throw std::runtime_error(
                "EquivalenceClasses::get_class elem does not have a class"
            );
        return m_elem_to_class.at(elem);
    }
    const auto& get_elems_of_class(int class_id) const {
        if (!has_class(class_id))
            throw std::runtime_error(
                "EquivalenceClasses::get_elems class does not exist"
            );
        return m_class_to_elems.at(class_id);
    }
    std::string to_string() const {
        std::string result = "EquivalenceClasses:\n";
        for (int i = 0; i < m_elem_to_class.size(); ++i)
            result += std::to_string(i) + " -> " + std::to_string(m_elem_to_class.at(i)) + "\n";
        return result;
    }
    void print() const { std::cout << to_string() << std::endl; }
    auto get_all_classes() const {
        return m_class_to_elems | std::views::transform([](const auto& pair) -> int {
            return pair.first;
        });
    }
};

void horizontal_edge_expander(
    const Shape& shape,
    const Graph& graph,
    int left, int right,
    int class_id,
    IsEdgeVisited& is_edge_visited,
    EquivalenceClasses& equivalence_classes_y
) {
    is_edge_visited.set_visited(left, right);
    is_edge_visited.set_visited(right, left);
    std::unordered_set<int> visited;
    visited.insert(left);
    visited.insert(right);
    std::list<int> eq_class;
    eq_class.push_back(left);
    eq_class.push_back(right);
    while (shape.has_node_a_left_neighbor(left)) {
        int new_left = shape.get_left_neighbor(left);
        is_edge_visited.set_visited(left, new_left);
        is_edge_visited.set_visited(new_left, left);
        left = new_left;
        if (visited.contains(left)) break;
        visited.insert(left);
        eq_class.push_front(left);
    }
    while (shape.has_node_a_right_neighbor(right)) {
        int new_right = shape.get_right_neighbor(right);
        is_edge_visited.set_visited(right, new_right);
        is_edge_visited.set_visited(new_right, right);
        right = new_right;
        if (visited.contains(right)) break;
        visited.insert(right);
        eq_class.push_back(right);
    }
    for (int node : eq_class)
        equivalence_classes_y.set_class(node, class_id);
}

void vertical_edge_expander(
    const Shape& shape,
    const Graph& graph,
    int down, int up,
    int class_id,
    IsEdgeVisited& is_edge_visited,
    EquivalenceClasses& equivalence_classes_x
) {
    is_edge_visited.set_visited(down, up);
    is_edge_visited.set_visited(up, down);
    std::unordered_set<int> visited;
    visited.insert(down);
    visited.insert(up);
    std::list<int> eq_class;
    eq_class.push_back(down);
    eq_class.push_back(up);
    while (shape.has_node_a_down_neighbor(down)) {
        int new_down = shape.get_down_neighbor(down);
        is_edge_visited.set_visited(down, new_down);
        is_edge_visited.set_visited(new_down, down);
        down = new_down;
        if (visited.contains(down)) break;
        visited.insert(down);
        eq_class.push_front(down);
    }
    while (shape.has_node_a_up_neighbor(up)) {
        int new_up = shape.get_up_neighbor(up);
        is_edge_visited.set_visited(up, new_up);
        is_edge_visited.set_visited(new_up, up);
        up = new_up;
        if (visited.contains(up)) break;
        visited.insert(up);
        eq_class.push_back(up);
    }
    for (int node : eq_class)
        equivalence_classes_x.set_class(node, class_id);
}

const auto build_equivalence_classes(
    const Shape& shape,
    const Graph& graph
) {
    EquivalenceClasses equivalence_classes_x;
    EquivalenceClasses equivalence_classes_y;
    int next_class_x = 0;
    int next_class_y = 0;
    IsEdgeVisited is_edge_visited;
    for (auto& node : graph.get_nodes()) {
        int i = node.get_id();
        for (auto& edge : node.get_edges()) {
            int j = edge.get_to().get_id();
            if (is_edge_visited.is_visited(i, j)) continue;
            if (shape.is_horizontal(i, j)) {
                int left = i;
                int right = j;
                if (shape.is_left(i, j)) {
                    left = j;
                    right = i;
                }
                horizontal_edge_expander(
                    shape, graph, left, right, next_class_y++,
                    is_edge_visited, equivalence_classes_y
                );
            } else {
                int down = i;
                int up = j;
                if (shape.is_down(i, j)) {
                    down = j;
                    up = i;
                }
                vertical_edge_expander(
                    shape, graph, down, up, next_class_x++,
                    is_edge_visited, equivalence_classes_x
                );
            }
        }
    }
    for (const auto& node : graph.get_nodes()) {
        if (!equivalence_classes_x.has_elem_a_class(node.get_id()))
            equivalence_classes_x.set_class(node.get_id(), next_class_x++);
        if (!equivalence_classes_y.has_elem_a_class(node.get_id()))
            equivalence_classes_y.set_class(node.get_id(), next_class_y++);
    }
    return std::make_pair(
        std::move(equivalence_classes_x), std::move(equivalence_classes_y)
    );
}

auto equivalence_classes_to_ordering(
    const EquivalenceClasses& equivalence_classes_x,
    const EquivalenceClasses& equivalence_classes_y,
    const Graph& graph,
    const Shape& shape
) {
    auto ordering_x = std::make_unique<Graph>();
    auto ordering_y = std::make_unique<Graph>();
    for (int class_id : equivalence_classes_x.get_all_classes())
        ordering_x->add_node(class_id);
    for (int class_id : equivalence_classes_y.get_all_classes())
        ordering_y->add_node(class_id);
    GraphAttributes ordering_x_edge_to_graph_edge;
    GraphAttributes ordering_y_edge_to_graph_edge;
    ordering_x_edge_to_graph_edge.add_attribute(Attribute::EDGES_ANY_LABEL);
    ordering_y_edge_to_graph_edge.add_attribute(Attribute::EDGES_ANY_LABEL);
    for (auto& node : graph.get_nodes()) {
        int i = node.get_id();
        for (auto& edge : node.get_edges()) {
            int j = edge.get_to().get_id();
            if (shape.is_right(i, j)) {
                int node_class_x = equivalence_classes_x.get_class_of_elem(i);
                int neighbor_class_x = equivalence_classes_x.get_class_of_elem(j);
                if (ordering_x->has_edge(node_class_x, neighbor_class_x))
                    continue;
                if (node_class_x == neighbor_class_x)
                    continue;
                auto& e = ordering_x->add_edge(node_class_x, neighbor_class_x);
                ordering_x_edge_to_graph_edge.set_edge_any_label(e.get_id(), std::make_pair(i, j));
            } else if (shape.is_up(i, j)) {
                int node_class_y = equivalence_classes_y.get_class_of_elem(i);
                int neighbor_class_y = equivalence_classes_y.get_class_of_elem(j);
                if (ordering_y->has_edge(node_class_y, neighbor_class_y))
                    continue;
                if (node_class_y == neighbor_class_y)
                    continue;
                auto& e = ordering_y->add_edge(node_class_y, neighbor_class_y);
                ordering_y_edge_to_graph_edge.set_edge_any_label(e.get_id(), std::make_pair(i, j));
            }
        }
    }
    return std::make_tuple(
        std::move(ordering_x),
        std::move(ordering_y),
        std::move(ordering_x_edge_to_graph_edge),
        std::move(ordering_y_edge_to_graph_edge)
    );
}

void NodesPositions::set_position(int node, int position_x, int position_y) {
    if (has_position(node))
        throw std::runtime_error("NodesPositions::set_position_x Node already has a position");
    m_nodeid_to_position_map[node] = NodePosition{position_x, position_y};
}

int NodesPositions::get_position_x(int node) const {
    if (!has_position(node))
        throw std::runtime_error("NodesPositions::get_position_x Node does not have a position");
    return m_nodeid_to_position_map.at(node).m_x;
}

int NodesPositions::get_position_y(int node) const {
    if (!has_position(node))
        throw std::runtime_error("NodesPositions::get_position_y Node does not have a position");
    return m_nodeid_to_position_map.at(node).m_y;
}

const NodePosition& NodesPositions::get_position(int node) const {
    if (!has_position(node))
        throw std::runtime_error("NodesPositions::get_position Node does not have a position");
    return m_nodeid_to_position_map.at(node);
}

bool NodesPositions::has_position(int node) const {
    return m_nodeid_to_position_map.contains(node);
}

void NodesPositions::remove_position(int node) {
    if (!has_position(node))
        throw std::runtime_error("NodesPositions::remove_position Node does not have a position");
    m_nodeid_to_position_map.erase(node);
}

std::vector<int> path_in_class(int from, int to, const std::vector<int>& class_elems) {
    std::vector<int> path;
    int from_pos = -1;
    int to_pos = -1;
    for (int i = 0; i < class_elems.size(); ++i) {
        if (class_elems[i] == from) from_pos = i;
        if (class_elems[i] == to) to_pos = i;
    }
    if (from_pos > to_pos) {
        auto path = path_in_class(to, from, class_elems);
        std::reverse(path.begin(), path.end());
        return path;
    }
    for (int i = from_pos; i <= to_pos; ++i)
        path.push_back(class_elems[i]);
    return path;
}

std::vector<int> build_cycle_in_graph_from_cycle_in_ordering(
    const std::vector<int>& cycle_in_ordering,
    const Graph& ordering,
    const EquivalenceClasses& equivalence_classes,
    const GraphAttributes& ordering_edge_to_graph_edge
) {
    std::vector<int> cycle;
    for (int i = 0; i < cycle_in_ordering.size(); ++i) {
        int class_id = cycle_in_ordering[i];
        int next_class_id = cycle_in_ordering[(i+1)%cycle_in_ordering.size()];
        auto& edge = ordering.get_edge(class_id, next_class_id);
        const std::any& edge_label = ordering_edge_to_graph_edge.get_edge_any_label(edge.get_id());
        int from = std::any_cast<std::pair<int,int>>(edge_label).first;
        int to = std::any_cast<std::pair<int,int>>(edge_label).second;
        cycle.push_back(from);
        int next_next_class_id = cycle_in_ordering[(i+2)%cycle_in_ordering.size()];
        auto& next_edge = ordering.get_edge(next_class_id, next_next_class_id);
        const std::any& next_edge_label = ordering_edge_to_graph_edge.get_edge_any_label(next_edge.get_id());
        int next_from = std::any_cast<std::pair<int,int>>(next_edge_label).first;
        if (to != next_from) {
            auto path = path_in_class(to, next_from, equivalence_classes.get_elems_of_class(next_class_id));
            for (int i = 0; i < path.size()-1; ++i)
                cycle.push_back(path[i]);
        }
    }
    return cycle;
}

BuildingResult build_nodes_positions(
    const Shape& shape,
    const Graph& graph
) {
    auto classes = build_equivalence_classes(shape, graph);
    auto& classes_x = classes.first;
    auto& classes_y = classes.second;
    auto ordering = equivalence_classes_to_ordering(classes_x, classes_y, graph, shape);
    auto& ordering_x = std::get<0>(ordering);
    auto& ordering_y = std::get<1>(ordering);
    auto& ordering_x_edge_to_graph_edge = std::get<2>(ordering);
    auto& ordering_y_edge_to_graph_edge = std::get<3>(ordering);
    auto cycle_x = find_a_cycle_directed_graph(*ordering_x);
    auto cycle_y = find_a_cycle_directed_graph(*ordering_y);
    if (cycle_x.has_value() || cycle_y.has_value()) {
        std::vector<std::vector<int>> cycles_to_be_added;
        if (cycle_x.has_value()) {
            auto cycle_x_in_original_graph = build_cycle_in_graph_from_cycle_in_ordering(
                cycle_x.value(), *ordering_x, classes_x, ordering_x_edge_to_graph_edge
            );
            cycles_to_be_added.push_back(cycle_x_in_original_graph);
        }
        if (cycle_y.has_value()) {
            auto cycle_y_in_original_graph = build_cycle_in_graph_from_cycle_in_ordering(
                cycle_y.value(), *ordering_y, classes_y, ordering_y_edge_to_graph_edge
            );
            cycles_to_be_added.push_back(cycle_y_in_original_graph);
        }
        return BuildingResult{std::nullopt, cycles_to_be_added, BuildingResultType::CYCLES_TO_BE_ADDED};
    }
    auto classes_x_ordering = make_topological_ordering(*ordering_x);
    auto classes_y_ordering = make_topological_ordering(*ordering_y);
    int current_position_x = 0;
    std::unordered_map<int, int> node_id_to_position_x;
    for (auto& class_id : classes_x_ordering) {
        for (auto& node : classes_x.get_elems_of_class(class_id))
            node_id_to_position_x[node] = current_position_x;
        ++current_position_x;
    }
    int current_position_y = 0;
    std::unordered_map<int, int> node_id_to_position_y;
    for (auto& class_id : classes_y_ordering) {
        for (auto& node : classes_y.get_elems_of_class(class_id))
            node_id_to_position_y[node] = current_position_y;
        ++current_position_y;
    }
    NodesPositions positions;
    for (int node_id : graph.get_nodes_ids()) {
        int x = node_id_to_position_x[node_id];
        int y = node_id_to_position_y[node_id];
        positions.set_position(node_id, x, y);
    }
    return BuildingResult{std::move(positions), {}, BuildingResultType::OK};
}

void node_positions_to_svg(
    const NodesPositions& positions,
    const Graph& graph,
    const GraphAttributes& attributes,
    const std::string& filename
) {
    int max_x = 0;
    int max_y = 0;
    for (auto& node : graph.get_nodes()) {
        max_x = std::max(max_x, positions.get_position_x(node.get_id()));
        max_y = std::max(max_y, positions.get_position_y(node.get_id()));
    }
    SvgDrawer drawer{800, 600};
    ScaleLinear scale_x = ScaleLinear(0, max_x+2, 0, 800);
    ScaleLinear scale_y = ScaleLinear(0, max_y+2, 0, 600);
    std::unordered_map<int, Point2D> points;
    for (auto& node : graph.get_nodes()) {
        double x = scale_x.map(positions.get_position_x(node.get_id())+1);
        double y = scale_y.map(positions.get_position_y(node.get_id())+1);
        points.emplace(node.get_id(), Point2D(x, y));
    }
    for (auto& node : graph.get_nodes()) {
        int i = node.get_id();
        for (auto& edge : node.get_edges()) {
            int j = edge.get_to().get_id();
            Line2D line(points.at(i), points.at(j));
            drawer.add(line);
        }
    }
    for (auto& node : graph.get_nodes()) {
        Color color = attributes.get_node_color(node.get_id());
        drawer.add(points.at(node.get_id()), color_to_string(color), std::to_string(node.get_id()));
    }
    drawer.saveToFile(filename);
}

double compute_stddev(const std::vector<int> &values) {
    double mean = 0;
    for (const auto &value : values)
        mean += value;
    mean /= values.size();
    double variance = 0;
    for (const auto &value : values)
        variance += (value - mean) * (value - mean);
    variance /= values.size();
    return std::sqrt(variance);
}

// total edge length
// max edge length
// edge length stddev
std::tuple<int, int, double> compute_edge_length_metrics(
    const NodesPositions &positions,
    const Graph &graph,
    const GraphAttributes& attributes
) {
    std::vector<int> edge_lengths;
    int total_edge_length = 0;
    int max_edge_length = 0;
    int n = graph.get_nodes().size();
    std::unordered_set<int> visited;
    for (const auto& node : graph.get_nodes()) {
        if (attributes.get_node_color(node.get_id()) != Color::BLACK)
            continue;
        std::function<void(int, int, int)> dfs = [
                &graph, &visited, &positions, &attributes, &total_edge_length, &edge_lengths, &max_edge_length, &dfs
            ] (int current_id, int black_id, int current_length) 
        {
            visited.insert(current_id);
            for (const auto &edge : graph.get_node_by_id(current_id).get_edges()) {
                int neighbor = edge.get_to().get_id();
                if (visited.contains(neighbor))
                    continue;
                int x1 = positions.get_position_x(current_id);
                int y1 = positions.get_position_y(current_id);
                int x2 = positions.get_position_x(neighbor);
                int y2 = positions.get_position_y(neighbor);
                int length = std::abs(x1 - x2) + std::abs(y1 - y2);
                Color neighbor_color = attributes.get_node_color(neighbor);
                if (neighbor_color == Color::RED)
                    dfs(neighbor, black_id, current_length + length);
                else if (neighbor_color == Color::BLACK) {
                    if (black_id < neighbor) {
                        int total_length = current_length + length;
                        total_edge_length += total_length;
                        edge_lengths.push_back(total_length);
                        max_edge_length = std::max(max_edge_length, total_length);
                    }
                }
            }
            visited.erase(current_id);
        };
        dfs(node.get_id(), node.get_id(), 0);
    }
    if (edge_lengths.empty())
        return std::make_tuple(total_edge_length, max_edge_length, 0.0);
    double stddev = compute_stddev(edge_lengths);
    return std::make_tuple(total_edge_length, max_edge_length, stddev);
}

// max bends per edge
// bends stddev
std::tuple<int, double> compute_bends_metrics(
    const Graph &graph, 
    const GraphAttributes& attributes
) {
    std::vector<int> red_counts;
    int max_reds = 0;
    for (const auto& node : graph.get_nodes()) {
        if (attributes.get_node_color(node.get_id()) != Color::BLACK)
            continue;
        std::unordered_set<int> visited;
        std::function<void(int, int, int)> dfs = [&](int current, int black, int red_count) {
            visited.insert(current);
            for (const auto &edge : graph.get_node_by_id(current).get_edges()) {
                int neighbor = edge.get_to().get_id();
                if (visited.contains(neighbor))
                    continue;
                Color neighbor_color = attributes.get_node_color(neighbor);
                if (neighbor_color == Color::RED) {
                    dfs(neighbor, black, red_count + 1);
                }
                else if (neighbor_color == Color::BLACK) {
                    if (black < neighbor) {
                        max_reds = std::max(max_reds, red_count);
                        red_counts.push_back(red_count);
                    }
                }
            }
            visited.erase(current);
        };
        dfs(node.get_id(), node.get_id(), 0);
    }
    if (red_counts.empty())
        return std::make_tuple(max_reds, 0.0);
    double stddev = compute_stddev(red_counts);
    return std::make_tuple(max_reds, stddev);
}

int compute_total_area(const NodesPositions& positions, const Graph& graph) {
    int min_x = graph.size();
    int min_y = graph.size();
    int max_x = 0;
    int max_y = 0;
    for (auto& node : graph.get_nodes()) {
        int i = node.get_id();
        min_x = std::min(min_x, positions.get_position_x(i));
        min_y = std::min(min_y, positions.get_position_y(i));
        max_x = std::max(max_x, positions.get_position_x(i));
        max_y = std::max(max_y, positions.get_position_y(i));
    }
    return (max_x - min_x + 1) * (max_y - min_y + 1);
}

bool do_edges_cross(
    const NodesPositions& positions,
    int i, int j,
    int k, int l
) {
    int i_pos_x = positions.get_position_x(i);
    int i_pos_y = positions.get_position_y(i);
    int j_pos_x = positions.get_position_x(j);
    int j_pos_y = positions.get_position_y(j);
    int k_pos_x = positions.get_position_x(k);
    int k_pos_y = positions.get_position_y(k);
    int l_pos_x = positions.get_position_x(l);
    int l_pos_y = positions.get_position_y(l);

    bool is_i_j_horizontal = i_pos_y == j_pos_y;
    bool is_k_l_horizontal = k_pos_y == l_pos_y;

    if (is_i_j_horizontal && is_k_l_horizontal) {
        return (i_pos_y == k_pos_y) && ((i_pos_x <= k_pos_x && j_pos_x >= k_pos_x) ||
                                        (i_pos_x <= l_pos_x && j_pos_x >= l_pos_x) ||
                                        (j_pos_x <= k_pos_x && i_pos_x >= k_pos_x) ||
                                        (j_pos_x <= l_pos_x && i_pos_x >= l_pos_x));
    }
    if (!is_i_j_horizontal && !is_k_l_horizontal) {
        return (i_pos_x == k_pos_x) && ((i_pos_y <= k_pos_y && j_pos_y >= k_pos_y) ||
                                        (i_pos_y <= l_pos_y && j_pos_y >= l_pos_y) ||
                                        (j_pos_y <= k_pos_y && i_pos_y >= k_pos_y) ||
                                        (j_pos_y <= l_pos_y && i_pos_y >= l_pos_y));
    }
    if (!is_i_j_horizontal)
        return do_edges_cross(positions, k, l, i, j);
    if (k_pos_x < std::min(i_pos_x, j_pos_x) || k_pos_x > std::max(i_pos_x, j_pos_x))
        return false;
    if (i_pos_y < std::min(k_pos_y, l_pos_y) || i_pos_y > std::max(k_pos_y, l_pos_y))
        return false;
    return true;
}

int compute_total_crossings(const NodesPositions& positions, const Graph& graph) {
    int total_crossings = 0;
    for (auto& edge : graph.get_edges()) {
        int edge_id = edge.get_id();
        for (auto& other_edge : graph.get_edges()) {
            int other_edge_id = other_edge.get_id();
            if (edge_id >= other_edge_id) continue;
            int i = edge.get_from().get_id();
            int j = edge.get_to().get_id();
            int k = other_edge.get_from().get_id();
            int l = other_edge.get_to().get_id();
            if (i == k || i == l || j == k || j == l)
                continue;
            if (do_edges_cross(positions, i, j, k, l))
                ++total_crossings;
        }
    }
    return total_crossings/4;
}

// removes useless corners from the graph and from the shape
// (useless corners are red nodes with two horizontal or vertical edges)
void refine_result(
    Graph& graph,
    GraphAttributes& attributes,
    NodesPositions& positions,
    Shape& shape
) {
    std::vector<int> nodes_to_remove;
    for (auto& node : graph.get_nodes()) {
        int i = node.get_id();
        if (attributes.get_node_color(i) == Color::BLACK)
            continue;
        std::vector<const GraphEdge*> edges;
        for (auto& edge : node.get_edges())
            edges.push_back(&edge);
        int j_1 = edges[0]->get_to().get_id();
        int j_2 = edges[1]->get_to().get_id();
        // if the added corner is flat, remove it
        if (shape.is_horizontal(i, j_1) == shape.is_horizontal(i, j_2))
            nodes_to_remove.push_back(i);
    }
    for (int i : nodes_to_remove) {
        const auto& node = graph.get_node_by_id(i);
        std::vector<const GraphEdge*> edges;
        for (auto& edge : node.get_edges())
            edges.push_back(&edge);
        int j_1 = edges[0]->get_to().get_id();
        int j_2 = edges[1]->get_to().get_id();
        Direction direction = shape.get_direction(j_1, i);
        graph.remove_node(i);
        graph.add_undirected_edge(j_1, j_2);
        shape.remove_direction(i, j_1);
        shape.remove_direction(i, j_2);
        shape.remove_direction(j_1, i);
        shape.remove_direction(j_2, i);
        shape.set_direction(j_1, j_2, direction);
        shape.set_direction(j_2, j_1, opposite_direction(direction));
        positions.remove_position(i);
    }
}

int get_x_coordinate(int node, const NodesPositions& positions) {
    return positions.get_position_x(node);
}

int get_y_coordinate(int node, const NodesPositions& positions) {
    return positions.get_position_y(node);
}

template <typename Func>
bool are_classes_in_conflict(
    EquivalenceClasses& classes,
    int class_id_1, int class_id_2,
    const NodesPositions& positions,
    Func get_position
) {
    int min_1 = INT_MAX;
    int max_1 = 0;
    for (auto& node : classes.get_elems_of_class(class_id_1)) {
        min_1 = std::min(min_1, get_position(node, positions));
        max_1 = std::max(max_1, get_position(node, positions));
    }
    int min_2 = INT_MAX;
    int max_2 = 0;
    for (auto& node : classes.get_elems_of_class(class_id_2)) {
        min_2 = std::min(min_2, get_position(node, positions));
        max_2 = std::max(max_2, get_position(node, positions));
    }
    if (min_1 > max_2 || min_2 > max_1) return false;
    return true;
}

bool are_classes_in_conflict_x(
    EquivalenceClasses& classes,
    int class_id_1, int class_id_2,
    const NodesPositions& positions
) {
    return are_classes_in_conflict(classes, class_id_1, class_id_2, positions, get_x_coordinate);
}

bool are_classes_in_conflict_y(
    EquivalenceClasses& classes,
    int class_id_1, int class_id_2,
    const NodesPositions& positions
) {
    return are_classes_in_conflict(classes, class_id_1, class_id_2, positions, get_y_coordinate);
}

bool can_move_left(
    int class_id,
    const std::unordered_map<int, std::unordered_set<int>>& coordinate_to_classes,
    int actual_coordinate,
    const NodesPositions& positions,
    EquivalenceClasses& classes_x
) {
    if (actual_coordinate == 0) return false;
    if (coordinate_to_classes.at(actual_coordinate - 1).empty()) return true;
    for (int other_class_id : coordinate_to_classes.at(actual_coordinate - 1))
        if (are_classes_in_conflict_y(classes_x, class_id, other_class_id, positions))
            return false;
    return true;
}

bool can_move_down(
    int class_id,
    const std::unordered_map<int, std::unordered_set<int>>& coordinate_to_classes,
    int actual_coordinate,
    const NodesPositions& positions,
    EquivalenceClasses& classes_y
) {
    if (actual_coordinate == 0) return false;
    if (coordinate_to_classes.at(actual_coordinate - 1).empty()) return true;
    for (int other_class_id : coordinate_to_classes.at(actual_coordinate - 1))
        if (are_classes_in_conflict_x(classes_y, class_id, other_class_id, positions))
            return false;
    return true;
}

NodesPositions compact_area_x(
    const Graph& graph,
    const Shape& shape,
    const NodesPositions& old_positions
) {
    auto classes = build_equivalence_classes(shape, graph);
    auto& classes_x = classes.first;
    auto& classes_y = classes.second;
    std::unordered_map<int, std::unordered_set<int>> coordinate_to_classes;
    int max_coordinate = 0;
    std::unordered_set<int> visited_classes;
    for (const auto& node : graph.get_nodes()) {
        int i = node.get_id();
        int class_id = classes_x.get_class_of_elem(i);
        if (visited_classes.contains(class_id)) continue;
        visited_classes.insert(class_id);
        int x = old_positions.get_position_x(i);
        coordinate_to_classes[x] = {class_id};
        max_coordinate = std::max(max_coordinate, x);
    }
    for (int i = 1; i <= max_coordinate; i++) {
        int actual_coordinate = i;
        int class_id = *coordinate_to_classes[i].begin();
        while (can_move_left(class_id, coordinate_to_classes, actual_coordinate, old_positions, classes_x)) {
            coordinate_to_classes[actual_coordinate - 1].insert(class_id);
            coordinate_to_classes[actual_coordinate].erase(class_id);
            actual_coordinate--;
        }
    }
    NodesPositions new_positions;
    for (auto& [coordinate, classes] : coordinate_to_classes)
        for (auto& class_id : classes)
            for (auto& node_id : classes_x.get_elems_of_class(class_id))
                new_positions.set_position(
                    node_id, coordinate, old_positions.get_position_y(node_id)
                );
    return std::move(new_positions);
}

NodesPositions compact_area_y(
    const Graph& graph,
    const Shape& shape,
    const NodesPositions& old_positions
) {
    auto classes = build_equivalence_classes(shape, graph);
    auto& classes_x = std::get<0>(classes);
    auto& classes_y = std::get<1>(classes);
    std::unordered_map<int, std::unordered_set<int>> coordinate_to_classes;
    int max_coordinate = 0;
    std::unordered_set<int> visited_classes;
    for (const auto& node : graph.get_nodes()) {
        int i = node.get_id();
        int class_id = classes_y.get_class_of_elem(i);
        if (visited_classes.contains(class_id)) continue;
        visited_classes.insert(class_id);
        int y = old_positions.get_position_y(i);
        coordinate_to_classes[y] = {class_id};
        max_coordinate = std::max(max_coordinate, y);
    }
    for (int i = 1; i <= max_coordinate; i++) {
        int actual_coordinate = i;
        int class_id = *coordinate_to_classes[i].begin();
        while (can_move_down(class_id, coordinate_to_classes, actual_coordinate, old_positions, classes_y)) {
            coordinate_to_classes[actual_coordinate - 1].insert(class_id);
            coordinate_to_classes[actual_coordinate].erase(class_id);
            actual_coordinate--;
        }
    }
    NodesPositions new_positions;
    for (auto& [coordinate, classes] : coordinate_to_classes)
        for (auto& class_id : classes)
            for (auto& node_id : classes_y.get_elems_of_class(class_id))
                new_positions.set_position(
                    node_id, old_positions.get_position_x(node_id), coordinate
                );
    return std::move(new_positions);
}

bool check_if_drawing_has_overlappings(const Graph& graph, const NodesPositions& positions) {
    // node - node overlappings
    for (int id : graph.get_nodes_ids()) {
        for (int other_id : graph.get_nodes_ids()) {
            if (id >= other_id) continue;
            if (positions.get_position(id) == positions.get_position(other_id)) {
                std::cout << "Node " << id << " overlaps with node " << other_id << std::endl;
                return true;
            }
        }
    }
    // node - edge overlappings
    for (int id : graph.get_nodes_ids()) {
        int i_x = positions.get_position_x(id);
        int i_y = positions.get_position_y(id);
        for (const auto& edge : graph.get_edges()) {
            int j_1 = edge.get_from().get_id();
            int j_2 = edge.get_to().get_id();
            if (j_1 == id || j_2 == id) continue;
            int j_1_x = positions.get_position_x(j_1);
            int j_1_y = positions.get_position_y(j_1);
            int j_2_x = positions.get_position_x(j_2);
            int j_2_y = positions.get_position_y(j_2);
            if (j_1_y == j_2_y) { // horizontal edge
                if (i_y == j_1_y && i_x >= std::min(j_1_x, j_2_x) && i_x <= std::max(j_1_x, j_2_x)) {
                    std::cout << "Node " << id << " overlaps with edge " << j_1 << "-" << j_2 << std::endl;
                    return true;
                }
            } else { // vertical edge
                if (i_x == j_1_x && i_y >= std::min(j_1_y, j_2_y) && i_y <= std::max(j_1_y, j_2_y)) {
                    std::cout << "Node " << id << " overlaps with edge " << j_1 << "-" << j_2 << std::endl;
                    return true;
                }
            }
        }
    }
    return false;
}

DrawingResult make_rectilinear_drawing_incremental(
    const Graph& graph, std::vector<std::vector<int>>& cycles
) {
    if (!is_graph_undirected(graph))
        throw std::runtime_error("make_rectilinear_drawing_incremental: graph is not undirected");
    if (!is_graph_connected(graph))
        throw std::runtime_error("make_rectilinear_drawing_incremental: graph is not connected");
    auto augmented_graph = std::make_unique<Graph>();
    GraphAttributes attributes;
    attributes.add_attribute(Attribute::NODES_COLOR);
    for (const auto& node : graph.get_nodes()) {
        augmented_graph->add_node(node.get_id());
        attributes.set_node_color(node.get_id(), Color::BLACK);
    }
    for (const auto& node : graph.get_nodes()) {
        if (node.get_degree() > 4)
            throw std::runtime_error("make_rectilinear_drawing_incremental: found node with degree > 4");
        for (auto& edge : node.get_edges())
            augmented_graph->add_edge(node.get_id(), edge.get_to().get_id());
    }
    Shape shape = build_shape(*augmented_graph, attributes, cycles);
    BuildingResult result = build_nodes_positions(shape, *augmented_graph);
    int number_of_added_cycles = 0;
    while (result.type == BuildingResultType::CYCLES_TO_BE_ADDED) {
        for (auto& cycle_to_add : result.cycles_to_be_added)
            cycles.push_back(cycle_to_add);
        number_of_added_cycles += result.cycles_to_be_added.size();
        shape = build_shape(*augmented_graph, attributes, cycles);
        result = build_nodes_positions(shape, *augmented_graph);
    }
    NodesPositions positions(std::move(result.positions.value()));
    int old_size = augmented_graph->size();
    refine_result(*augmented_graph, attributes, positions, shape);
    int number_of_useless_bends = old_size - augmented_graph->size();
    result = build_nodes_positions(shape, *augmented_graph);
    positions = std::move(result.positions.value());
    auto new_positions = compact_area_x(*augmented_graph, shape, positions);
    positions = std::move(new_positions);
    new_positions = compact_area_y(*augmented_graph, shape, positions);
    positions = std::move(new_positions);
    int number_of_corners = augmented_graph->size() - graph.size();
    std::tuple<int, int, double> edge_length_metrics = compute_edge_length_metrics(
        positions, *augmented_graph, attributes
    );
    std::tuple<int, double> bends_metrics = compute_bends_metrics(*augmented_graph, attributes);
    int number_of_crossings = compute_total_crossings(positions, *augmented_graph);
    int total_area = compute_total_area(positions, *augmented_graph);
    return {
        std::move(augmented_graph),
        std::move(attributes),
        std::move(shape),
        std::move(positions),
        number_of_crossings,
        number_of_corners,
        total_area,
        (int)cycles.size() - number_of_added_cycles,
        number_of_added_cycles,
        std::get<0>(edge_length_metrics),
        std::get<1>(edge_length_metrics),
        std::get<2>(edge_length_metrics),
        std::get<0>(bends_metrics),
        std::get<1>(bends_metrics),
        number_of_useless_bends,
    };
}

DrawingResult make_rectilinear_drawing_incremental_basis(const Graph& graph) {
    auto cycles = compute_cycle_basis(graph);
    return make_rectilinear_drawing_incremental(graph, cycles);
}

DrawingResult make_rectilinear_drawing_incremental_no_cycles(const Graph& graph) {
    std::vector<std::vector<int>> cycles;
    return make_rectilinear_drawing_incremental(graph, cycles);
}