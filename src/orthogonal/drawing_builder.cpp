#include "orthogonal/drawing_builder.hpp"

#include <algorithm>
#include <cfloat>
#include <functional>
#include <list>
#include <ranges>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include "core/utils.hpp"

class EquivalenceClasses {
 private:
  std::unordered_map<int, int> m_elem_to_class;
  std::unordered_map<int, std::unordered_set<int>> m_class_to_elems;
  bool has_class(int class_id) const {
    return m_class_to_elems.contains(class_id);
  }

 public:
  void set_class(int elem, int class_id) {
    if (has_elem_a_class(elem))
      throw std::runtime_error(
          "EquivalenceClasses::set_class elem already has an assigned class");
    m_elem_to_class[elem] = class_id;
    m_class_to_elems[class_id].insert(elem);
  }
  bool has_elem_a_class(int elem) const {
    return m_elem_to_class.contains(elem);
  }
  int get_class_of_elem(int elem) const {
    if (!has_elem_a_class(elem))
      throw std::runtime_error(
          "EquivalenceClasses::get_class elem does not have a class");
    return m_elem_to_class.at(elem);
  }
  const auto& get_elems_of_class(int class_id) const {
    if (!has_class(class_id))
      throw std::runtime_error(
          "EquivalenceClasses::get_elems class does not exist");
    return m_class_to_elems.at(class_id);
  }
  std::string to_string() const {
    std::string result = "EquivalenceClasses:\n";
    for (const auto& [elem, class_id] : m_elem_to_class)
      result += std::to_string(elem) + " -> " + std::to_string(class_id) + "\n";
    return result;
  }
  void print() const { std::cout << to_string() << std::endl; }
  auto get_all_classes() const {
    return m_class_to_elems |
           std::views::transform(
               [](const auto& pair) -> int { return pair.first; });
  }
};

void directional_node_expander(
    const Shape& shape, const Graph& graph, const GraphNode& node, int class_id,
    EquivalenceClasses& equivalence_classes,
    std::function<bool(const Shape&, int, int)> is_direction_wrong) {
  int node_id = node.get_id();
  equivalence_classes.set_class(node_id, class_id);
  for (auto& edge : node.get_edges()) {
    int neighbor_id = edge.get_to().get_id();
    if (equivalence_classes.has_elem_a_class(neighbor_id)) continue;
    if (is_direction_wrong(shape, node_id, neighbor_id)) continue;
    directional_node_expander(shape, graph, edge.get_to(), class_id,
                              equivalence_classes, is_direction_wrong);
  }
}

void horizontal_node_expander(const Shape& shape, const Graph& graph,
                              const GraphNode& node, int class_id,
                              GraphEdgeHashSet& is_edge_visited,
                              EquivalenceClasses& equivalence_classes) {
  auto is_direction_wrong = [](const Shape& shape, int i, int j) {
    return shape.is_vertical(i, j);
  };
  directional_node_expander(shape, graph, node, class_id, equivalence_classes,
                            is_direction_wrong);
}

void vertical_node_expander(const Shape& shape, const Graph& graph,
                            const GraphNode& node, int class_id,
                            GraphEdgeHashSet& is_edge_visited,
                            EquivalenceClasses& equivalence_classes) {
  auto is_direction_wrong = [](const Shape& shape, int i, int j) {
    return shape.is_horizontal(i, j);
  };
  directional_node_expander(shape, graph, node, class_id, equivalence_classes,
                            is_direction_wrong);
}

const auto build_equivalence_classes(const Shape& shape, const Graph& graph) {
  EquivalenceClasses equivalence_classes_x;
  EquivalenceClasses equivalence_classes_y;
  int next_class_x = 0;
  int next_class_y = 0;
  GraphEdgeHashSet is_edge_visited;
  for (auto& node : graph.get_nodes()) {
    if (!equivalence_classes_y.has_elem_a_class(node.get_id()))
      horizontal_node_expander(shape, graph, node, next_class_y++,
                               is_edge_visited, equivalence_classes_y);
    if (!equivalence_classes_x.has_elem_a_class(node.get_id()))
      vertical_node_expander(shape, graph, node, next_class_x++,
                             is_edge_visited, equivalence_classes_x);
  }
  for (const auto& node : graph.get_nodes()) {
    if (!equivalence_classes_x.has_elem_a_class(node.get_id()))
      equivalence_classes_x.set_class(node.get_id(), next_class_x++);
    if (!equivalence_classes_y.has_elem_a_class(node.get_id()))
      equivalence_classes_y.set_class(node.get_id(), next_class_y++);
  }
  return std::make_pair(std::move(equivalence_classes_x),
                        std::move(equivalence_classes_y));
}

auto equivalence_classes_to_ordering(
    const EquivalenceClasses& equivalence_classes_x,
    const EquivalenceClasses& equivalence_classes_y, const Graph& graph,
    const Shape& shape) {
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
        if (ordering_x->has_edge(node_class_x, neighbor_class_x)) continue;
        if (node_class_x == neighbor_class_x) continue;
        auto& e = ordering_x->add_edge(node_class_x, neighbor_class_x);
        ordering_x_edge_to_graph_edge.set_edge_any_label(e.get_id(),
                                                         std::make_pair(i, j));
      } else if (shape.is_up(i, j)) {
        int node_class_y = equivalence_classes_y.get_class_of_elem(i);
        int neighbor_class_y = equivalence_classes_y.get_class_of_elem(j);
        if (ordering_y->has_edge(node_class_y, neighbor_class_y)) continue;
        if (node_class_y == neighbor_class_y) continue;
        auto& e = ordering_y->add_edge(node_class_y, neighbor_class_y);
        ordering_y_edge_to_graph_edge.set_edge_any_label(e.get_id(),
                                                         std::make_pair(i, j));
      }
    }
  }
  return std::make_tuple(std::move(ordering_x), std::move(ordering_y),
                         std::move(ordering_x_edge_to_graph_edge),
                         std::move(ordering_y_edge_to_graph_edge));
}

void NodesPositions::change_position(int node, float position_x,
                                     float position_y) {
  if (!has_position(node))
    throw std::runtime_error(
        "NodesPositions::change_position Node does not have a position");
  m_nodeid_to_position_map.insert_or_assign(
      node, NodePosition(position_x, position_y));
}

void NodesPositions::set_position(int node, float position_x,
                                  float position_y) {
  if (has_position(node))
    throw std::runtime_error(
        "NodesPositions::set_position_x Node already has a position");
  m_nodeid_to_position_map.insert({node, NodePosition(position_x, position_y)});
}

float NodesPositions::get_position_x(int node) const {
  if (!has_position(node))
    throw std::runtime_error(
        "NodesPositions::get_position_x Node does not have a position");
  return m_nodeid_to_position_map.at(node).m_x;
}

float NodesPositions::get_position_y(int node) const {
  if (!has_position(node))
    throw std::runtime_error(
        "NodesPositions::get_position_y Node does not have a position");
  return m_nodeid_to_position_map.at(node).m_y;
}

const NodePosition& NodesPositions::get_position(int node) const {
  if (!has_position(node))
    throw std::runtime_error(
        "NodesPositions::get_position Node does not have a position");
  return m_nodeid_to_position_map.at(node);
}

bool NodesPositions::has_position(int node) const {
  return m_nodeid_to_position_map.contains(node);
}

void NodesPositions::remove_position(int node) {
  if (!has_position(node))
    throw std::runtime_error(
        "NodesPositions::remove_position Node does not have a position");
  m_nodeid_to_position_map.erase(node);
}

std::vector<int> path_in_class(const Graph& graph, int from, int to,
                               const Shape& shape, bool go_horizontal) {
  std::vector<int> path;
  std::unordered_set<int> visited;
  std::function<void(int)> dfs = [&](int current) {
    if (current == to) {
      path.push_back(current);
      return;
    }
    visited.insert(current);
    for (const auto& edge : graph.get_node_by_id(current).get_edges()) {
      int neighbor = edge.get_to().get_id();
      if (visited.contains(neighbor)) continue;
      if (go_horizontal == shape.is_horizontal(current, neighbor)) {
        dfs(neighbor);
        if (!path.empty()) {
          path.push_back(current);
          return;
        }
      }
    }
    visited.erase(current);
  };
  dfs(from);
  std::reverse(path.begin(), path.end());
  return path;
}

std::vector<int> build_cycle_in_graph_from_cycle_in_ordering(
    const Graph& graph, const Shape& shape,
    const std::vector<int>& cycle_in_ordering, const Graph& ordering,
    const EquivalenceClasses& equivalence_classes,
    const GraphAttributes& ordering_edge_to_graph_edge, bool go_horizontal) {
  std::vector<int> cycle;
  for (int i = 0; i < cycle_in_ordering.size(); ++i) {
    int class_id = cycle_in_ordering[i];
    int next_class_id = cycle_in_ordering[(i + 1) % cycle_in_ordering.size()];
    auto& edge = ordering.get_edge(class_id, next_class_id);
    const std::any& edge_label =
        ordering_edge_to_graph_edge.get_edge_any_label(edge.get_id());
    int from = std::any_cast<std::pair<int, int>>(edge_label).first;
    int to = std::any_cast<std::pair<int, int>>(edge_label).second;
    cycle.push_back(from);
    int next_next_class_id =
        cycle_in_ordering[(i + 2) % cycle_in_ordering.size()];
    auto& next_edge = ordering.get_edge(next_class_id, next_next_class_id);
    const std::any& next_edge_label =
        ordering_edge_to_graph_edge.get_edge_any_label(next_edge.get_id());
    int next_from = std::any_cast<std::pair<int, int>>(next_edge_label).first;
    if (to != next_from) {
      auto path = path_in_class(graph, to, next_from, shape, go_horizontal);
      for (int i = 0; i < path.size() - 1; ++i) cycle.push_back(path[i]);
    }
  }
  return cycle;
}

BuildingResult build_nodes_positions(const Shape& shape, const Graph& graph) {
  auto classes = build_equivalence_classes(shape, graph);
  auto& classes_x = classes.first;
  auto& classes_y = classes.second;
  auto ordering =
      equivalence_classes_to_ordering(classes_x, classes_y, graph, shape);
  auto& ordering_x = std::get<0>(ordering);
  auto& ordering_y = std::get<1>(ordering);
  auto& ordering_x_edge_to_graph_edge = std::get<2>(ordering);
  auto& ordering_y_edge_to_graph_edge = std::get<3>(ordering);
  auto cycle_x = find_a_cycle_directed_graph(*ordering_x);
  auto cycle_y = find_a_cycle_directed_graph(*ordering_y);
  if (cycle_x.has_value() || cycle_y.has_value()) {
    std::vector<std::vector<int>> cycles_to_be_added;
    if (cycle_x.has_value()) {
      auto cycle_x_in_original_graph =
          build_cycle_in_graph_from_cycle_in_ordering(
              graph, shape, cycle_x.value(), *ordering_x, classes_x,
              ordering_x_edge_to_graph_edge, false);
      cycles_to_be_added.push_back(cycle_x_in_original_graph);
    } else {
      auto cycle_y_in_original_graph =
          build_cycle_in_graph_from_cycle_in_ordering(
              graph, shape, cycle_y.value(), *ordering_y, classes_y,
              ordering_y_edge_to_graph_edge, true);
      cycles_to_be_added.push_back(cycle_y_in_original_graph);
    }
    return BuildingResult{std::nullopt, cycles_to_be_added,
                          BuildingResultType::CYCLES_TO_BE_ADDED};
  }
  auto classes_x_ordering = make_topological_ordering(*ordering_x);
  auto classes_y_ordering = make_topological_ordering(*ordering_y);
  float current_position_x = 0;
  std::unordered_map<int, float> node_id_to_position_x;
  for (auto& class_id : classes_x_ordering) {
    for (auto& node : classes_x.get_elems_of_class(class_id))
      node_id_to_position_x[node] = current_position_x;
    ++current_position_x;
  }
  float current_position_y = 0;
  std::unordered_map<int, float> node_id_to_position_y;
  for (auto& class_id : classes_y_ordering) {
    for (auto& node : classes_y.get_elems_of_class(class_id))
      node_id_to_position_y[node] = current_position_y;
    ++current_position_y;
  }
  NodesPositions positions;
  for (int node_id : graph.get_nodes_ids()) {
    float x = node_id_to_position_x[node_id];
    float y = node_id_to_position_y[node_id];
    positions.set_position(node_id, x, y);
  }
  return BuildingResult{std::move(positions), {}, BuildingResultType::OK};
}

void node_positions_to_svg(const NodesPositions& positions, const Graph& graph,
                           const GraphAttributes& attributes,
                           const std::string& filename) {
  float max_x = 0;
  float max_y = 0;
  for (auto& node : graph.get_nodes()) {
    max_x = std::max(max_x, positions.get_position_x(node.get_id()));
    max_y = std::max(max_y, positions.get_position_y(node.get_id()));
  }
  SvgDrawer drawer{800, 600};
  ScaleLinear scale_x = ScaleLinear(0, max_x + 2, 0, 800);
  ScaleLinear scale_y = ScaleLinear(0, max_y + 2, 0, 600);
  std::unordered_map<int, Point2D> points;
  for (auto& node : graph.get_nodes()) {
    double x = scale_x.map(positions.get_position_x(node.get_id()) + 1);
    double y = scale_y.map(positions.get_position_y(node.get_id()) + 1);
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
    if (color == Color::RED || color == Color::BLUE) continue;
    drawer.add(points.at(node.get_id()), color_to_string(color),
               std::to_string(node.get_id()));
  }
  drawer.saveToFile(filename);
}

bool do_edges_cross(const NodesPositions& positions, int i, int j, int k,
                    int l) {
  float i_pos_x = positions.get_position_x(i);
  float i_pos_y = positions.get_position_y(i);
  float j_pos_x = positions.get_position_x(j);
  float j_pos_y = positions.get_position_y(j);
  float k_pos_x = positions.get_position_x(k);
  float k_pos_y = positions.get_position_y(k);
  float l_pos_x = positions.get_position_x(l);
  float l_pos_y = positions.get_position_y(l);

  auto same_position = [](float ax, float ay, float bx, float by) {
    return ax == bx && ay == by;
  };

  if (same_position(i_pos_x, i_pos_y, k_pos_x, k_pos_y) ||
      same_position(i_pos_x, i_pos_y, l_pos_x, l_pos_y) ||
      same_position(j_pos_x, j_pos_y, k_pos_x, k_pos_y) ||
      same_position(j_pos_x, j_pos_y, l_pos_x, l_pos_y)) {
    return false;
  }

  bool is_i_j_horizontal = i_pos_y == j_pos_y;
  bool is_k_l_horizontal = k_pos_y == l_pos_y;

  if (is_i_j_horizontal && is_k_l_horizontal) {
    return (i_pos_y == k_pos_y) &&
           ((i_pos_x <= k_pos_x && j_pos_x >= k_pos_x) ||
            (i_pos_x <= l_pos_x && j_pos_x >= l_pos_x) ||
            (j_pos_x <= k_pos_x && i_pos_x >= k_pos_x) ||
            (j_pos_x <= l_pos_x && i_pos_x >= l_pos_x));
  }
  if (!is_i_j_horizontal && !is_k_l_horizontal) {
    return (i_pos_x == k_pos_x) &&
           ((i_pos_y <= k_pos_y && j_pos_y >= k_pos_y) ||
            (i_pos_y <= l_pos_y && j_pos_y >= l_pos_y) ||
            (j_pos_y <= k_pos_y && i_pos_y >= k_pos_y) ||
            (j_pos_y <= l_pos_y && i_pos_y >= l_pos_y));
  }
  if (!is_i_j_horizontal) return do_edges_cross(positions, k, l, i, j);
  if (k_pos_x < std::min(i_pos_x, j_pos_x) ||
      k_pos_x > std::max(i_pos_x, j_pos_x))
    return false;
  if (i_pos_y < std::min(k_pos_y, l_pos_y) ||
      i_pos_y > std::max(k_pos_y, l_pos_y))
    return false;
  return true;
}

// removes useless corners from the graph and from the shape
// (useless corners are red nodes with two horizontal or vertical edges)
void refine_result(Graph& graph, GraphAttributes& attributes,
                   NodesPositions& positions, Shape& shape) {
  std::vector<int> nodes_to_remove;
  for (auto& node : graph.get_nodes()) {
    int i = node.get_id();
    if (attributes.get_node_color(i) == Color::BLACK) continue;
    std::vector<const GraphEdge*> edges;
    for (auto& edge : node.get_edges()) edges.push_back(&edge);
    int j_1 = edges[0]->get_to().get_id();
    int j_2 = edges[1]->get_to().get_id();
    // if the added corner is flat, remove it
    if (shape.is_horizontal(i, j_1) == shape.is_horizontal(i, j_2))
      nodes_to_remove.push_back(i);
  }
  for (int i : nodes_to_remove) {
    const auto& node = graph.get_node_by_id(i);
    std::vector<const GraphEdge*> edges;
    for (auto& edge : node.get_edges()) edges.push_back(&edge);
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

float get_x_coordinate(int node, const NodesPositions& positions) {
  return positions.get_position_x(node);
}

float get_y_coordinate(int node, const NodesPositions& positions) {
  return positions.get_position_y(node);
}

template <typename Func>
bool are_classes_in_conflict(EquivalenceClasses& classes, int class_id_1,
                             int class_id_2, const NodesPositions& positions,
                             Func get_position) {
  float min_1 = FLT_MAX;
  float max_1 = 0;
  for (auto& node : classes.get_elems_of_class(class_id_1)) {
    min_1 = std::min(min_1, get_position(node, positions));
    max_1 = std::max(max_1, get_position(node, positions));
  }
  float min_2 = FLT_MAX;
  float max_2 = 0;
  for (auto& node : classes.get_elems_of_class(class_id_2)) {
    min_2 = std::min(min_2, get_position(node, positions));
    max_2 = std::max(max_2, get_position(node, positions));
  }
  if (min_1 > max_2 || min_2 > max_1) return false;
  return true;
}

bool are_classes_in_conflict_x(EquivalenceClasses& classes, int class_id_1,
                               int class_id_2,
                               const NodesPositions& positions) {
  return are_classes_in_conflict(classes, class_id_1, class_id_2, positions,
                                 get_x_coordinate);
}

bool are_classes_in_conflict_y(EquivalenceClasses& classes, int class_id_1,
                               int class_id_2,
                               const NodesPositions& positions) {
  return are_classes_in_conflict(classes, class_id_1, class_id_2, positions,
                                 get_y_coordinate);
}

bool can_move_left(int class_id,
                   const std::unordered_map<int, std::unordered_set<int>>&
                       coordinate_to_classes,
                   int actual_coordinate, const NodesPositions& positions,
                   EquivalenceClasses& classes_x) {
  if (actual_coordinate == 0) return false;
  if (coordinate_to_classes.at(actual_coordinate - 1).empty()) return true;
  for (int other_class_id : coordinate_to_classes.at(actual_coordinate - 1))
    if (are_classes_in_conflict_y(classes_x, class_id, other_class_id,
                                  positions))
      return false;
  return true;
}

bool can_move_down(int class_id,
                   const std::unordered_map<int, std::unordered_set<int>>&
                       coordinate_to_classes,
                   int actual_coordinate, const NodesPositions& positions,
                   EquivalenceClasses& classes_y) {
  if (actual_coordinate == 0) return false;
  if (coordinate_to_classes.at(actual_coordinate - 1).empty()) return true;
  for (int other_class_id : coordinate_to_classes.at(actual_coordinate - 1))
    if (are_classes_in_conflict_x(classes_y, class_id, other_class_id,
                                  positions))
      return false;
  return true;
}

NodesPositions compact_area_x(const Graph& graph, const Shape& shape,
                              const NodesPositions& old_positions) {
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
    while (can_move_left(class_id, coordinate_to_classes, actual_coordinate,
                         old_positions, classes_x)) {
      coordinate_to_classes[actual_coordinate - 1].insert(class_id);
      coordinate_to_classes[actual_coordinate].erase(class_id);
      actual_coordinate--;
    }
  }
  NodesPositions new_positions;
  for (auto& [coordinate, classes] : coordinate_to_classes)
    for (auto& class_id : classes)
      for (auto& node_id : classes_x.get_elems_of_class(class_id))
        new_positions.set_position(node_id, coordinate,
                                   old_positions.get_position_y(node_id));
  return std::move(new_positions);
}

NodesPositions compact_area_y(const Graph& graph, const Shape& shape,
                              const NodesPositions& old_positions) {
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
    while (can_move_down(class_id, coordinate_to_classes, actual_coordinate,
                         old_positions, classes_y)) {
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
            node_id, old_positions.get_position_x(node_id), coordinate);
  return std::move(new_positions);
}

bool check_if_drawing_has_overlappings(const Graph& graph,
                                       const NodesPositions& positions) {
  // node - node overlappings
  for (int id : graph.get_nodes_ids()) {
    for (int other_id : graph.get_nodes_ids()) {
      if (id >= other_id) continue;
      if (positions.get_position(id) == positions.get_position(other_id)) {
        std::cout << "Node " << id << " overlaps with node " << other_id
                  << std::endl;
        return true;
      }
    }
  }
  // node - edge overlappings
  for (int id : graph.get_nodes_ids()) {
    float i_x = positions.get_position_x(id);
    float i_y = positions.get_position_y(id);
    for (const auto& edge : graph.get_edges()) {
      float j_1 = edge.get_from().get_id();
      float j_2 = edge.get_to().get_id();
      if (j_1 == id || j_2 == id) continue;
      float j_1_x = positions.get_position_x(j_1);
      float j_1_y = positions.get_position_y(j_1);
      float j_2_x = positions.get_position_x(j_2);
      float j_2_y = positions.get_position_y(j_2);
      if (j_1_y == j_2_y) {  // horizontal edge
        if (i_y == j_1_y && i_x >= std::min(j_1_x, j_2_x) &&
            i_x <= std::max(j_1_x, j_2_x)) {
          std::cout << "Node " << id << " overlaps with edge " << j_1 << "-"
                    << j_2 << std::endl;
          return true;
        }
      } else {  // vertical edge
        if (i_x == j_1_x && i_y >= std::min(j_1_y, j_2_y) &&
            i_y <= std::max(j_1_y, j_2_y)) {
          std::cout << "Node " << id << " overlaps with edge " << j_1 << "-"
                    << j_2 << std::endl;
          return true;
        }
      }
    }
  }
  return false;
}

DrawingResult make_orthogonal_drawing_incremental(
    const Graph& graph, std::vector<std::vector<int>>& cycles) {
  if (!is_graph_undirected(graph))
    throw std::runtime_error(
        "make_orthogonal_drawing_incremental: graph is not undirected");
  if (!is_graph_connected(graph))
    throw std::runtime_error(
        "make_orthogonal_drawing_incremental: graph is not connected");
  auto augmented_graph = std::make_unique<Graph>();
  GraphAttributes attributes;
  attributes.add_attribute(Attribute::NODES_COLOR);
  for (const auto& node : graph.get_nodes()) {
    augmented_graph->add_node(node.get_id());
    attributes.set_node_color(node.get_id(), Color::BLACK);
  }
  for (const auto& node : graph.get_nodes()) {
    if (node.get_degree() > 4)
      throw std::runtime_error(
          "make_orthogonal_drawing_incremental: found node with degree > 4");
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
  return {std::move(augmented_graph),
          std::move(attributes),
          std::move(shape),
          std::move(positions),
          (int)cycles.size() - number_of_added_cycles,
          number_of_added_cycles,
          number_of_useless_bends};
}

DrawingResult make_orthogonal_drawing_any_degree(const Graph& graph);

DrawingResult make_orthogonal_drawing_low_degree(const Graph& graph) {
  auto cycles = compute_cycle_basis(graph);
  return make_orthogonal_drawing_incremental(graph, cycles);
}

DrawingResult make_orthogonal_drawing(const Graph& graph) {
  for (const auto& node : graph.get_nodes())
    if (node.get_degree() > 4) return make_orthogonal_drawing_any_degree(graph);
  return make_orthogonal_drawing_low_degree(graph);
}

std::pair<std::unique_ptr<Graph>, GraphEdgeHashSet>
compute_maximal_degree_4_subgraph(const Graph& graph) {
  auto subgraph = std::make_unique<Graph>();
  GraphEdgeHashSet removed_edges;
  for (const auto& node : graph.get_nodes()) subgraph->add_node(node.get_id());
  for (const auto& node : graph.get_nodes()) {
    int node_id = node.get_id();
    for (auto& edge : node.get_edges()) {
      int neighbor_id = edge.get_to().get_id();
      if (subgraph->has_edge(node_id, neighbor_id)) continue;
      if (removed_edges.contains({node_id, neighbor_id})) continue;
      if (subgraph->get_node_by_id(node_id).get_degree() < 4 &&
          subgraph->get_node_by_id(neighbor_id).get_degree() < 4) {
        subgraph->add_undirected_edge(node_id, neighbor_id);
      } else {
        removed_edges.insert({node_id, neighbor_id});
        removed_edges.insert({neighbor_id, node_id});
      }
    }
  }
  return std::make_pair(std::move(subgraph), std::move(removed_edges));
}

DrawingResult merge_connected_components(std::vector<DrawingResult>& results);

void create_set_positions(std::set<int>& x_position_set,
                          std::set<int>& y_position_set, Graph& graph,
                          NodesPositions& positions);

void all_positive_positions(Graph& graph, NodesPositions& positions);

int make_chain_key(int x, int y) { return (x << 16) ^ y; }

std::tuple<float, float, float, float> shift_by_epsilon_factor(
    float x_j, float x_i, float y_j, float y_i, int z, float& from_y,
    float& to_y, float epsilon, int last_index, float& from_x, float& to_x) {
  // 1 quadrant
  if (x_j >= x_i && y_j >= y_i && z == 0) {
    if (from_x == to_x) {
      from_y += epsilon;
      from_x += epsilon;
      to_x += epsilon;
    } else if (from_y == to_y) {
      from_x += epsilon;
      from_y += epsilon;
      to_y += epsilon;
    }
  }
  // 3 quadrant
  else if (x_j <= x_i && y_j <= y_i && z == last_index) {
    if (from_x == to_x) {
      to_y += epsilon;
      from_x += epsilon;
      to_x += epsilon;
    } else if (from_y == to_y) {
      to_x += epsilon;
      from_y += epsilon;
      to_y += epsilon;
    }
  }
  // 2 quadrant
  else if (x_j <= x_i && y_j >= y_i && z == 0) {
    if (from_x == to_x) {
      from_y += epsilon;
      from_x -= epsilon;
      to_x -= epsilon;
    } else if (from_y == to_y) {
      from_x -= epsilon;
      from_y += epsilon;
      to_y += epsilon;
    }
  }

  // 4 quadrant
  else if (x_j >= x_i && y_j <= y_i && z == last_index) {
    if (from_x == to_x) {
      to_y += epsilon;
      from_x -= epsilon;
      to_x -= epsilon;
    } else if (from_y == to_y) {
      to_x -= epsilon;
      from_y += epsilon;
      to_y += epsilon;
    }
  }

  //   3 quadrant
  else if (x_j <= x_i && y_j <= y_i && z == 0) {
    if (from_x == to_x) {
      from_y -= epsilon;
      from_x -= epsilon;
      to_x -= epsilon;
    } else if (from_y == to_y) {
      from_x -= epsilon;
      from_y -= epsilon;
      to_y -= epsilon;
    }
  }

  // 1 quadrant
  else if (x_j >= x_i && y_j >= y_i && z == last_index) {
    if (from_x == to_x) {
      to_y -= epsilon;
      from_x -= epsilon;
      to_x -= epsilon;
    } else if (from_y == to_y) {
      to_x -= epsilon;
      from_y -= epsilon;
      to_y -= epsilon;
    }
  }

  //    4 quadrant
  else if (x_j >= x_i && y_j <= y_i && z == 0) {
    if (from_x == to_x) {
      from_y -= epsilon;
      from_x += epsilon;
      to_x += epsilon;
    } else if (from_y == to_y) {
      from_x += epsilon;
      from_y -= epsilon;
      to_y -= epsilon;
    }
  }

  else if (x_j <= x_i && y_j >= y_i && z == last_index) {
    if (from_x == to_x) {
      to_y -= epsilon;
      from_x += epsilon;
      to_x += epsilon;
    } else if (from_y == to_y) {
      to_x += epsilon;
      from_y -= epsilon;
      to_y -= epsilon;
    }
  }
  return std::make_tuple(from_x, to_x, from_y, to_y);
}

void shift_edges(GraphAttributes& attributes, int i, int j,
                 NodesPositions& positions, float x_j, float x_i, float y_j,
                 float y_i) {
  std::vector<std::tuple<int, int>> list =
      attributes.get_chain_edges(make_chain_key(i, j));

  float epsilon = 0.1;
  for (int z = 0; z < list.size(); z++) {
    if (z == 0 || z == list.size() - 1) {
      int from = std::get<0>(list[z]);
      int to = std::get<1>(list[z]);
      float from_x = positions.get_position_x(from),
            from_y = positions.get_position_y(from);
      float to_x = positions.get_position_x(to),
            to_y = positions.get_position_y(to);
      auto shifted_positions =
          shift_by_epsilon_factor(x_j, x_i, y_j, y_i, z, from_y, to_y, epsilon,
                                  list.size() - 1, from_x, to_x);
      from_x = std::get<0>(shifted_positions);
      to_x = std::get<1>(shifted_positions);
      from_y = std::get<2>(shifted_positions);
      to_y = std::get<3>(shifted_positions);
      positions.change_position(from, from_x, from_y);
      positions.change_position(to, to_x, to_y);
    }
  }
}

DrawingResult make_orthogonal_drawing_any_degree(const Graph& graph) {
  auto [subgraph, removed_edges] = compute_maximal_degree_4_subgraph(graph);
  auto components = compute_connected_components(*subgraph);
  std::vector<DrawingResult> results;
  for (auto& component : components)
    results.push_back(
        std::move(make_orthogonal_drawing_low_degree(*component)));
  DrawingResult result = merge_connected_components(results);
  for (auto& edge : removed_edges) {
    if (edge.first > edge.second) continue;
    if (edge.first < edge.second) add_back_removed_edge(result, edge);
  }

  for (auto& edge : removed_edges) {
    if (edge.first > edge.second) continue;
    float x_i = result.positions.get_position_x(edge.first);
    float y_i = result.positions.get_position_y(edge.first);
    float x_j = result.positions.get_position_x(edge.second);
    float y_j = result.positions.get_position_y(edge.second);
    shift_edges(result.attributes, edge.first, edge.second, result.positions,
                x_j, x_i, y_j, y_i);
  }

  return result;
}

DrawingResult merge_connected_components(std::vector<DrawingResult>& results) {
  if (results.size() != 1)
    throw std::runtime_error(
        "merge_connected_components: not really implemented");
  return std::move(results[0]);
}

void NodesPositions::x_right_shift(float x_pos) {
  for (auto& entry : m_nodeid_to_position_map)
    if (entry.second.m_x >= x_pos) entry.second.m_x++;
}

void NodesPositions::x_left_shift(float x_pos) {
  for (auto& entry : m_nodeid_to_position_map)
    if (entry.second.m_x <= x_pos) entry.second.m_x--;
}

void NodesPositions::y_up_shift(float y_pos) {
  for (auto& entry : m_nodeid_to_position_map)
    if (entry.second.m_y >= y_pos) entry.second.m_y++;
}

void NodesPositions::y_down_shift(float y_pos) {
  for (auto& entry : m_nodeid_to_position_map)
    if (entry.second.m_y <= y_pos) entry.second.m_y--;
}

void add_colored_node(Graph& graph, GraphAttributes& attributes, int& node_id,
                      Color color) {
  node_id = graph.add_node().get_id();
  attributes.set_node_color(node_id, color);
}

void set_chain_and_edge(Graph& graph, GraphAttributes& attributes, int i, int j,
                        int u, int v) {
  attributes.set_chain_edges(make_chain_key(i, j), std::make_tuple(u, v));
  graph.add_undirected_edge(u, v);
}

void split_and_rewire(int i, int j, Direction direction_ia,
                      Direction direction_ab, bool x_true, bool y_true,
                      bool aligned, Graph& graph, GraphAttributes& attributes,
                      NodesPositions& positions) {
  int n0 = -1, n1 = -1, n2 = -1, n3 = -1, n4 = -1;
  add_colored_node(graph, attributes, n0, Color::BLUE);
  add_colored_node(graph, attributes, n1, Color::RED);
  add_colored_node(graph, attributes, n2, Color::RED);
  add_colored_node(graph, attributes, n3, Color::BLUE);

  set_chain_and_edge(graph, attributes, i, j, n0, n1);
  set_chain_and_edge(graph, attributes, i, j, n1, n2);
  set_chain_and_edge(graph, attributes, i, j, n2, n3);

  if (!x_true && !y_true && !aligned) {
    add_colored_node(graph, attributes, n4, Color::RED);
    set_chain_and_edge(graph, attributes, i, j, n3, n4);
  }

  float i_x = positions.get_position_x(i);
  float i_y = positions.get_position_y(i);
  float j_x = positions.get_position_x(j);
  float j_y = positions.get_position_y(j);

  float n1_x = i_x, n1_y = i_y;
  float n2_x = j_x, n2_y = j_y;
  float n3_x = j_x, n3_y = j_y;

  auto shift_x_left = [&](float x) { positions.x_left_shift(x); };
  auto shift_x_right = [&](float x) { positions.x_right_shift(x); };
  auto shift_y_up = [&](float y) { positions.y_up_shift(y); };
  auto shift_y_down = [&](float y) { positions.y_down_shift(y); };

  if (!aligned) {
    switch (direction_ia) {
      case Direction::LEFT:
        n2_x = i_x;
        n2_y = j_y;
        shift_x_right(i_x);
        if (direction_ab == Direction::UP)
          shift_y_up(j_y);
        else
          shift_y_down(j_y);
        break;

      case Direction::UP:
        n2_x = j_x;
        n2_y = i_y;
        if (direction_ab == Direction::RIGHT) {
          shift_x_right(j_x);
          shift_y_down(i_y);
        } else {
          shift_x_left(j_x);
          shift_y_down(i_y);
        }
        break;

      case Direction::RIGHT:
        n2_x = i_x;
        n2_y = j_y;
        shift_x_left(i_x);
        if (direction_ab == Direction::DOWN)
          shift_y_down(j_y);
        else
          shift_y_up(j_y);
        break;

      case Direction::DOWN:
        n2_x = j_x;
        n2_y = i_y;
        shift_y_up(i_y);
        if (direction_ab == Direction::LEFT)
          shift_x_left(j_x);
        else
          shift_x_right(j_x);
        break;
    }
  } else {
    switch (direction_ia) {
      case Direction::UP:
        shift_y_down(i_y);
        break;
      case Direction::RIGHT:
        shift_x_left(i_x);
        break;
      default:
        break;
    }
  }

  i_x = positions.get_position_x(i);
  i_y = positions.get_position_y(i);
  j_x = positions.get_position_x(j);
  j_y = positions.get_position_y(j);

  positions.set_position(n0, i_x, i_y);
  positions.set_position(n1, n1_x, n1_y);

  if (aligned) {
    positions.set_position(n2, n2_x, n2_y);
    positions.set_position(n3, j_x, j_y);
  } else if (y_true) {
    positions.set_position(n2, j_x, n2_y);
    positions.set_position(n3, j_x, j_y);
  } else if (x_true) {
    positions.set_position(n2, n2_x, j_y);
    positions.set_position(n3, j_x, j_y);
  } else {
    positions.set_position(n2, n2_x, n2_y);
    positions.set_position(n3, n3_x, n3_y);
    positions.set_position(n4, j_x, j_y);
  }
}

// assume that coor_i < coor_j
bool check_if_the_segment_is_free(int coor_i, int coor_j,
                                  const std::set<int>& position_set) {
  for (int c = coor_i + 1; c < coor_j; c++)
    if (position_set.find(c) != position_set.end()) return false;
  return true;
}

void add_back_removed_edge(DrawingResult& result,
                           const std::pair<int, int>& edge) {
  auto& graph = *result.augmented_graph;
  auto& attributes = result.attributes;
  auto& positions = result.positions;
  int node_count = graph.size();

  std::set<int> x_position_set, y_position_set;
  create_set_positions(x_position_set, y_position_set, graph, positions);

  int i = edge.first;
  int j = edge.second;
  if (i > j) std::swap(i, j);

  float x_i = positions.get_position_x(i), y_i = positions.get_position_y(i);
  float x_j = positions.get_position_x(j), y_j = positions.get_position_y(j);
  if (x_i > x_j && y_i > y_j) {
    if (check_if_the_segment_is_free(x_j, x_i, x_position_set))
      split_and_rewire(i, j, Direction::LEFT, Direction::DOWN, true, false,
                       false, graph, attributes, positions);
    else if (check_if_the_segment_is_free(y_j, y_i, y_position_set))
      split_and_rewire(i, j, Direction::DOWN, Direction::LEFT, false, true,
                       false, graph, attributes, positions);
    else
      split_and_rewire(i, j, Direction::DOWN, Direction::LEFT, false, false,
                       false, graph, attributes, positions);
  } else if (x_i < x_j && y_i < y_j) {
    if (check_if_the_segment_is_free(x_i, x_j, x_position_set))
      split_and_rewire(i, j, Direction::RIGHT, Direction::UP, true, false,
                       false, graph, attributes, positions);
    else if (check_if_the_segment_is_free(y_i, y_j, y_position_set))
      split_and_rewire(i, j, Direction::UP, Direction::RIGHT, false, true,
                       false, graph, attributes, positions);
    else
      split_and_rewire(i, j, Direction::UP, Direction::RIGHT, false, false,
                       false, graph, attributes, positions);
  } else if (x_i > x_j && y_i < y_j) {
    if (check_if_the_segment_is_free(x_j, x_i, x_position_set))
      split_and_rewire(i, j, Direction::LEFT, Direction::UP, true, false, false,
                       graph, attributes, positions);
    else if (check_if_the_segment_is_free(y_i, y_j, y_position_set))
      split_and_rewire(i, j, Direction::UP, Direction::LEFT, false, true, false,
                       graph, attributes, positions);
    else
      split_and_rewire(i, j, Direction::LEFT, Direction::UP, false, false,
                       false, graph, attributes, positions);
  } else if (x_i < x_j && y_i > y_j) {
    if (check_if_the_segment_is_free(x_i, x_j, x_position_set))
      split_and_rewire(i, j, Direction::RIGHT, Direction::DOWN, true, false,
                       false, graph, attributes, positions);
    else if (check_if_the_segment_is_free(y_j, y_i, y_position_set))
      split_and_rewire(i, j, Direction::DOWN, Direction::RIGHT, false, true,
                       false, graph, attributes, positions);
    else
      split_and_rewire(i, j, Direction::RIGHT, Direction::DOWN, false, false,
                       false, graph, attributes, positions);
  } else if (y_i == y_j && x_i < x_j)
    split_and_rewire(i, j, Direction::UP, Direction::RIGHT, false, false, true,
                     graph, attributes, positions);
  else if (y_i == y_j && x_i > x_j)
    split_and_rewire(i, j, Direction::UP, Direction::LEFT, false, false, true,
                     graph, attributes, positions);

  else if (x_i == x_j && y_i < y_j)
    split_and_rewire(i, j, Direction::RIGHT, Direction::UP, false, false, true,
                     graph, attributes, positions);
  else if (x_i == x_j && y_i > y_j)
    split_and_rewire(i, j, Direction::RIGHT, Direction::DOWN, false, false,
                     true, graph, attributes, positions);

  x_i = positions.get_position_x(i), y_i = positions.get_position_y(i);
  x_j = positions.get_position_x(j), y_j = positions.get_position_y(j);

  all_positive_positions(graph, positions);
}

void create_set_positions(std::set<int>& x_position_set,
                          std::set<int>& y_position_set, Graph& graph,
                          NodesPositions& positions) {
  for (auto& node : graph.get_nodes()) {
    int i = node.get_id();
    int x = positions.get_position_x(i);
    int y = positions.get_position_y(i);
    x_position_set.insert(x);
    y_position_set.insert(y);
  }
}

void all_positive_positions(Graph& graph, NodesPositions& positions) {
  float min_x = FLT_MAX;
  float min_y = FLT_MAX;
  for (auto& node : graph.get_nodes()) {
    int i = node.get_id();
    min_x = std::min(min_x, positions.get_position_x(i));
    min_y = std::min(min_y, positions.get_position_y(i));
  }
  for (auto& node : graph.get_nodes()) {
    int i = node.get_id();
    positions.change_position(i, positions.get_position_x(i) - min_x,
                              positions.get_position_y(i) - min_y);
  }
}

DrawingResult make_orthogonal_drawing_incremental_special(
    const Graph& graph, std::vector<std::vector<int>>& cycles);

DrawingResult make_orthogonal_drawing_sperimental(const Graph& graph) {
  bool has_low_degree = true;
  for (const auto& node : graph.get_nodes())
    if (node.get_degree() > 4) {
      has_low_degree = false;
      break;
    }
  if (has_low_degree) return make_orthogonal_drawing_low_degree(graph);
  // auto cycles = compute_cycle_basis(graph);
  auto cycles = compute_all_cycles_in_undirected_graph(graph);
  return make_orthogonal_drawing_incremental_special(graph, cycles);
}

BuildingResult build_nodes_positions_special(const Shape& shape,
                                             const Graph& graph);

DrawingResult make_orthogonal_drawing_incremental_special(
    const Graph& graph, std::vector<std::vector<int>>& cycles) {
  if (!is_graph_undirected(graph))
    throw std::runtime_error(
        "make_orthogonal_drawing_incremental: graph is not undirected");
  if (!is_graph_connected(graph))
    throw std::runtime_error(
        "make_orthogonal_drawing_incremental: graph is not connected");
  auto augmented_graph = std::make_unique<Graph>();
  GraphAttributes attributes;
  attributes.add_attribute(Attribute::NODES_COLOR);
  for (const auto& node : graph.get_nodes()) {
    augmented_graph->add_node(node.get_id());
    attributes.set_node_color(node.get_id(), Color::BLACK);
  }
  for (const auto& node : graph.get_nodes())
    for (auto& edge : node.get_edges())
      augmented_graph->add_edge(node.get_id(), edge.get_to().get_id());
  Shape shape = build_shape_special(*augmented_graph, attributes, cycles);
  BuildingResult result = build_nodes_positions(shape, *augmented_graph);
  int number_of_added_cycles = 0;
  while (result.type == BuildingResultType::CYCLES_TO_BE_ADDED) {
    for (auto& cycle_to_add : result.cycles_to_be_added)
      cycles.push_back(cycle_to_add);
    number_of_added_cycles += result.cycles_to_be_added.size();
    shape = build_shape_special(*augmented_graph, attributes, cycles);
    result = build_nodes_positions(shape, *augmented_graph);
  }
  NodesPositions positions(std::move(result.positions.value()));
  int old_size = augmented_graph->size();
  // refine_result(*augmented_graph, attributes, positions, shape);
  int number_of_useless_bends = old_size - augmented_graph->size();
  result = build_nodes_positions(shape, *augmented_graph);
  positions = std::move(result.positions.value());
  // auto new_positions = compact_area_x(*augmented_graph, shape, positions);
  // positions = std::move(new_positions);
  // new_positions = compact_area_y(*augmented_graph, shape, positions);
  // positions = std::move(new_positions);
  return {
      std::move(augmented_graph),
      std::move(attributes),
      std::move(shape),
      std::move(positions),
      (int)cycles.size() - number_of_added_cycles,
      number_of_added_cycles,
      number_of_useless_bends,
  };
}
