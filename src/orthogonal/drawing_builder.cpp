#include "drawing_builder.hpp"

#include <list>

class EquivalenceClasses {
private:
    std::vector<int> m_elem_to_class;
    std::vector<std::vector<size_t>> m_class_to_elems;
public:
    void set_class(size_t elem, size_t class_id)  {
        while (m_elem_to_class.size() <= elem)
            m_elem_to_class.push_back(-1);
        assert(m_elem_to_class[elem] == -1);
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
        for (auto& edge : graph.get_node(i).get_edges()) {
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
    auto* ordering_x = new LabeledEdgeGraph<Pair<Int>>();
    auto* ordering_y = new LabeledEdgeGraph<Pair<Int>>();
    for (int i = 0; i < equivalence_classes_x.number_of_classes(); ++i)
        ordering_x->add_node();
    for (int i = 0; i < equivalence_classes_y.number_of_classes(); ++i)
        ordering_y->add_node();
    const Pair<Int>** ordering_x_edge_to_graph_edge =
        (const Pair<Int>**)malloc(sizeof(const Pair<Int>*) * ordering_x->size() * ordering_x->size());
    const Pair<Int>** ordering_y_edge_to_graph_edge =
        (const Pair<Int>**)malloc(sizeof(const Pair<Int>*) * ordering_y->size() * ordering_y->size());
    for (int i = 0; i < graph.size(); ++i) {
        for (auto& edge : graph.get_node(i).get_edges()) {
            int j = edge.get_to();
            if (shape.is_horizontal(i, j)) {
                int node_class_x = equivalence_classes_x.get_class(i);
                int neighbor_class_x = equivalence_classes_x.get_class(j);
                if (shape.is_right(i, j)) {
                    ordering_x->add_edge(node_class_x, neighbor_class_x, Pair<Int>{i, j});
                    int last_edge_index = ordering_x->get_node(node_class_x).get_degree() - 1;
                    auto& edge = ordering_x->get_node(node_class_x).get_edge(last_edge_index);
                    ordering_x_edge_to_graph_edge[node_class_x * ordering_x->size() + neighbor_class_x] = &edge.get_label();
                }
            } else {
                int node_class_y = equivalence_classes_y.get_class(i);
                int neighbor_class_y = equivalence_classes_y.get_class(j);
                if (shape.is_up(i, j)) {
                    ordering_y->add_edge(node_class_y, neighbor_class_y, Pair<Int>{i, j});
                    int last_edge_index = ordering_y->get_node(node_class_y).get_degree() - 1;
                    auto& edge = ordering_y->get_node(node_class_y).get_edge(last_edge_index);
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
    const LabeledEdgeGraph<Pair<Int>>& ordering,
    const EquivalenceClasses& equivalence_classes,
    const Pair<Int>** ordering_edge_to_graph_edge
) {
    std::vector<size_t> cycle;
    for (int i = 0; i < cycle_in_ordering.size(); ++i) {
        size_t class_id = cycle_in_ordering[i];
        size_t next_class_id = cycle_in_ordering[(i+1)%cycle_in_ordering.size()];
        int from = (ordering_edge_to_graph_edge[class_id * ordering.size() + next_class_id])->first.value;
        int to = (ordering_edge_to_graph_edge[class_id * ordering.size() + next_class_id])->second.value;
        cycle.push_back(from);
        size_t next_next_class_id = cycle_in_ordering[(i+2)%cycle_in_ordering.size()];
        size_t next_from = (ordering_edge_to_graph_edge[next_class_id * ordering.size() + next_next_class_id])->first.value;
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
    auto ordering_x = std::unique_ptr<LabeledEdgeGraph<Pair<Int>>>(std::get<0>(ordering));
    auto ordering_y = std::unique_ptr<LabeledEdgeGraph<Pair<Int>>>(std::get<1>(ordering));
    auto ordering_x_edge_to_graph_edge = std::get<2>(ordering);
    auto ordering_y_edge_to_graph_edge = std::get<3>(ordering);
    auto cycle_x = find_a_cycle_directed_graph(*ordering_x);
    auto cycle_y = find_a_cycle_directed_graph(*ordering_y);
    // std::vector<size_t> cycle_y_in_original_graph;

    if (cycle_x.has_value() || cycle_y.has_value()) {
        std::vector<std::vector<size_t>> cycles_to_be_added;
        if (cycle_x.has_value()) {
            auto cycle_x_in_original_graph = build_cycle_in_graph_from_cycle_in_ordering(
                cycle_x.value(), *ordering_x, *classes_x, ordering_x_edge_to_graph_edge
            );
            cycles_to_be_added.push_back(cycle_x_in_original_graph);
        }
        if (cycle_y.has_value()) {
            auto cycle_y_in_original_graph = build_cycle_in_graph_from_cycle_in_ordering(
                cycle_y.value(), *ordering_y, *classes_y, ordering_y_edge_to_graph_edge
            );
            cycles_to_be_added.push_back(cycle_y_in_original_graph);
        }
        free(ordering_x_edge_to_graph_edge);
        free(ordering_y_edge_to_graph_edge);
        return new BuildingResult{nullptr, cycles_to_be_added, BuildingResultType::CYCLES_TO_BE_ADDED};
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

void node_positions_to_svg(
    const NodesPositions& positions,
    const ColoredNodesGraph& graph,
    const std::string& filename
) {
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
        for (auto& edge : graph.get_node(i).get_edges()) {
            size_t j = edge.get_to();
            Line2D line{points[i], points[j]};
            drawer.add(line);
        }
    for (size_t i = 0; i < graph.size(); ++i) {
        Color color = graph.get_node(i).get_color();
        drawer.add(points[i], color_to_string(color), std::to_string(i));
    }
    drawer.saveToFile(filename);
}

int compute_total_edge_length(
    const NodesPositions &positions,
    const ColoredNodesGraph &graph)
{
    int total_edge_length = 0;
    for (size_t i = 0; i < graph.size(); ++i)
    {
        for (auto &edge : graph.get_node(i).get_edges())
        {
            size_t j = edge.get_to();
            if (i < j)
            {
                int x1 = positions.get_position_x(i);
                int y1 = positions.get_position_y(i);
                int x2 = positions.get_position_x(j);
                int y2 = positions.get_position_y(j);
                total_edge_length += std::abs(x1 - x2) + std::abs(y1 - y2);
            }
        }
    }
    return total_edge_length;
}

int compute_total_area(const NodesPositions& positions, const ColoredNodesGraph& graph) {
    int min_x = graph.size();
    int min_y = graph.size();
    int max_x = 0;
    int max_y = 0;
    for (size_t i = 0; i < graph.size(); ++i) {
        min_x = std::min(min_x, positions.get_position_x(i));
        min_y = std::min(min_y, positions.get_position_y(i));
        max_x = std::max(max_x, positions.get_position_x(i));
        max_y = std::max(max_y, positions.get_position_y(i));
    }
    return (max_x - min_x + 1) * (max_y - min_y + 1);
}

bool do_edges_cross(
    const NodesPositions& positions,
    size_t i, size_t j,
    size_t k, size_t l
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

int compute_total_crossings(const NodesPositions& positions, const ColoredNodesGraph& graph) {
    int total_crossings = 0;
    // for each pair of edges, check if they cross
    for (size_t i = 0; i < graph.size(); ++i) {
        for (auto& edge1 : graph.get_node(i).get_edges()) {
            size_t j = edge1.get_to();
            for (size_t k = i+1; k < graph.size(); ++k) {
                for (auto& edge2 : graph.get_node(k).get_edges()) {
                    size_t l = edge2.get_to();
                    if (j == l || i == l || j == k) continue;
                    if (do_edges_cross(positions, i, j, k, l))
                        total_crossings++;
                }
            }
        }
    }
    return total_crossings/4;
}

void refine_result(
    const ColoredNodesGraph& graph,
    const Shape& shape,
    ColoredNodesGraph& refined_graph,
    Shape& refined_shape
) {
    int next_index = 0;
    std::vector<int> old_to_new_index;
    for (int i = 0; i < graph.size(); ++i) {
        auto& node = graph.get_node(i);
        if (node.get_color() == Color::BLACK) {
            refined_graph.add_node(Color::BLACK);
            old_to_new_index.push_back(next_index++);
            continue;
        }
        assert(node.get_color() == Color::RED);
        int j_1 = node.get_edges()[0].get_to();
        int j_2 = node.get_edges()[1].get_to();
        assert(node.get_degree() == 2);
        if (shape.is_horizontal(i, j_1) != shape.is_horizontal(i, j_2)) {
            refined_graph.add_node(Color::RED);
            old_to_new_index.push_back(next_index++);
            continue;
        }
        old_to_new_index.push_back(-1);
    }
    assert(graph.size() == old_to_new_index.size());
    for (int i = 0; i < graph.size(); ++i) {
        auto& node = graph.get_node(i);
        switch (node.get_color()) {
            case Color::BLACK:
                for (auto& edge : node.get_edges()) {
                    int j = edge.get_to();
                    if (i < j && graph.get_node(j).get_color() == Color::BLACK) {
                        refined_graph.add_undirected_edge(old_to_new_index[i], old_to_new_index[j]);
                        refined_shape.set_direction(old_to_new_index[i], old_to_new_index[j], shape.get_direction(i, j));
                        refined_shape.set_direction(old_to_new_index[j], old_to_new_index[i], shape.get_direction(j, i));
                    }
                    else if (graph.get_node(j).get_color() == Color::RED && 
                             old_to_new_index[j] != -1 &&
                             old_to_new_index[j] != -2) {
                        refined_graph.add_undirected_edge(old_to_new_index[i], old_to_new_index[j]);
                        refined_shape.set_direction(old_to_new_index[i], old_to_new_index[j], shape.get_direction(i, j));
                        refined_shape.set_direction(old_to_new_index[j], old_to_new_index[i], shape.get_direction(j, i));
                    }
                }
                break;
            case Color::RED:
                if (old_to_new_index[i] == -2) continue;
                int j_1 = node.get_edges()[0].get_to();
                int j_2 = node.get_edges()[1].get_to();
                if (old_to_new_index[i] >= 0) {
                    if (graph.get_node(j_1).get_color() == Color::RED && old_to_new_index[j_1] >= 0 && i < j_1) {
                        refined_graph.add_undirected_edge(old_to_new_index[i], old_to_new_index[j_1]);
                        refined_shape.set_direction(old_to_new_index[i], old_to_new_index[j_1], shape.get_direction(i, j_1));
                        refined_shape.set_direction(old_to_new_index[j_1], old_to_new_index[i], shape.get_direction(j_1, i));
                    }
                    if (graph.get_node(j_2).get_color() == Color::RED && old_to_new_index[j_2] >= 0 && i < j_2) {
                        refined_graph.add_undirected_edge(old_to_new_index[i], old_to_new_index[j_2]);
                        refined_shape.set_direction(old_to_new_index[i], old_to_new_index[j_2], shape.get_direction(i, j_2));
                        refined_shape.set_direction(old_to_new_index[j_2], old_to_new_index[i], shape.get_direction(j_2, i));
                    }
                } else if (old_to_new_index[i] == -1) {
                    old_to_new_index[i] = -2;
                    int prev = i;
                    int old_j_2 = j_2;
                    while (old_to_new_index[j_1] == -1) {
                        old_to_new_index[j_1] = -2;
                        int j_1_ = graph.get_node(j_1).get_edges()[0].get_to();
                        int j_2_ = graph.get_node(j_1).get_edges()[1].get_to();
                        if (prev == j_2_) {
                            prev = j_1;
                            j_1 = j_1_;
                        } else {
                            prev = j_1;
                            j_1 = j_2_;
                        }
                    }
                    prev = i;
                    while (old_to_new_index[j_2] == -1) {
                        old_to_new_index[j_2] = -2;
                        int j_1_ = graph.get_node(j_2).get_edges()[0].get_to();
                        int j_2_ = graph.get_node(j_2).get_edges()[1].get_to();
                        if (prev == j_2_) {
                            prev = j_2;
                            j_2 = j_1_;
                        } else {
                            prev = j_2;
                            j_2 = j_2_;
                        }
                    }
                    refined_graph.add_undirected_edge(old_to_new_index[j_1], old_to_new_index[j_2]);
                    refined_shape.set_direction(old_to_new_index[j_1], old_to_new_index[j_2], shape.get_direction(i, old_j_2));
                    refined_shape.set_direction(old_to_new_index[j_2], old_to_new_index[j_1], shape.get_direction(old_j_2, i));
                }
                break;
        }
    }
}

int get_x_coordinate(int node, const NodesPositions& positions) {
    return positions.get_position_x(node);
}

int get_y_coordinate(int node, const NodesPositions& positions) {
    return positions.get_position_y(node);
}

// this function instead of associating a different coordinate to each class,
// it associates the same coordinate to close classes that are not in conflict
// this way the drawing will be more compact
template <typename Func1, typename Func2>
std::vector<std::vector<size_t>> make_topological_ordering_smart(
    const LabeledEdgeGraph<Pair<Int>>& graph,
    EquivalenceClasses& classes,
    Func1 are_classes_in_conflict,
    const NodesPositions& positions,
    Func2 get_coordinate
) {
    std::vector<int> in_degree(graph.size(), 0);
    for (int u = 0; u < graph.size(); ++u)
        for (auto& edge : graph.get_node(u).get_edges()) {
            int v = edge.get_to();
            in_degree[v]++;
        }
    std::unordered_set<int> zero_degree;
    std::vector<std::vector<size_t>> topological_order;
    for (int i = 0; i < graph.size(); ++i)
        if (in_degree[i] == 0)
            zero_degree.insert(i);
    while (!zero_degree.empty()) {
        std::vector<size_t> are_equals;
        int u = *zero_degree.begin();
        for (int elem : zero_degree)
            if (get_coordinate(elem, positions) < get_coordinate(u, positions))
                u = elem;
        zero_degree.erase(u);
        are_equals.push_back(u);
        for (int elem : zero_degree) {
            bool can_be_added = true;
            for (auto& class_id : are_equals) {
                if (are_classes_in_conflict(classes, elem, class_id, positions)) {
                    can_be_added = false;
                    break;
                }
            }
            if (can_be_added)
                are_equals.push_back(elem);
        }
        for (auto& class_id : are_equals)
            zero_degree.erase(class_id);
        topological_order.push_back(are_equals);
        std::queue<int> next_queue;
        for (int u : are_equals) {
            for (auto& edge : graph.get_node(u).get_edges()) {
                int v = edge.get_to();
                if (--in_degree[v] == 0)
                    zero_degree.insert(v);
            }
        }
    }
    return topological_order;
}

bool are_classes_in_conflict_x(
    EquivalenceClasses& classes,
    int class_id_1, int class_id_2,
    const NodesPositions& positions
) {
    int min_y_1 = INT_MAX;
    int max_y_1 = 0;
    for (auto& node : classes.get_elems(class_id_1)) {
        min_y_1 = std::min(min_y_1, positions.get_position_y(node));
        max_y_1 = std::max(max_y_1, positions.get_position_y(node));
    }
    int min_y_2 = INT_MAX;
    int max_y_2 = 0;
    for (auto& node : classes.get_elems(class_id_2)) {
        min_y_2 = std::min(min_y_2, positions.get_position_y(node));
        max_y_2 = std::max(max_y_2, positions.get_position_y(node));
    }
    if (min_y_1 > max_y_2 || min_y_2 > max_y_1) return false;
    return true;
}

bool are_classes_in_conflict_y(
    EquivalenceClasses& classes,
    int class_id_1, int class_id_2,
    const NodesPositions& positions
) {
    int min_x_1 = INT_MAX;
    int max_x_1 = 0;
    for (auto& node : classes.get_elems(class_id_1)) {
        min_x_1 = std::min(min_x_1, positions.get_position_x(node));
        max_x_1 = std::max(max_x_1, positions.get_position_x(node));
    }
    int min_x_2 = INT_MAX;
    int max_x_2 = 0;
    for (auto& node : classes.get_elems(class_id_2)) {
        min_x_2 = std::min(min_x_2, positions.get_position_x(node));
        max_x_2 = std::max(max_x_2, positions.get_position_x(node));
    }
    if (min_x_1 > max_x_2 || min_x_2 > max_x_1) return false;
    return true;
}

NodesPositions* compact_area_x(
    const ColoredNodesGraph& graph,
    const Shape& shape,
    const NodesPositions& old_positions
) {
    auto classes = build_equivalence_classes(shape, graph);
    auto classes_x = std::unique_ptr<EquivalenceClasses>(std::get<0>(classes));
    auto classes_y = std::unique_ptr<EquivalenceClasses>(std::get<1>(classes));
    auto ordering = equivalence_classes_to_ordering(*classes_x, *classes_y, graph, shape);
    auto ordering_x = std::unique_ptr<LabeledEdgeGraph<Pair<Int>>>(std::get<0>(ordering));
    delete std::get<1>(ordering);
    free(std::get<2>(ordering));
    free(std::get<3>(ordering));
    auto classes_x_ordering = make_topological_ordering_smart(*ordering_x, *classes_x,
        are_classes_in_conflict_x, old_positions, get_x_coordinate);
    NodesPositions* new_positions = new NodesPositions();
    int current_position_x = 0;
    for (auto& classes_id : classes_x_ordering) {
        for (auto& class_id : classes_id)
            for (auto& node : classes_x->get_elems(class_id)) {
                new_positions->set_position_x(node, current_position_x);
                new_positions->set_position_y(node, old_positions.get_position_y(node));
            }
        ++current_position_x;
    }
    return new_positions;
}

NodesPositions* compact_area_y(
    const ColoredNodesGraph& graph,
    const Shape& shape,
    const NodesPositions& old_positions
) {
    auto classes = build_equivalence_classes(shape, graph);
    auto classes_x = std::unique_ptr<EquivalenceClasses>(std::get<0>(classes));
    auto classes_y = std::unique_ptr<EquivalenceClasses>(std::get<1>(classes));
    auto ordering = equivalence_classes_to_ordering(*classes_x, *classes_y, graph, shape);
    delete std::get<0>(ordering);
    auto ordering_y = std::unique_ptr<LabeledEdgeGraph<Pair<Int>>>(std::get<1>(ordering));
    free(std::get<2>(ordering));
    free(std::get<3>(ordering));
    auto classes_y_ordering = make_topological_ordering_smart(*ordering_y, *classes_y,
        are_classes_in_conflict_y, old_positions, get_y_coordinate);
    NodesPositions* new_positions = new NodesPositions();
    int current_position_y = 0;
    for (auto& classes_id : classes_y_ordering) {
        for (auto& class_id : classes_id)
            for (auto& node : classes_y->get_elems(class_id)) {
                new_positions->set_position_x(node, old_positions.get_position_x(node));
                new_positions->set_position_y(node, current_position_y);
            }
        ++current_position_y;
    }
    return new_positions;
}