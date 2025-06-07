#include "orthogonal/drawing_builder.hpp"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <functional>
#include <list>
#include <queue>
#include <ranges>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include "core/utils.hpp"
#include "orthogonal/area_compacter.hpp"
#include "orthogonal/equivalence_classes.hpp"

void NodesPositions::change_position(int node, float position_x,
                                     float position_y) {
  if (!has_position(node))
    throw std::runtime_error(
        "NodesPositions::change_position Node does not have a position");
  m_nodeid_to_position_map.insert_or_assign(
      node, NodePosition(position_x, position_y));
}

void NodesPositions::change_position_x(int node, float position_x) {
  if (!has_position(node))
    throw std::runtime_error(
        "NodesPositions::change_position Node does not have a position");
  auto& position = m_nodeid_to_position_map.at(node);
  position.m_x = position_x;
}

void NodesPositions::change_position_y(int node, float position_y) {
  if (!has_position(node))
    throw std::runtime_error(
        "NodesPositions::change_position_y Node does not have a position");
  auto& position = m_nodeid_to_position_map.at(node);
  position.m_y = position_y;
}

void NodesPositions::set_position(int node, float position_x,
                                  float position_y) {
  if (has_position(node))
    throw std::runtime_error(
        "NodesPositions::set_position_x Node already has a position");
  m_nodeid_to_position_map.insert({node, NodePosition(position_x, position_y)});
}

float NodesPositions::get_position_x(int node) const {
  if (!has_position(node)) {
    std::cout << node << std::endl;
    throw std::runtime_error(
        "NodesPositions::get_position_x Node does not have a position");
  }
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

void node_positions_to_svg(const NodesPositions& positions, const Graph& graph,
                           const GraphAttributes& attributes,
                           const std::string& filename) {
  float max_x = 0.0f;
  float max_y = 0.0f;
  for (auto& node : graph.get_nodes()) {
    max_x = std::max(max_x, positions.get_position_x(node.get_id()));
    max_y = std::max(max_y, positions.get_position_y(node.get_id()));
  }
  const int width = static_cast<int>(1 + (max_x + 2) * 130);
  const int height = static_cast<int>(1 + (max_y + 2) * 130);
  SvgDrawer drawer{width, height};
  ScaleLinear scale_x = ScaleLinear(0, max_x + 2, 0, width);
  ScaleLinear scale_y = ScaleLinear(0, max_y + 2, 0, height);
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
    if (color == Color::RED) continue;
    if (color == Color::GREEN) continue;
    if (color == Color::BLUE) continue;
    if (color == Color::RED_SPECIAL) continue;
    int side;
    if (node.get_degree() <= 4)
      side = 25;
    else
      side = ceil(25 * sqrt((node.get_degree() - 3)));
    Square2D square{points.at(node.get_id()), side};
    square.setColor(color_to_string(color));
    square.setLabel(std::to_string(node.get_id()));
    drawer.add(square);
  }
  drawer.save_to_file(filename);
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
void refine_result(Graph& graph, GraphAttributes& attributes, Shape& shape) {
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
  }
}

bool check_if_drawing_has_overlappings(const Graph& graph,
                                       const NodesPositions& positions) {
  // node - node overlappings
  for (int id : graph.get_nodes_ids()) {
    for (int other_id : graph.get_nodes_ids()) {
      if (id >= other_id) continue;
      if (positions.get_position(id) == positions.get_position(other_id))
        return true;
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
            i_x <= std::max(j_1_x, j_2_x))
          return true;
      } else {  // vertical edge
        if (i_x == j_1_x && i_y >= std::min(j_1_y, j_2_y) &&
            i_y <= std::max(j_1_y, j_2_y))
          return true;
      }
    }
  }
  return false;
}

DrawingResult make_orthogonal_drawing_any_degree(const Graph& graph);

DrawingResult make_orthogonal_drawing(const Graph& graph) {
  for (const auto& node : graph.get_nodes())
    if (node.get_degree() > 4) return make_orthogonal_drawing_any_degree(graph);
  return make_orthogonal_drawing_sperimental(graph);
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
        std::move(make_orthogonal_drawing_sperimental(*component)));
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

DrawingResult make_orthogonal_drawing_incremental(
    const Graph& graph, std::vector<std::vector<int>>& cycles);

DrawingResult make_orthogonal_drawing_sperimental(const Graph& graph) {
  auto cycles = compute_cycle_basis(graph);
  // auto cycles = compute_all_cycles_in_undirected_graph(graph);
  return make_orthogonal_drawing_incremental(graph, cycles);
}

std::optional<std::vector<int>> check_if_metrics_exist(
    Shape& shape, Graph& graph, GraphAttributes& attributes) {
  auto [classes_x, classes_y] = build_equivalence_classes(shape, graph);
  auto [ordering_x, ordering_y, ordering_x_edge_to_graph_edge,
        ordering_y_edge_to_graph_edge] =
      equivalence_classes_to_ordering(classes_x, classes_y, graph, shape);
  auto cycle_x = find_a_cycle_directed_graph(*ordering_x);
  auto cycle_y = find_a_cycle_directed_graph(*ordering_y);
  if (cycle_x.has_value()) {
    auto cycle_x_in_original_graph =
        build_cycle_in_graph_from_cycle_in_ordering(
            graph, shape, cycle_x.value(), *ordering_x, classes_x,
            ordering_x_edge_to_graph_edge, false);
    return cycle_x_in_original_graph;
  }
  if (cycle_y.has_value()) {
    auto cycle_y_in_original_graph =
        build_cycle_in_graph_from_cycle_in_ordering(
            graph, shape, cycle_y.value(), *ordering_y, classes_y,
            ordering_y_edge_to_graph_edge, true);
    return cycle_y_in_original_graph;
  }
  return std::nullopt;
}

void add_green_blue_nodes(Graph& graph, GraphAttributes& attributes,
                          Shape& shape);

NodesPositions build_nodes_positions(Graph& graph, GraphAttributes& attributes,
                                     Shape& shape);

void make_shifts(Graph& graph, GraphAttributes& attributes, Shape& shape,
                 NodesPositions& positions);

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
  for (const auto& node : graph.get_nodes())
    for (auto& edge : node.get_edges())
      augmented_graph->add_edge(node.get_id(), edge.get_to().get_id());
  Shape shape = build_shape(*augmented_graph, attributes, cycles);
  std::optional<std::vector<int>> cycle_to_add =
      check_if_metrics_exist(shape, *augmented_graph, attributes);
  int number_of_added_cycles = 0;
  while (cycle_to_add.has_value()) {
    cycles.push_back(cycle_to_add.value());
    number_of_added_cycles++;
    shape = build_shape(*augmented_graph, attributes, cycles);
    cycle_to_add = check_if_metrics_exist(shape, *augmented_graph, attributes);
  }
  int old_size = augmented_graph->size();
  refine_result(*augmented_graph, attributes, shape);
  int number_of_useless_bends = old_size - augmented_graph->size();
  add_green_blue_nodes(*augmented_graph, attributes, shape);
  NodesPositions positions =
      build_nodes_positions(*augmented_graph, attributes, shape);
  // compact_area(*augmented_graph, shape, positions, attributes);
  make_shifts(*augmented_graph, attributes, shape, positions);
  return {std::move(augmented_graph),
          std::move(attributes),
          std::move(shape),
          std::move(positions),
          (int)cycles.size() - number_of_added_cycles,
          number_of_added_cycles,
          number_of_useless_bends};
}

void fix_useless_green_blue_nodes(Graph& graph, GraphAttributes& attributes,
                                  Shape& shape, NodesPositions& positions);

void add_special_edge(
    Graph& ordering, int class_1, int class_2, int node_1, int node_2,
    GraphEdgeHashMap<std::pair<int, int>>& added_to_original_edge) {
  if (!ordering.has_node(class_1)) ordering.add_node(class_1);
  if (!ordering.has_node(class_2)) ordering.add_node(class_2);
  if (!ordering.has_edge(class_1, class_2)) {
    ordering.add_edge(class_1, class_2);
    added_to_original_edge[{class_1, class_2}] = {node_1, node_2};
  }
}

auto add_special_edges_in_orderings(const Graph& graph, Graph& ordering_x,
                                    Graph& ordering_y,
                                    const GraphAttributes& attributes,
                                    const Shape& shape,
                                    const EquivalenceClasses& classes_x,
                                    const EquivalenceClasses& classes_y) {
  GraphEdgeHashMap<std::pair<int, int>> added_to_original_edge_x;
  GraphEdgeHashMap<std::pair<int, int>> added_to_original_edge_y;
  for (const GraphNode& node : graph.get_nodes()) {
    if (node.get_degree() <= 4) continue;
    int node_id = node.get_id();
    std::optional<int> black_up_node_id;
    std::optional<int> black_right_node_id;
    std::unordered_set<int> green_up_nodes;
    std::unordered_set<int> red_up_nodes;
    std::unordered_set<int> red_special_up_nodes;
    std::unordered_set<int> blue_right_nodes;
    std::unordered_set<int> red_right_nodes;
    std::unordered_set<int> red_special_right_nodes;
    for (const GraphEdge& edge : node.get_edges()) {
      int neighbor_id = edge.get_to().get_id();
      Direction direction = shape.get_direction(node_id, neighbor_id);
      Color color = attributes.get_node_color(neighbor_id);
      switch (direction) {
        case Direction::UP:
          if (color == Color::GREEN)
            green_up_nodes.insert(neighbor_id);
          else if (color == Color::RED)
            red_up_nodes.insert(neighbor_id);
          else if (color == Color::RED_SPECIAL)
            red_special_up_nodes.insert(neighbor_id);
          else {
            if (black_up_node_id.has_value())
              throw std::runtime_error(
                  "make_orthogonal_drawing_incremental: multiple black up "
                  "nodes");
            black_up_node_id = neighbor_id;
          }
          break;
        case Direction::RIGHT:
          if (color == Color::BLUE)
            blue_right_nodes.insert(neighbor_id);
          else if (color == Color::RED)
            red_right_nodes.insert(neighbor_id);
          else if (color == Color::RED_SPECIAL)
            red_special_right_nodes.insert(neighbor_id);
          else {
            if (black_right_node_id.has_value())
              throw std::runtime_error(
                  "make_orthogonal_drawing_incremental: multiple black right "
                  "nodes");
            black_right_node_id = neighbor_id;
          }
        default:
          break;
      }
    }
    // RIGHT
    if (black_right_node_id.has_value()) {
      int black_class =
          classes_x.get_class_of_elem(black_right_node_id.value());
      // adding from blue to black
      for (int blue_id : blue_right_nodes) {
        int blue_class = classes_x.get_class_of_elem(blue_id);
        add_special_edge(ordering_x, blue_class, black_class, node_id, blue_id,
                         added_to_original_edge_x);
      }
      // adding from red_special to black
      for (int red_special_id : red_special_right_nodes) {
        int red_special_class = classes_x.get_class_of_elem(red_special_id);
        add_special_edge(ordering_x, red_special_class, black_class, node_id,
                         red_special_id, added_to_original_edge_x);
      }
    }
    for (int blue_id : blue_right_nodes) {
      int blue_class = classes_x.get_class_of_elem(blue_id);
      // adding from blue to red
      for (int red_id : red_right_nodes) {
        int red_class = classes_x.get_class_of_elem(red_id);
        add_special_edge(ordering_x, blue_class, red_class, node_id, blue_id,
                         added_to_original_edge_x);
      }
      // adding from blue to red_special
      for (int red_id : red_special_right_nodes) {
        int red_class = classes_x.get_class_of_elem(red_id);
        add_special_edge(ordering_x, blue_class, red_class, node_id, blue_id,
                         added_to_original_edge_x);
      }
    }
    for (int red_special_id : red_special_right_nodes) {
      int red_special_class = classes_x.get_class_of_elem(red_special_id);
      // adding from red_special to red
      for (int red_id : red_right_nodes) {
        int red_class = classes_x.get_class_of_elem(red_id);
        add_special_edge(ordering_x, red_special_class, red_class, node_id,
                         red_id, added_to_original_edge_x);
      }
    }
    // UP
    if (black_up_node_id.has_value()) {
      int black_class = classes_y.get_class_of_elem(black_up_node_id.value());
      // adding from green to black
      for (int green_id : green_up_nodes) {
        int green_class = classes_y.get_class_of_elem(green_id);
        add_special_edge(ordering_y, green_class, black_class, node_id,
                         green_id, added_to_original_edge_y);
      }
      // adding from red_special to black
      for (int red_special_id : red_special_up_nodes) {
        int red_special_class = classes_y.get_class_of_elem(red_special_id);
        add_special_edge(ordering_y, red_special_class, black_class, node_id,
                         red_special_id, added_to_original_edge_y);
      }
    }
    for (int green_id : green_up_nodes) {
      int green_class = classes_y.get_class_of_elem(green_id);
      // adding from green to red
      for (int red_id : red_up_nodes) {
        int red_class = classes_y.get_class_of_elem(red_id);
        add_special_edge(ordering_y, green_class, red_class, node_id, green_id,
                         added_to_original_edge_y);
      }
      // adding from green to red_special
      for (int red_special_id : red_special_up_nodes) {
        int red_special_class = classes_y.get_class_of_elem(red_special_id);
        add_special_edge(ordering_y, green_class, red_special_class, node_id,
                         green_id, added_to_original_edge_y);
      }
    }
    for (int red_special_id : red_special_up_nodes) {
      int red_special_class = classes_y.get_class_of_elem(red_special_id);
      // adding from red_special to red
      for (int red_id : red_up_nodes) {
        int red_class = classes_y.get_class_of_elem(red_id);
        add_special_edge(ordering_y, red_special_class, red_class, node_id,
                         red_special_id, added_to_original_edge_y);
      }
    }
  }
  return std::make_pair(std::move(added_to_original_edge_x),
                        std::move(added_to_original_edge_y));
}

auto neighbors_at_each_direction(const GraphNode& node, const Shape& shape,
                                 const GraphAttributes& attributes) {
  std::unordered_map<Direction, std::vector<int>> nodes_at_direction;
  for (const GraphEdge& edge : node.get_edges()) {
    const GraphNode& neighbor = edge.get_to();
    int neighbor_id = neighbor.get_id();
    Direction dir = shape.get_direction(node.get_id(), neighbor_id);
    nodes_at_direction[dir].push_back(neighbor_id);
  }
  return nodes_at_direction;
}

int get_other_neighbor_id(const Graph& graph, int node_id, int neighbor_id) {
  for (const GraphEdge& edge : graph.get_node_by_id(node_id).get_edges()) {
    if (edge.get_to().get_id() != neighbor_id) {
      return edge.get_to().get_id();
    }
  }
  throw std::runtime_error("No other neighbor found for node " +
                           std::to_string(node_id));
}

std::unordered_map<int, Direction> compute_shifting_direction(
    const Graph& graph, const GraphAttributes& attributes, const Shape& shape,
    std::vector<int>& nodes_at_direction, Direction increasing_direction,
    int& index_of_fixed_node) {
  bool has_black_node = false;
  int black_node_index = -1;
  std::unordered_map<int, Direction> node_to_direction;
  for (int i = 0; i < nodes_at_direction.size(); ++i) {
    int node_id = nodes_at_direction[i];
    if (attributes.get_node_color(node_id) == Color::BLACK) {
      has_black_node = true;
      black_node_index = i;
      break;
    }
  }
  if (has_black_node) {
    Direction direction = opposite_direction(increasing_direction);
    for (int i = 0; i < nodes_at_direction.size(); ++i) {
      if (i == black_node_index) {
        direction = increasing_direction;
        continue;
      }
      node_to_direction[nodes_at_direction[i]] = direction;
    }
    index_of_fixed_node = black_node_index;
    return node_to_direction;
  }
  int red_node_index = -1;
  for (int i = 0; i < nodes_at_direction.size(); ++i) {
    int node_id = nodes_at_direction[i];
    if (attributes.get_node_color(node_id) == Color::RED) {
      red_node_index = i;
      break;
    }
  }
  if (red_node_index == -1) {
    return {};
  }
  Direction direction = opposite_direction(increasing_direction);
  for (int i = 0; i < nodes_at_direction.size(); ++i) {
    if (i == red_node_index) {
      direction = increasing_direction;
      continue;
    }
    node_to_direction[nodes_at_direction[i]] = direction;
  }
  index_of_fixed_node = red_node_index;
  return node_to_direction;
}

template <typename Func>
void shifting_order(int node_id, Graph& graph, Shape& shape,
                    std::vector<int>& nodes_at_direction,
                    const NodesPositions& positions,
                    GraphAttributes& attributes,
                    const Direction increasing_direction, Func get_position) {
  const Direction decreasing_direction =
      opposite_direction(increasing_direction);
  std::sort(
      nodes_at_direction.begin(), nodes_at_direction.end(), [&](int a, int b) {
        if (attributes.get_node_color(a) == Color::BLACK) {
          int b_other_neighbor_id = get_other_neighbor_id(graph, b, node_id);
          return shape.get_direction(b, b_other_neighbor_id) ==
                 increasing_direction;
        }
        if (attributes.get_node_color(b) == Color::BLACK) {
          int a_other_neighbor_id = get_other_neighbor_id(graph, a, node_id);
          return shape.get_direction(a, a_other_neighbor_id) ==
                 decreasing_direction;
        }
        int a_other_neighbor = get_other_neighbor_id(graph, a, node_id);
        int b_other_neighbor = get_other_neighbor_id(graph, b, node_id);
        if (shape.get_direction(a, a_other_neighbor) == increasing_direction &&
            shape.get_direction(b, b_other_neighbor) == decreasing_direction) {
          return false;
        }
        if (shape.get_direction(a, a_other_neighbor) == decreasing_direction &&
            shape.get_direction(b, b_other_neighbor) == increasing_direction) {
          return true;
        }
        if (shape.get_direction(a, a_other_neighbor) == increasing_direction &&
            shape.get_direction(b, b_other_neighbor) == increasing_direction) {
          return (get_position(positions, a) > get_position(positions, b));
        }
        return (get_position(positions, a) < get_position(positions, b));
      });
}

void make_shifts(int node_id, Graph& graph, Shape& shape,
                 GraphAttributes& attributes, NodesPositions& positions,
                 std::vector<int>& right_nodes, Direction nodes_direction,
                 Color new_color, Direction increasing_direction) {
  shifting_order(node_id, graph, shape, right_nodes, positions, attributes,
                 increasing_direction,
                 [](const NodesPositions& positions, int id) {
                   return positions.get_position_x(id);
                 });
  int index_of_fixed_node = -1;
  std::unordered_map<int, Direction> node_to_direction =
      compute_shifting_direction(graph, attributes, shape, right_nodes,
                                 increasing_direction, index_of_fixed_node);
  float up_initial_position = positions.get_position_y(node_id);
  for (int i = 0; i < right_nodes.size(); ++i) {
    int node_to_shift_id = right_nodes[i];
    if (!node_to_direction.contains(node_to_shift_id)) continue;
    float shift = (i - index_of_fixed_node) * 0.05f;
    if (node_to_direction[node_to_shift_id] == increasing_direction)
      for (const GraphNode& node : graph.get_nodes()) {
        int node_id = node.get_id();
        if (positions.get_position_y(node_id) > up_initial_position + shift) {
          positions.change_position_y(
              node_id, positions.get_position_y(node_id) + 0.05f);
        }
      }
    else if (node_to_direction[node_to_shift_id] ==
             opposite_direction(increasing_direction)) {
      for (const GraphNode& node : graph.get_nodes()) {
        int node_id = node.get_id();
        if (positions.get_position_y(node_id) < up_initial_position + shift) {
          positions.change_position_y(
              node_id, positions.get_position_y(node_id) - 0.05f);
        }
      }
    } else
      throw std::runtime_error("wtf");
    int added_node_id = graph.add_node().get_id();
    attributes.set_node_color(added_node_id, new_color);
    shape.set_direction(node_id, added_node_id,
                        node_to_direction[node_to_shift_id]);
    shape.set_direction(
        added_node_id, node_id,
        opposite_direction(node_to_direction[node_to_shift_id]));
    shape.set_direction(added_node_id, node_to_shift_id, nodes_direction);
    shape.set_direction(node_to_shift_id, added_node_id,
                        opposite_direction(nodes_direction));
    shape.remove_direction(node_id, node_to_shift_id);
    shape.remove_direction(node_to_shift_id, node_id);
    graph.remove_undirected_edge(node_id, node_to_shift_id);
    graph.add_undirected_edge(node_id, added_node_id);
    graph.add_undirected_edge(added_node_id, node_to_shift_id);
    if (increasing_direction == Direction::UP) {
      positions.set_position(added_node_id, positions.get_position_x(node_id),
                             up_initial_position + shift);
      positions.change_position_y(node_to_shift_id,
                                  positions.get_position_y(added_node_id));
    } else if (increasing_direction == Direction::RIGHT) {
      positions.set_position(added_node_id,
                             positions.get_position_x(node_id) + shift,
                             positions.get_position_y(node_id));
      positions.change_position_x(node_to_shift_id,
                                  positions.get_position_x(added_node_id));
    } else
      throw std::runtime_error("make_shifts: wtf");
  }
}

void make_shifts(Graph& graph, GraphAttributes& attributes, Shape& shape,
                 NodesPositions& positions) {
  std::vector<const GraphNode*> nodes;
  for (const GraphNode& node : graph.get_nodes()) {
    if (node.get_degree() > 4) nodes.push_back(&node);
  }
  for (const GraphNode* node : nodes) {
    int node_id = node->get_id();
    std::unordered_map<Direction, std::vector<int>> nodes_to_sort =
        neighbors_at_each_direction(*node, shape, attributes);
    make_shifts(node_id, graph, shape, attributes, positions,
                nodes_to_sort[Direction::RIGHT], Direction::RIGHT, Color::GREEN,
                Direction::UP);
    make_shifts(node_id, graph, shape, attributes, positions,
                nodes_to_sort[Direction::UP], Direction::UP, Color::BLUE,
                Direction::RIGHT);
  }
}

void add_green_blue_nodes(Graph& graph, GraphAttributes& attributes,
                          Shape& shape) {
  std::vector<const GraphNode*> nodes;
  for (const GraphNode& node : graph.get_nodes()) {
    if (node.get_degree() > 4) nodes.push_back(&node);
  }
  for (const GraphNode* node : nodes) {
    int node_id = node->get_id();
    std::vector<std::pair<int, int>> edges_to_remove;
    std::vector<std::pair<int, int>> edges_to_add;
    for (const GraphEdge& edge : node->get_edges()) {
      const GraphNode& neighbor = edge.get_to();
      int neighbor_id = neighbor.get_id();
      const GraphNode& added = graph.add_node();
      int added_id = added.get_id();
      edges_to_add.push_back({added_id, node_id});
      edges_to_add.push_back({added_id, neighbor_id});
      shape.set_direction(added_id, neighbor_id,
                          shape.get_direction(node_id, neighbor_id));
      shape.set_direction(neighbor_id, added_id,
                          shape.get_direction(neighbor_id, node_id));
      if (shape.is_horizontal(node_id, neighbor_id)) {
        attributes.set_node_color(added_id, Color::GREEN);
        shape.set_direction(node_id, added_id, Direction::UP);
        shape.set_direction(added_id, node_id, Direction::DOWN);
      } else {
        attributes.set_node_color(added_id, Color::BLUE);
        shape.set_direction(node_id, added_id, Direction::RIGHT);
        shape.set_direction(added_id, node_id, Direction::LEFT);
      }
      shape.remove_direction(node_id, neighbor_id);
      shape.remove_direction(neighbor_id, node_id);
      edges_to_remove.push_back({node_id, neighbor_id});
    }
    for (auto [from_id, to_id] : edges_to_add)
      graph.add_undirected_edge(from_id, to_id);
    for (auto [from_id, to_id] : edges_to_remove)
      graph.remove_undirected_edge(from_id, to_id);
  }
  auto [classes_x, classes_y] = build_equivalence_classes(shape, graph);
  auto ordering =
      equivalence_classes_to_ordering(classes_x, classes_y, graph, shape);
  std::unique_ptr<Graph>& ordering_x = std::get<0>(ordering);
  std::unique_ptr<Graph>& ordering_y = std::get<1>(ordering);
  auto classes_x_ordering = make_topological_ordering(*ordering_x);
  auto classes_y_ordering = make_topological_ordering(*ordering_y);
  float current_position_x = 0.0f;
  std::unordered_map<int, float> node_id_to_position_x;
  for (auto& class_id : classes_x_ordering) {
    for (auto& node : classes_x.get_elems_of_class(class_id))
      node_id_to_position_x[node] = current_position_x;
    ++current_position_x;
  }
  float current_position_y = 0.0f;
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
  fix_useless_green_blue_nodes(graph, attributes, shape, positions);
}

auto find_edges_to_fix(const Graph& graph, const Shape& shape,
                       const NodesPositions& positions) {
  std::unordered_map<int, int> node_to_leftest_up;
  std::unordered_map<int, int> node_to_leftest_down;
  std::unordered_map<int, int> node_to_downest_left;
  std::unordered_map<int, int> node_to_downest_right;
  for (const GraphNode& node : graph.get_nodes()) {
    if (node.get_degree() <= 4) continue;
    int node_id = node.get_id();
    std::optional<int> downest_left = std::nullopt;
    std::optional<int> downest_right = std::nullopt;
    std::optional<int> leftest_up = std::nullopt;
    std::optional<int> leftest_down = std::nullopt;
    for (const GraphEdge& edge : node.get_edges()) {
      const GraphNode& added = edge.get_to();
      int added_id = added.get_id();
      if (shape.is_horizontal(node_id, added_id)) {
        if (shape.is_left(node_id, added_id)) throw std::runtime_error("wtf 0");
        int other_neighbor_id = 0;
        bool found = false;
        for (const GraphEdge& added_edge : added.get_edges()) {
          int neighbor_id = added_edge.get_to().get_id();
          if (neighbor_id == node_id) continue;
          found = true;
          other_neighbor_id = neighbor_id;
        }
        if (!found) throw std::runtime_error("wtf 1");
        if (shape.is_up(added_id, other_neighbor_id)) {
          if (!leftest_up.has_value())
            leftest_up = added_id;
          else if (positions.get_position_x(added_id) <
                   positions.get_position_x(leftest_up.value()))
            leftest_up = added_id;
        } else {
          if (!leftest_down.has_value())
            leftest_down = added_id;
          else if (positions.get_position_x(added_id) <
                   positions.get_position_x(leftest_down.value()))
            leftest_down = added_id;
        }
      } else {
        if (shape.is_down(node_id, added_id)) throw std::runtime_error("wtf 2");
        int other_neighbor_id = 0;
        bool found = false;
        for (const GraphEdge& green_edge : added.get_edges()) {
          int neighbor_id = green_edge.get_to().get_id();
          if (neighbor_id == node_id) continue;
          found = true;
          other_neighbor_id = neighbor_id;
        }
        if (!found) throw std::runtime_error("wtf 3");
        if (shape.is_left(added_id, other_neighbor_id)) {
          if (!downest_left.has_value())
            downest_left = added_id;
          else if (positions.get_position_y(added_id) <
                   positions.get_position_y(downest_left.value()))
            downest_left = added_id;
        } else {
          if (!downest_right.has_value())
            downest_right = added_id;
          else if (positions.get_position_y(added_id) <
                   positions.get_position_y(downest_right.value())) {
            downest_right = added_id;
          }
        }
      }
    }
    node_to_leftest_up[node_id] = leftest_up.value();
    node_to_leftest_down[node_id] = leftest_down.value();
    node_to_downest_left[node_id] = downest_left.value();
    node_to_downest_right[node_id] = downest_right.value();
  }
  return std::make_tuple(
      std::move(node_to_leftest_up), std::move(node_to_leftest_down),
      std::move(node_to_downest_left), std::move(node_to_downest_right));
}

void fix_edge(Graph& graph, int node, int other_node, Shape& shape,
              GraphAttributes& attributes, NodesPositions& positions,
              Direction direction) {
  std::vector<int> neighbors;
  for (const GraphEdge& edge : graph.get_node_by_id(other_node).get_edges())
    neighbors.push_back(edge.get_to().get_id());
  int other_neighbor;
  if (neighbors[0] == node)
    other_neighbor = neighbors[1];
  else
    other_neighbor = neighbors[0];
  graph.remove_node(other_node);
  positions.remove_position(other_node);
  attributes.remove_nodes_attribute(other_node);
  graph.add_undirected_edge(neighbors[0], neighbors[1]);
  shape.remove_direction(node, other_node);
  shape.remove_direction(other_node, node);
  shape.remove_direction(other_node, other_neighbor);
  shape.remove_direction(other_neighbor, other_node);
  shape.set_direction(node, other_neighbor, direction);
  shape.set_direction(other_neighbor, node, opposite_direction(direction));
}

// at the moment, a node with degree > 4 doesnt have all its "ports" used,
// this method takes some of its neighbors and places them in the unused
// "ports"
void fix_useless_green_blue_nodes(Graph& graph, GraphAttributes& attributes,
                                  Shape& shape, NodesPositions& positions) {
  auto [node_to_leftest_up, node_to_leftest_down, node_to_downest_left,
        node_to_downest_right] = find_edges_to_fix(graph, shape, positions);
  for (auto [node, leftest_up] : node_to_leftest_up) {
    fix_edge(graph, node, leftest_up, shape, attributes, positions,
             Direction::UP);
  }
  for (auto [node, leftest_down] : node_to_leftest_down) {
    fix_edge(graph, node, leftest_down, shape, attributes, positions,
             Direction::DOWN);
  }
  for (auto [node, downest_left] : node_to_downest_left) {
    fix_edge(graph, node, downest_left, shape, attributes, positions,
             Direction::LEFT);
  }
  for (auto [node, downest_right] : node_to_downest_right) {
    fix_edge(graph, node, downest_right, shape, attributes, positions,
             Direction::RIGHT);
  }
}

void add_edge_for_internal_loops(
    std::vector<int>& cycle,
    GraphEdgeHashMap<std::pair<int, int>>& added_to_original_edge, Graph& graph,
    Shape& shape, GraphAttributes& attributes, EquivalenceClasses& classes) {
  int class_from_id = cycle[0];
  int class_to_id = cycle[1];
  if (!added_to_original_edge.contains({class_from_id, class_to_id}))
    throw std::runtime_error("Edge not found in added edges");
  auto [from_id, to_id] = added_to_original_edge[{class_from_id, class_to_id}];
  std::cout << "adding mini corner " << from_id << " " << to_id << "\n";
  Direction direction = shape.get_direction(from_id, to_id);
  Direction direction_to_add;
  if (direction == Direction::RIGHT)
    direction_to_add = Direction::UP;
  else if (direction == Direction::UP)
    direction_to_add = Direction::RIGHT;
  else
    throw std::runtime_error("add_edge_for_internal_loops: wtf 4");
  int new_node_id = graph.add_node().get_id();
  graph.remove_undirected_edge(from_id, to_id);
  shape.remove_direction(from_id, to_id);
  shape.remove_direction(to_id, from_id);
  graph.add_undirected_edge(from_id, new_node_id);
  graph.add_undirected_edge(new_node_id, to_id);
  shape.set_direction(from_id, new_node_id, direction_to_add);
  shape.set_direction(new_node_id, from_id,
                      opposite_direction(direction_to_add));
  shape.set_direction(new_node_id, to_id, direction);
  shape.set_direction(to_id, new_node_id, opposite_direction(direction));
  attributes.set_node_color(new_node_id, Color::RED_SPECIAL);
}

void find_inconsistencies(Graph& graph, Shape& shape,
                          GraphAttributes& attributes) {
  auto [classes_x, classes_y] = build_equivalence_classes(shape, graph);
  Graph ordering_x;
  Graph ordering_y;
  auto [added_to_original_edge_x, added_to_original_edge_y] =
      add_special_edges_in_orderings(graph, ordering_x, ordering_y, attributes,
                                     shape, classes_x, classes_y);
  auto cycle_x = find_a_cycle_directed_graph(ordering_x);
  auto cycle_y = find_a_cycle_directed_graph(ordering_y);
  if (cycle_x.has_value() || cycle_y.has_value()) {
    if (cycle_x.has_value()) {
      add_edge_for_internal_loops(cycle_x.value(), added_to_original_edge_x,
                                  graph, shape, attributes, classes_x);
    } else {
      add_edge_for_internal_loops(cycle_y.value(), added_to_original_edge_y,
                                  graph, shape, attributes, classes_y);
    }
    find_inconsistencies(graph, shape, attributes);
  }
}

NodesPositions build_nodes_positions(Graph& graph, GraphAttributes& attributes,
                                     Shape& shape) {
  find_inconsistencies(graph, shape, attributes);
  auto [classes_x, classes_y] = build_equivalence_classes(shape, graph);
  auto [ordering_x, ordering_y, _, _] =
      equivalence_classes_to_ordering(classes_x, classes_y, graph, shape);
  add_special_edges_in_orderings(graph, *ordering_x, *ordering_y, attributes,
                                 shape, classes_x, classes_y);
  auto new_classes_x_ordering = make_topological_ordering(*ordering_x);
  auto new_classes_y_ordering = make_topological_ordering(*ordering_y);
  float current_position_x = -1.0f;
  std::unordered_map<int, float> node_id_to_position_x;
  for (auto& class_id : new_classes_x_ordering) {
    float next_position_x = current_position_x + 1.0f;
    for (auto& node : classes_x.get_elems_of_class(class_id))
      if (attributes.get_node_color(node) == Color::BLUE)
        next_position_x = current_position_x + 1.0f;
    for (auto& node : classes_x.get_elems_of_class(class_id))
      node_id_to_position_x[node] = next_position_x;
    current_position_x = next_position_x;
  }
  float current_position_y = -1.0f;
  std::unordered_map<int, float> node_id_to_position_y;
  for (auto& class_id : new_classes_y_ordering) {
    float next_position_y = current_position_y + 1.0f;
    for (auto& node : classes_y.get_elems_of_class(class_id))
      if (attributes.get_node_color(node) == Color::GREEN)
        next_position_y = current_position_y + 1.0f;
    for (auto& node : classes_y.get_elems_of_class(class_id))
      node_id_to_position_y[node] = next_position_y;
    current_position_y = next_position_y;
  }
  NodesPositions positions;
  for (int node_id : graph.get_nodes_ids()) {
    float x = node_id_to_position_x[node_id];
    float y = node_id_to_position_y[node_id];
    positions.set_position(node_id, x, y);
  }
  return std::move(positions);
}

void prova_special() {
  Graph graph;
  for (int i = 0; i < 19; i++) graph.add_node(i);
  graph.add_undirected_edge(0, 8);
  graph.add_undirected_edge(9, 8);
  graph.add_undirected_edge(10, 9);
  graph.add_undirected_edge(1, 18);
  graph.add_undirected_edge(2, 1);
  graph.add_undirected_edge(2, 11);
  graph.add_undirected_edge(6, 17);
  graph.add_undirected_edge(7, 6);
  graph.add_undirected_edge(7, 12);
  graph.add_undirected_edge(16, 15);
  graph.add_undirected_edge(14, 15);
  graph.add_undirected_edge(14, 13);
  graph.add_undirected_edge(0, 18);
  graph.add_undirected_edge(8, 1);
  graph.add_undirected_edge(9, 2);
  graph.add_undirected_edge(10, 11);
  graph.add_undirected_edge(18, 17);
  graph.add_undirected_edge(3, 1);
  graph.add_undirected_edge(7, 1);
  graph.add_undirected_edge(2, 4);
  graph.add_undirected_edge(2, 5);
  graph.add_undirected_edge(12, 11);
  graph.add_undirected_edge(3, 6);
  graph.add_undirected_edge(4, 6);
  graph.add_undirected_edge(5, 7);
  graph.add_undirected_edge(17, 16);
  graph.add_undirected_edge(6, 15);
  graph.add_undirected_edge(7, 14);
  graph.add_undirected_edge(12, 13);
  Shape shape;
  shape.set_direction(0, 8, Direction::RIGHT);
  shape.set_direction(8, 0, Direction::LEFT);
  shape.set_direction(8, 9, Direction::RIGHT);
  shape.set_direction(9, 8, Direction::LEFT);
  shape.set_direction(9, 10, Direction::RIGHT);
  shape.set_direction(10, 9, Direction::LEFT);
  shape.set_direction(18, 1, Direction::RIGHT);
  shape.set_direction(1, 18, Direction::LEFT);
  shape.set_direction(1, 2, Direction::RIGHT);
  shape.set_direction(2, 1, Direction::LEFT);
  shape.set_direction(2, 11, Direction::RIGHT);
  shape.set_direction(11, 2, Direction::LEFT);
  shape.set_direction(17, 6, Direction::RIGHT);
  shape.set_direction(6, 17, Direction::LEFT);
  shape.set_direction(6, 7, Direction::RIGHT);
  shape.set_direction(7, 6, Direction::LEFT);
  shape.set_direction(7, 12, Direction::RIGHT);
  shape.set_direction(12, 7, Direction::LEFT);
  shape.set_direction(16, 15, Direction::RIGHT);
  shape.set_direction(15, 16, Direction::LEFT);
  shape.set_direction(15, 14, Direction::RIGHT);
  shape.set_direction(14, 15, Direction::LEFT);
  shape.set_direction(14, 13, Direction::RIGHT);
  shape.set_direction(13, 14, Direction::LEFT);

  shape.set_direction(16, 17, Direction::UP);
  shape.set_direction(17, 16, Direction::DOWN);
  shape.set_direction(15, 6, Direction::UP);
  shape.set_direction(6, 15, Direction::DOWN);
  shape.set_direction(14, 7, Direction::UP);
  shape.set_direction(7, 14, Direction::DOWN);
  shape.set_direction(13, 12, Direction::UP);
  shape.set_direction(12, 13, Direction::DOWN);

  shape.set_direction(18, 0, Direction::UP);
  shape.set_direction(0, 18, Direction::DOWN);
  shape.set_direction(1, 8, Direction::UP);
  shape.set_direction(8, 1, Direction::DOWN);
  shape.set_direction(2, 9, Direction::UP);
  shape.set_direction(9, 2, Direction::DOWN);
  shape.set_direction(11, 10, Direction::UP);
  shape.set_direction(10, 11, Direction::DOWN);

  shape.set_direction(17, 18, Direction::UP);
  shape.set_direction(18, 17, Direction::DOWN);
  shape.set_direction(6, 3, Direction::UP);
  shape.set_direction(3, 6, Direction::DOWN);
  shape.set_direction(3, 1, Direction::UP);
  shape.set_direction(1, 3, Direction::DOWN);
  shape.set_direction(6, 4, Direction::UP);
  shape.set_direction(4, 6, Direction::DOWN);
  shape.set_direction(4, 2, Direction::UP);
  shape.set_direction(2, 4, Direction::DOWN);

  shape.set_direction(7, 1, Direction::UP);
  shape.set_direction(1, 7, Direction::DOWN);
  shape.set_direction(7, 5, Direction::UP);
  shape.set_direction(5, 7, Direction::DOWN);
  shape.set_direction(5, 2, Direction::UP);
  shape.set_direction(2, 5, Direction::DOWN);
  shape.set_direction(12, 11, Direction::UP);
  shape.set_direction(11, 12, Direction::DOWN);

  for (const GraphNode& node : graph.get_nodes()) {
    int node_id{node.get_id()};
    for (const GraphEdge& edge : node.get_edges()) {
      int neighbor_id{edge.get_to().get_id()};
      Direction direction{shape.get_direction(node_id, neighbor_id)};
    }
  }
  GraphAttributes attributes;
  attributes.add_attribute(Attribute::NODES_COLOR);
  for (int i = 0; i < 19; ++i) attributes.set_node_color(i, Color::BLACK);
  add_green_blue_nodes(graph, attributes, shape);
  NodesPositions positions = build_nodes_positions(graph, attributes, shape);
  // compact_area(graph, shape, positions, attributes);
  make_shifts(graph, attributes, shape, positions);
  node_positions_to_svg(positions, graph, attributes, "dajetutta.svg");
}