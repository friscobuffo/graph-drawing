#include "orthogonal/area_compacter.hpp"

#include <cmath>

#include "orthogonal/equivalence_classes.hpp"

auto build_coordinate_x_to_nodes(const Graph& graph,
                                 const NodesPositions& positions) {
  std::unordered_map<int, std::unordered_set<int>> coordinate_x_to_nodes;
  for (const auto& node : graph.get_nodes()) {
    int node_id = node.get_id();
    int x = std::round(100.0 * positions.get_position_x(node_id));
    coordinate_x_to_nodes[x].insert(node_id);
  }
  return coordinate_x_to_nodes;
}

auto build_coordinate_y_to_nodes(const Graph& graph,
                                 const NodesPositions& positions) {
  std::unordered_map<int, std::unordered_set<int>> coordinate_y_to_nodes;
  for (const auto& node : graph.get_nodes()) {
    int node_id = node.get_id();
    int y = std::round(100.0 * positions.get_position_y(node_id));
    coordinate_y_to_nodes[y].insert(node_id);
  }
  return coordinate_y_to_nodes;
}

auto build_index_to_nodes_map_x(const Graph& graph,
                                const NodesPositions& positions) {
  auto coordinate_x_to_nodes = build_coordinate_x_to_nodes(graph, positions);
  int actual_x = 0;
  int x_index = 0;
  std::unordered_map<int, std::unordered_set<int>> index_to_nodes;
  std::unordered_map<int, int> nodes_to_index;
  while (coordinate_x_to_nodes.contains(actual_x)) {
    for (int node_id : coordinate_x_to_nodes[actual_x]) {
      index_to_nodes[x_index].insert(node_id);
      nodes_to_index[node_id] = x_index;
    }
    if (coordinate_x_to_nodes.contains(actual_x + 5)) {
      actual_x += 5;
    } else {
      actual_x += 100;
      ++x_index;
    }
  }
  return std::make_pair(index_to_nodes, nodes_to_index);
}

auto build_index_to_nodes_map_y(const Graph& graph,
                                const NodesPositions& positions) {
  auto coordinate_y_to_nodes = build_coordinate_y_to_nodes(graph, positions);
  int actual_y = 0;
  int y_index = 0;
  std::unordered_map<int, std::unordered_set<int>> index_to_nodes;
  std::unordered_map<int, int> nodes_to_index;
  while (coordinate_y_to_nodes.contains(actual_y)) {
    for (int node_id : coordinate_y_to_nodes[actual_y]) {
      index_to_nodes[y_index].insert(node_id);
      nodes_to_index[node_id] = y_index;
    }
    if (coordinate_y_to_nodes.contains(actual_y + 5)) {
      actual_y += 5;
    } else {
      actual_y += 100;
      ++y_index;
    }
  }
  return std::make_pair(index_to_nodes, nodes_to_index);
}

bool can_move_to_prev_index(IntPairHashSet& prev, IntPairHashSet& to_shift) {
  if (to_shift.size() != 1) throw std::runtime_error("can_move_left: wtf");
  auto [to_shift_min, to_shift_max] = *to_shift.begin();
  for (auto [prev_min, prev_max] : prev)
    if (!(prev_min > to_shift_max || to_shift_min > prev_max)) return false;
  return true;
}

int compute_shift_amount(
    int index,
    std::unordered_map<int, IntPairHashSet>& index_to_min_max_coordinate) {
  int shift = 0;
  IntPairHashSet& to_shift = index_to_min_max_coordinate[index];
  while (true) {
    if (index - shift == 0) return shift;
    IntPairHashSet& prev = index_to_min_max_coordinate[index - shift - 1];
    if (can_move_to_prev_index(prev, to_shift))
      shift++;
    else
      break;
  }
  return shift;
}

auto build_index_x_to_min_max_index_y(
    std::unordered_map<int, std::unordered_set<int>>& index_x_to_nodes,
    const NodesPositions& positions,
    std::unordered_map<int, int>& node_to_index_y) {
  std::unordered_map<int, IntPairHashSet> index_to_min_max_y;
  for (const auto& [index, nodes] : index_x_to_nodes) {
    int min_y = INT_MAX;
    int max_y = 0;
    for (int node_id : nodes) {
      int y = node_to_index_y[node_id];
      min_y = std::min(min_y, y);
      max_y = std::max(max_y, y);
    }
    index_to_min_max_y[index] = {{min_y, max_y}};
  }
  return index_to_min_max_y;
}

auto build_index_y_to_min_max_index_x(
    std::unordered_map<int, std::unordered_set<int>>& index_to_nodes,
    const NodesPositions& positions,
    std::unordered_map<int, int>& node_to_index_x) {
  std::unordered_map<int, IntPairHashSet> index_to_min_max_x;
  for (const auto& [index, nodes] : index_to_nodes) {
    int min_x = INT_MAX;
    int max_x = 0;
    for (int node_id : nodes) {
      int x = node_to_index_x[node_id];
      min_x = std::min(min_x, x);
      max_x = std::max(max_x, x);
    }
    index_to_min_max_x[index] = {{min_x, max_x}};
  }
  return index_to_min_max_x;
}

void compact_area(const Graph& graph, const Shape& shape,
                  NodesPositions& positions,
                  const GraphAttributes& attributes) {
  auto [index_x_to_nodes, nodes_to_index_x] =
      build_index_to_nodes_map_x(graph, positions);
  auto [index_y_to_nodes, nodes_to_index_y] =
      build_index_to_nodes_map_y(graph, positions);
  // compacting x
  auto index_to_min_max_y = build_index_x_to_min_max_index_y(
      index_x_to_nodes, positions, nodes_to_index_y);
  int index = 0;
  while (index_to_min_max_y.contains(index + 1)) {
    ++index;
    int shift_amount = compute_shift_amount(index, index_to_min_max_y);
    if (shift_amount == 0) {
      continue;
    }
    std::unordered_set<int>& nodes_to_shift = index_x_to_nodes[index];
    for (int node_id : nodes_to_shift) {
      float old_x = positions.get_position_x(node_id);
      positions.change_position_x(node_id, old_x - shift_amount);
    }
    index_to_min_max_y[index - shift_amount].insert(
        *index_to_min_max_y[index].begin());
    index_to_min_max_y[index].clear();
  }
  // compacting y
  auto index_to_min_max_x = build_index_y_to_min_max_index_x(
      index_y_to_nodes, positions, nodes_to_index_x);
  index = 0;
  while (index_to_min_max_x.contains(index + 1)) {
    ++index;
    int shift_amount = compute_shift_amount(index, index_to_min_max_x);
    if (shift_amount == 0) continue;
    std::unordered_set<int>& nodes_to_shift = index_y_to_nodes[index];
    for (int node_id : nodes_to_shift) {
      float old_y = positions.get_position_y(node_id);
      positions.change_position_y(node_id, old_y - shift_amount);
    }
    index_to_min_max_x[index - shift_amount].insert(
        *index_to_min_max_x[index].begin());
    index_to_min_max_x[index].clear();
  }
}