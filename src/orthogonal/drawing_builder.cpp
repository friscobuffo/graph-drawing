#include "drawing_builder.hpp"

#include <unordered_set>
#include <queue>
#include <vector>
#include <memory>
#include <tuple>

class EquivalenceClassesHandler {
private:
    std::vector<int> m_class_x;
    std::vector<int> m_class_y;
    std::vector<std::vector<size_t>> m_class_x_to_nodes;
    std::vector<std::vector<size_t>> m_class_y_to_nodes;
public:
    void set_class_x(size_t node, size_t class_id)  {
        while (m_class_x.size() <= node)
            m_class_x.push_back(-1);
        m_class_x[node] = class_id;
        while (m_class_x_to_nodes.size() <= class_id)
            m_class_x_to_nodes.push_back(std::vector<size_t>());
        m_class_x_to_nodes[class_id].push_back(node);
    }
    void set_class_y(size_t node, size_t class_id)  {
        while (m_class_y.size() <= node)
            m_class_y.push_back(-1);
        m_class_y[node] = class_id;
        while (m_class_y_to_nodes.size() <= class_id)
            m_class_y_to_nodes.push_back(std::vector<size_t>());
        m_class_y_to_nodes[class_id].push_back(node);
    }
    int get_class_x(size_t node) const  {
        if (node >= m_class_x.size()) return -1;
        return m_class_x[node];
    }
    int get_class_y(size_t node) const  {
        if (node >= m_class_y.size()) return -1;
        return m_class_y[node];
    }
    const std::vector<size_t>& get_nodes_x(size_t class_id) const {
        return m_class_x_to_nodes[class_id];
    }
    const std::vector<size_t>& get_nodes_y(size_t class_id) const {
        return m_class_y_to_nodes[class_id];
    }
    std::string to_string() const {
        std::string result = "EquivalenceClassesHandler:\n";
        result += "Class X:\n";
        for (size_t i = 0; i < m_class_x.size(); ++i) {
            result += std::to_string(i) + " -> " + std::to_string(m_class_x[i]) + "\n";
        }
        result += "Class Y:\n";
        for (size_t i = 0; i < m_class_y.size(); ++i) {
            result += std::to_string(i) + " -> " + std::to_string(m_class_y[i]) + "\n";
        }
        return result;
    }
    void print() const {
        std::cout << to_string() << std::endl;
    }
};

void _horizontal_edge_expander(
    const Shape& shape,
    const ColoredNodesGraph& graph,
    int left, int right,
    int class_id,
    std::vector<std::vector<bool>>& is_edge_visited,
    EquivalenceClassesHandler& handler
) {
    std::unordered_set<size_t> visited;
    visited.insert(left);
    visited.insert(right);
    while (shape.has_node_a_left_neighbor(left) != -1) {
        int new_left = shape.has_node_a_left_neighbor(left);
        is_edge_visited[left][new_left] = true;
        is_edge_visited[new_left][left] = true;
        left = new_left;
        if (visited.contains(left)) {
            throw std::runtime_error("Cycle detected");
        }
        visited.insert(left);
        handler.set_class_y(left, class_id);
    }
    while (shape.has_node_a_right_neighbor(right) != -1) {
        int new_right = shape.has_node_a_right_neighbor(right);
        is_edge_visited[right][new_right] = true;
        is_edge_visited[new_right][right] = true;
        right = new_right;
        if (visited.contains(right)) {
            throw std::runtime_error("Cycle detected");
        }
        visited.insert(right);
        handler.set_class_y(right, class_id);
    }
}

void _vertical_edge_expander(
    const Shape& shape,
    const ColoredNodesGraph& graph,
    int down, int up,
    int class_id,
    std::vector<std::vector<bool>>& is_edge_visited,
    EquivalenceClassesHandler& handler
) {
    std::unordered_set<size_t> visited;
    visited.insert(down);
    visited.insert(up);
    while (shape.has_node_a_down_neighbor(down) != -1) {
        int new_down = shape.has_node_a_down_neighbor(down);
        is_edge_visited[down][new_down] = true;
        is_edge_visited[new_down][down] = true;
        down = new_down;
        if (visited.contains(down)) {
            throw std::runtime_error("Cycle detected");
        }
        visited.insert(down);
        handler.set_class_x(down, class_id);
    }
    while (shape.has_node_a_up_neighbor(up) != -1) {
        int new_up = shape.has_node_a_up_neighbor(up);
        is_edge_visited[up][new_up] = true;
        is_edge_visited[new_up][up] = true;
        up = new_up;
        if (visited.contains(up)) {
            throw std::runtime_error("Cycle detected");
        }
        visited.insert(up);
        handler.set_class_x(up, class_id);
    }
}

const EquivalenceClassesHandler* build_equivalence_classes(const Shape& shape, const ColoredNodesGraph& graph) {
    EquivalenceClassesHandler* handler = new EquivalenceClassesHandler();
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
                handler->set_class_y(i, next_class_y);
                handler->set_class_y(j, next_class_y);
                int left = i;
                int right = j;
                if (shape.is_left(i, j)) {
                    left = j;
                    right = i;
                }
                _horizontal_edge_expander(shape, graph, left, right, next_class_y, is_edge_visited, *handler);
                ++next_class_y;
            } else {
                handler->set_class_x(i, next_class_x);
                handler->set_class_x(j, next_class_x);
                int down = i;
                int up = j;
                if (shape.is_down(i, j)) {
                    down = j;
                    up = i;
                }
                _vertical_edge_expander(shape, graph, down, up, next_class_x, is_edge_visited, *handler);
                ++next_class_x;
            }
        }
    }
    for (int i = 0; i < graph.size(); ++i) {
        if (handler->get_class_x(i) == -1) {
            handler->set_class_x(i, next_class_x);
            ++next_class_x;
        }
        if (handler->get_class_y(i) == -1) {
            handler->set_class_y(i, next_class_y);
            ++next_class_y;
        }
    }
    return handler;
}

struct PartialOrdering {
private:
    SimpleGraph m_partial_ordering_x;
    SimpleGraph m_partial_ordering_y;;
    std::vector<int> _make_topological_ordering(SimpleGraph& graph) {
        std::vector<int> in_degree(graph.size(), 0);
        for (int u = 0; u < graph.size(); ++u)
            for (auto& edge : graph.get_nodes()[u].get_edges()) {
                int v = edge.get_to();
                in_degree[v]++;
            }
        std::queue<int> queue;
        std::vector<int> topological_order;
        for (int i = 0; i < graph.size(); ++i)
            if (in_degree[i] == 0)
                queue.push(i);
        int count = 0;
        while (!queue.empty()) {
            int u = queue.front();
            ++count;
            queue.pop();
            topological_order.push_back(u);
            for (auto& edge : graph.get_nodes()[u].get_edges()) {
                int v = edge.get_to();
                if (--in_degree[v] == 0)
                    queue.push(v);
            }
        }
        if (count != graph.size())
            throw std::runtime_error("Graph contains cycle");
        return topological_order;
    }
public:
    void add_edge_x(int from, int to) {
        while (m_partial_ordering_x.size() <= from) {
            m_partial_ordering_x.add_node();
        }
        while (m_partial_ordering_x.size() <= to) {
            m_partial_ordering_x.add_node();
        }
        m_partial_ordering_x.add_edge(from, to);
    }
    void add_edge_y(int from, int to) {
        while (m_partial_ordering_y.size() <= from) {
            m_partial_ordering_y.add_node();
        }
        while (m_partial_ordering_y.size() <= to) {
            m_partial_ordering_y.add_node();
        }
        m_partial_ordering_y.add_edge(from, to);
    }
    std::tuple<std::vector<int>, std::vector<int>> make_topological_ordering() {
        auto topological_order_x = _make_topological_ordering(m_partial_ordering_x);
        auto topological_order_y = _make_topological_ordering(m_partial_ordering_y);
        return std::make_tuple(topological_order_x, topological_order_y);
    }
};

PartialOrdering* equivalence_classes_to_partial_ordering(
    const EquivalenceClassesHandler& classes,
    const ColoredNodesGraph& graph,
    const Shape& shape
) {
    PartialOrdering* ordering = new PartialOrdering();
    for (int i = 0; i < graph.size(); ++i) {
        for (auto& edge : graph.get_nodes()[i].get_edges()) {
            int j = edge.get_to();
            if (shape.is_horizontal(i, j)) {
                int node_class_x = classes.get_class_x(i);
                int neighbor_class_x = classes.get_class_x(j);
                if (shape.is_left(i, j))
                    ordering->add_edge_x(neighbor_class_x, node_class_x);
                else
                    ordering->add_edge_x(node_class_x, neighbor_class_x);
            } else {
                int node_class_y = classes.get_class_y(i);
                int neighbor_class_y = classes.get_class_y(j);
                if (shape.is_down(i, j))
                    ordering->add_edge_y(neighbor_class_y, node_class_y);
                else
                    ordering->add_edge_y(node_class_y, neighbor_class_y);
            }
        }
    }
    return ordering;
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

const NodesPositions* build_nodes_positions(const Shape& shape, const ColoredNodesGraph& graph) {
    const EquivalenceClassesHandler* classes = build_equivalence_classes(shape, graph);
    PartialOrdering* partial_ordering = equivalence_classes_to_partial_ordering(*classes, graph, shape);
    auto [classes_x_ordering, classes_y_ordering] = partial_ordering->make_topological_ordering();
    NodesPositions* positions = new NodesPositions();
    int current_position_x = 0;
    for (auto& class_id : classes_x_ordering) {
        for (auto& node : classes->get_nodes_x(class_id))
            positions->set_position_x(node, current_position_x);
        ++current_position_x;
    }
    int current_position_y = 0;
    for (auto& class_id : classes_y_ordering) {
        for (auto& node : classes->get_nodes_y(class_id))
            positions->set_position_y(node, current_position_y);
        ++current_position_y;
    }
    delete classes;
    delete partial_ordering;
    return positions;
}