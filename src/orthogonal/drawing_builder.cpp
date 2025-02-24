#include "drawing_builder.hpp"

#include <unordered_set>
#include <vector>
#include <memory>
#include <tuple>
#include <list>
#include <optional>
#include <cassert>

#include "../core/graph/algorithms.hpp"

class EquivalenceClasses {
private:
    std::vector<int> m_elem_to_class;
    std::vector<std::vector<size_t>> m_class_to_elems;
public:
    void set_class(size_t elem, size_t class_id)  {
        while (m_elem_to_class.size() <= elem)
            m_elem_to_class.push_back(-1);
        m_elem_to_class[elem] = class_id;
        while (m_class_to_elems.size() <= class_id)
            m_class_to_elems.push_back(std::vector<size_t>());
        m_class_to_elems[class_id].push_back(elem);
    }
    int get_class(size_t elem) const  {
        if (elem >= m_elem_to_class.size()) return -1;
        return m_elem_to_class[elem];
    }
    const std::vector<size_t>& get_elems(size_t class_id) const {
        return m_class_to_elems[class_id];
    }
    std::string to_string() const {
        std::string result = "EquivalenceClasses:\n";
        for (size_t i = 0; i < m_elem_to_class.size(); ++i)
            result += std::to_string(i) + " -> " + std::to_string(m_elem_to_class[i]) + "\n";
        return result;
    }
    size_t number_of_classes() const {
        return m_class_to_elems.size();
    }
    void print() const { std::cout << to_string() << std::endl; }
};

void horizontal_edge_expander(
    const Shape& shape,
    const ColoredNodesGraph& graph,
    int left, int right,
    int class_id,
    std::vector<std::vector<bool>>& is_edge_visited,
    EquivalenceClasses& equivalence_classes_y
) {
    std::unordered_set<size_t> visited;
    visited.insert(left);
    visited.insert(right);
    std::list<size_t> nodes_in_class;
    nodes_in_class.push_back(left);
    nodes_in_class.push_back(right);
    while (shape.has_node_a_left_neighbor(left) != -1) {
        int new_left = shape.has_node_a_left_neighbor(left);
        is_edge_visited[left][new_left] = true;
        is_edge_visited[new_left][left] = true;
        left = new_left;
        if (visited.contains(left)) break;
        visited.insert(left);
        nodes_in_class.push_front(left);
    }
    while (shape.has_node_a_right_neighbor(right) != -1) {
        int new_right = shape.has_node_a_right_neighbor(right);
        is_edge_visited[right][new_right] = true;
        is_edge_visited[new_right][right] = true;
        right = new_right;
        if (visited.contains(right)) break;
        visited.insert(right);
        nodes_in_class.push_back(right);
    }
    for (auto& node : nodes_in_class)
        equivalence_classes_y.set_class(node, class_id);
}

void vertical_edge_expander(
    const Shape& shape,
    const ColoredNodesGraph& graph,
    int down, int up,
    int class_id,
    std::vector<std::vector<bool>>& is_edge_visited,
    EquivalenceClasses& equivalence_classes_x
) {
    std::unordered_set<size_t> visited;
    visited.insert(down);
    visited.insert(up);
    std::list<size_t> nodes_in_class;
    nodes_in_class.push_back(down);
    nodes_in_class.push_back(up);
    while (shape.has_node_a_down_neighbor(down) != -1) {
        int new_down = shape.has_node_a_down_neighbor(down);
        is_edge_visited[down][new_down] = true;
        is_edge_visited[new_down][down] = true;
        down = new_down;
        if (visited.contains(down)) break;
        visited.insert(down);
        nodes_in_class.push_front(down);
    }
    while (shape.has_node_a_up_neighbor(up) != -1) {
        int new_up = shape.has_node_a_up_neighbor(up);
        is_edge_visited[up][new_up] = true;
        is_edge_visited[new_up][up] = true;
        up = new_up;
        if (visited.contains(up)) break;
        visited.insert(up);
        nodes_in_class.push_back(up);
    }
    for (auto& node : nodes_in_class)
        equivalence_classes_x.set_class(node, class_id);
}

const std::tuple<EquivalenceClasses*, EquivalenceClasses*> build_equivalence_classes(
    const Shape& shape,
    const ColoredNodesGraph& graph
) {
    EquivalenceClasses* equivalence_classes_x = new EquivalenceClasses();
    EquivalenceClasses* equivalence_classes_y = new EquivalenceClasses();
    int next_class_x = 0;
    int next_class_y = 0;
    std::vector<std::vector<bool>> is_edge_visited(graph.size(), std::vector<bool>(graph.size(), false));
    for (int i = 0; i < graph.size(); ++i) {
        for (auto& edge : graph.get_nodes()[i].get_edges()) {
            int j = edge.get_to();
            if (is_edge_visited[i][j]) continue;
            is_edge_visited[i][j] = true;
            is_edge_visited[j][i] = true;
            if (shape.is_horizontal(i, j)) {
                int left = i;
                int right = j;
                if (shape.is_left(i, j)) {
                    left = j;
                    right = i;
                }
                horizontal_edge_expander(shape, graph, left, right, next_class_y, is_edge_visited, *equivalence_classes_y);
                ++next_class_y;
            } else {
                int down = i;
                int up = j;
                if (shape.is_down(i, j)) {
                    down = j;
                    up = i;
                }
                vertical_edge_expander(shape, graph, down, up, next_class_x, is_edge_visited, *equivalence_classes_x);
                ++next_class_x;
            }
        }
    }
    for (int i = 0; i < graph.size(); ++i) {
        if (equivalence_classes_x->get_class(i) == -1) {
            equivalence_classes_x->set_class(i, next_class_x);
            ++next_class_x;
        }
        if (equivalence_classes_y->get_class(i) == -1) {
            equivalence_classes_y->set_class(i, next_class_y);
            ++next_class_y;
        }
    }
    for (int i = 0; i < graph.size(); ++i) {
        assert(equivalence_classes_x->get_class(i) != -1);
        assert(equivalence_classes_y->get_class(i) != -1);
    }
    for (int i = 0; i < equivalence_classes_x->number_of_classes(); ++i)
        assert(equivalence_classes_x->get_elems(i).size() > 0);
    for (int i = 0; i < equivalence_classes_y->number_of_classes(); ++i)
        assert(equivalence_classes_y->get_elems(i).size() > 0);
    return std::make_tuple(equivalence_classes_x, equivalence_classes_y);
}

auto equivalence_classes_to_ordering(
    const EquivalenceClasses& equivalence_classes_x,
    const EquivalenceClasses& equivalence_classes_y,
    const ColoredNodesGraph& graph,
    const Shape& shape
) {
    auto* ordering_x = new LabeledEdgeGraph<std::tuple<size_t,size_t>>();
    auto* ordering_y = new LabeledEdgeGraph<std::tuple<size_t,size_t>>();
    for (int i = 0; i < equivalence_classes_x.number_of_classes(); ++i)
        ordering_x->add_node();
    for (int i = 0; i < equivalence_classes_y.number_of_classes(); ++i)
        ordering_y->add_node();
    const std::tuple<size_t, size_t>** ordering_x_edge_to_graph_edge =
        (const std::tuple<size_t, size_t>**)malloc(sizeof(const std::tuple<size_t, size_t>*) * ordering_x->size() * ordering_x->size());
    const std::tuple<size_t, size_t>** ordering_y_edge_to_graph_edge =
        (const std::tuple<size_t, size_t>**)malloc(sizeof(const std::tuple<size_t, size_t>*) * ordering_y->size() * ordering_y->size());
    for (int i = 0; i < graph.size(); ++i) {
        for (auto& edge : graph.get_nodes()[i].get_edges()) {
            int j = edge.get_to();
            if (shape.is_horizontal(i, j)) {
                int node_class_x = equivalence_classes_x.get_class(i);
                int neighbor_class_x = equivalence_classes_x.get_class(j);
                if (shape.is_right(i, j)) {
                    ordering_x->add_edge(node_class_x, neighbor_class_x, std::make_tuple(i, j));
                    int last_edge_index = ordering_x->get_nodes()[node_class_x].get_edges().size() - 1;
                    auto& edge = ordering_x->get_nodes()[node_class_x].get_edges()[last_edge_index];
                    ordering_x_edge_to_graph_edge[node_class_x * ordering_x->size() + neighbor_class_x] = &edge.get_label();
                }
            } else {
                int node_class_y = equivalence_classes_y.get_class(i);
                int neighbor_class_y = equivalence_classes_y.get_class(j);
                if (shape.is_up(i, j)) {
                    ordering_y->add_edge(node_class_y, neighbor_class_y, std::make_tuple(i, j));
                    int last_edge_index = ordering_y->get_nodes()[node_class_y].get_edges().size() - 1;
                    auto& edge = ordering_y->get_nodes()[node_class_y].get_edges()[last_edge_index];
                    ordering_y_edge_to_graph_edge[node_class_y * ordering_y->size() + neighbor_class_y] = &edge.get_label();
                }
            }
        }
    }
    return std::make_tuple(
        ordering_x,
        ordering_y,
        ordering_x_edge_to_graph_edge,
        ordering_y_edge_to_graph_edge
    );
}

void NodesPositions::set_position_x(size_t node, size_t position) {
    while (m_positions.size() <= node)
        m_positions.push_back(std::make_pair(-1, -1));
    m_positions[node].first = position;
}

void NodesPositions::set_position_y(size_t node, size_t position) {
    while (m_positions.size() <= node)
        m_positions.push_back(std::make_pair(-1, -1));
    m_positions[node].second = position;
}

int NodesPositions::get_position_x(size_t node) const {
    return m_positions[node].first;
}

int NodesPositions::get_position_y(size_t node) const {
    return m_positions[node].second;
}

std::vector<size_t> path_in_class(size_t from, size_t to, const std::vector<size_t>& class_elems) {
    std::vector<size_t> path;
    int from_pos = -1;
    int to_pos = -1;
    for (int i = 0; i < class_elems.size(); ++i) {
        if (class_elems[i] == from) from_pos = i;
        if (class_elems[i] == to) to_pos = i;
    }
    assert(from_pos != -1 && to_pos != -1);
    if (from_pos > to_pos) {
        auto path = path_in_class(to, from, class_elems);
        std::reverse(path.begin(), path.end());
        return path;
    }
    for (int i = from_pos; i <= to_pos; ++i)
        path.push_back(class_elems[i]);
    return path;
}

std::vector<size_t> build_cycle_in_graph_from_cycle_in_ordering(
    const std::vector<size_t>& cycle_in_ordering,
    const LabeledEdgeGraph<std::tuple<size_t, size_t>>& ordering,
    const EquivalenceClasses& equivalence_classes,
    const std::tuple<size_t, size_t>** ordering_edge_to_graph_edge
) {
    std::vector<size_t> cycle;
    for (int i = 0; i < cycle_in_ordering.size(); ++i) {
        size_t class_id = cycle_in_ordering[i];
        size_t next_class_id = cycle_in_ordering[(i+1)%cycle_in_ordering.size()];
        size_t from = get<0>(*ordering_edge_to_graph_edge[class_id * ordering.size() + next_class_id]);
        size_t to = get<1>(*ordering_edge_to_graph_edge[class_id * ordering.size() + next_class_id]);
        cycle.push_back(from);
        size_t next_next_class_id = cycle_in_ordering[(i+2)%cycle_in_ordering.size()];
        size_t next_from = get<0>(*ordering_edge_to_graph_edge[next_class_id * ordering.size() + next_next_class_id]);
        if (to != next_from) {
            auto path = path_in_class(to, next_from, equivalence_classes.get_elems(next_class_id));
            for (int i = 0; i < path.size()-1; ++i)
                cycle.push_back(path[i]);
        }
    }
    return cycle;
}

BuildingResult* build_nodes_positions(const Shape& shape, const ColoredNodesGraph& graph) {
    auto classes = build_equivalence_classes(shape, graph);
    auto classes_x = std::unique_ptr<EquivalenceClasses>(std::get<0>(classes));
    auto classes_y = std::unique_ptr<EquivalenceClasses>(std::get<1>(classes));
    auto ordering = equivalence_classes_to_ordering(*classes_x, *classes_y, graph, shape);
    auto ordering_x = std::unique_ptr<LabeledEdgeGraph<std::tuple<size_t, size_t>>>(std::get<0>(ordering));
    auto ordering_y = std::unique_ptr<LabeledEdgeGraph<std::tuple<size_t, size_t>>>(std::get<1>(ordering));
    auto ordering_x_edge_to_graph_edge = std::get<2>(ordering);
    auto ordering_y_edge_to_graph_edge = std::get<3>(ordering);
    auto cycle_x = find_a_cycle_directed_graph(*ordering_x);
    if (cycle_x.has_value()) {
        auto cycle = build_cycle_in_graph_from_cycle_in_ordering(cycle_x.value(), *ordering_x, *classes_x, ordering_x_edge_to_graph_edge);
        free(ordering_x_edge_to_graph_edge);
        free(ordering_y_edge_to_graph_edge);
        return new BuildingResult{nullptr, cycle, BuildingResultType::CYCLE_TO_BE_ADDED};
    }
    auto cycle_y = find_a_cycle_directed_graph(*ordering_y);
    if (cycle_y.has_value()) {
        auto cycle = build_cycle_in_graph_from_cycle_in_ordering(cycle_y.value(), *ordering_y, *classes_y, ordering_y_edge_to_graph_edge);
        free(ordering_x_edge_to_graph_edge);
        free(ordering_y_edge_to_graph_edge);
        return new BuildingResult{nullptr, cycle, BuildingResultType::CYCLE_TO_BE_ADDED};
    }
    auto classes_x_ordering = make_topological_ordering(*ordering_x);
    auto classes_y_ordering = make_topological_ordering(*ordering_y);
    NodesPositions* positions = new NodesPositions();
    int current_position_x = 0;
    for (auto& class_id : classes_x_ordering) {
        for (auto& node : classes_x->get_elems(class_id))
            positions->set_position_x(node, current_position_x);
        ++current_position_x;
    }
    int current_position_y = 0;
    for (auto& class_id : classes_y_ordering) {
        for (auto& node : classes_y->get_elems(class_id))
            positions->set_position_y(node, current_position_y);
        ++current_position_y;
    }
    free(ordering_x_edge_to_graph_edge);
    free(ordering_y_edge_to_graph_edge);
    return new BuildingResult{positions, {}, BuildingResultType::OK};
}

void node_positions_to_svg(const NodesPositions& positions, const ColoredNodesGraph& graph) {
    int max_x = 0;
    int max_y = 0;
    for (size_t i = 0; i < graph.size(); ++i) {
        max_x = std::max(max_x, positions.get_position_x(i));
        max_y = std::max(max_y, positions.get_position_y(i));
    }
    SvgDrawer drawer{800, 600};
    ScaleLinear scale_x = ScaleLinear(0, max_x+2, 0, 800);
    ScaleLinear scale_y = ScaleLinear(0, max_y+2, 0, 600);
    std::vector<Point2D> points;
    for (size_t i = 0; i < graph.size(); ++i) {
        double x = scale_x.map(positions.get_position_x(i)+1);
        double y = scale_y.map(positions.get_position_y(i)+1);
        points.push_back(Point2D{x, y});
    }
    for (size_t i = 0; i < graph.size(); ++i)
        for (auto& edge : graph.get_nodes()[i].get_edges()) {
            size_t j = edge.get_to();
            Line2D line{points[i], points[j]};
            drawer.add(line);
        }
    for (size_t i = 0; i < graph.size(); ++i) {
        Color color = graph.get_nodes()[i].get_color();
        drawer.add(points[i], color_to_string(color), std::to_string(i));
    }
    drawer.saveToFile("graph.svg");
}