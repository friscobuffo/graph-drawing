#include "orthogonal/drawing_stats.hpp"

#include <cmath>
#include <functional>
#include <tuple>
#include <unordered_set>
#include <vector>

float compute_total_edge_length(const DrawingResult& result) {
  float total_edge_length = 0;
  for (const auto& node : result.augmented_graph->get_nodes())
    for (const auto& edge : node.get_edges()) {
      int neighbor = edge.get_to().get_id();
      float x1 = result.positions.get_position_x(node.get_id());
      float y1 = result.positions.get_position_y(node.get_id());
      float x2 = result.positions.get_position_x(neighbor);
      float y2 = result.positions.get_position_y(neighbor);
      total_edge_length += std::abs(x1 - x2) + std::abs(y1 - y2);
    }
  return total_edge_length / 2;
}

float compute_max_edge_length(const DrawingResult& result) {
  const auto& graph = *result.augmented_graph;
  const auto& attributes = result.attributes;
  const auto& positions = result.positions;
  float max_edge_length = 0;
  std::unordered_set<int> visited;
  for (const auto& node : graph.get_nodes()) {
    if (attributes.get_node_color(node.get_id()) != Color::BLACK) continue;
    std::function<void(int, int, int)> dfs = [&](int current_id, int black_id,
                                                 float current_length) {
      visited.insert(current_id);
      for (const auto& edge : graph.get_node_by_id(current_id).get_edges()) {
        int neighbor = edge.get_to().get_id();
        if (visited.contains(neighbor)) continue;
        float x1 = positions.get_position_x(current_id);
        float y1 = positions.get_position_y(current_id);
        float x2 = positions.get_position_x(neighbor);
        float y2 = positions.get_position_y(neighbor);
        float length = std::abs(x1 - x2) + std::abs(y1 - y2);
        Color neighbor_color = attributes.get_node_color(neighbor);
        if (neighbor_color == Color::RED)
          dfs(neighbor, black_id, current_length + length);
        else if (neighbor_color == Color::BLACK) {
          if (black_id < neighbor) {
            float total_length = current_length + length;
            max_edge_length = std::max(max_edge_length, total_length);
          }
        }
      }
      visited.erase(current_id);
    };
    dfs(node.get_id(), node.get_id(), 0);
  }
  return max_edge_length;
}

double compute_edge_length_std_dev(const DrawingResult& result) {
  const auto& graph = *result.augmented_graph;
  const auto& attributes = result.attributes;
  const auto& positions = result.positions;
  std::vector<int> edge_lengths;
  float total_edge_length = 0;
  std::unordered_set<int> visited;
  for (const auto& node : graph.get_nodes()) {
    if (attributes.get_node_color(node.get_id()) != Color::BLACK) continue;
    std::function<void(int, int, float)> dfs = [&](int current_id, int black_id,
                                                   float current_length) {
      visited.insert(current_id);
      for (const auto& edge : graph.get_node_by_id(current_id).get_edges()) {
        int neighbor = edge.get_to().get_id();
        if (visited.contains(neighbor)) continue;
        float x1 = positions.get_position_x(current_id);
        float y1 = positions.get_position_y(current_id);
        float x2 = positions.get_position_x(neighbor);
        float y2 = positions.get_position_y(neighbor);
        float length = std::abs(x1 - x2) + std::abs(y1 - y2);
        Color neighbor_color = attributes.get_node_color(neighbor);
        if (neighbor_color == Color::RED)
          dfs(neighbor, black_id, current_length + length);
        else if (neighbor_color == Color::BLACK) {
          if (black_id < neighbor) {
            float total_length = current_length + length;
            total_edge_length += total_length;
            edge_lengths.push_back(total_length);
          }
        }
      }
      visited.erase(current_id);
    };
    dfs(node.get_id(), node.get_id(), 0);
  }
  return compute_stddev(edge_lengths);
}

int compute_total_bends(const DrawingResult& result) {
  int total_bends = 0;
  for (const auto& node : result.augmented_graph->get_nodes())
    if (result.attributes.get_node_color(node.get_id()) == Color::RED)
      total_bends++;
  return total_bends;
}

int compute_max_bends_per_edge(const DrawingResult& result) {
  const auto& graph = *result.augmented_graph;
  const auto& attributes = result.attributes;
  int max_reds = 0;
  for (const auto& node : graph.get_nodes()) {
    if (attributes.get_node_color(node.get_id()) != Color::BLACK) continue;
    std::unordered_set<int> visited;
    std::function<void(int, int, int)> dfs = [&](int current, int black,
                                                 int red_count) {
      visited.insert(current);
      for (const auto& edge : graph.get_node_by_id(current).get_edges()) {
        int neighbor = edge.get_to().get_id();
        if (visited.contains(neighbor)) continue;
        Color neighbor_color = attributes.get_node_color(neighbor);
        if (neighbor_color == Color::RED)
          dfs(neighbor, black, red_count + 1);
        else if (neighbor_color == Color::BLACK && black < neighbor)
          max_reds = std::max(max_reds, red_count);
      }
      visited.erase(current);
    };
    dfs(node.get_id(), node.get_id(), 0);
  }
  return max_reds;
}

double compute_bends_std_dev(const DrawingResult& result) {
  const auto& graph = *result.augmented_graph;
  const auto& attributes = result.attributes;
  std::vector<int> red_counts;
  for (const auto& node : graph.get_nodes()) {
    if (attributes.get_node_color(node.get_id()) != Color::BLACK) continue;
    std::unordered_set<int> visited;
    std::function<void(int, int, int)> dfs = [&](int current, int black,
                                                 int red_count) {
      visited.insert(current);
      for (const auto& edge : graph.get_node_by_id(current).get_edges()) {
        int neighbor = edge.get_to().get_id();
        if (visited.contains(neighbor)) continue;
        Color neighbor_color = attributes.get_node_color(neighbor);
        if (neighbor_color == Color::RED)
          dfs(neighbor, black, red_count + 1);
        else if (neighbor_color == Color::BLACK && black < neighbor)
          red_counts.push_back(red_count);
      }
      visited.erase(current);
    };
    dfs(node.get_id(), node.get_id(), 0);
  }
  return compute_stddev(red_counts);
}

// to remove here
auto build_coordinate_x_to_nodes_(const Graph& graph,
                                  const NodesPositions& positions) {
  std::unordered_map<int, std::unordered_set<int>> coordinate_x_to_nodes;
  for (const auto& node : graph.get_nodes()) {
    int node_id = node.get_id();
    int x = std::round(100.0 * positions.get_position_x(node_id));
    coordinate_x_to_nodes[x].insert(node_id);
  }
  return coordinate_x_to_nodes;
}

// to remove here
auto build_coordinate_y_to_nodes_(const Graph& graph,
                                  const NodesPositions& positions) {
  std::unordered_map<int, std::unordered_set<int>> coordinate_y_to_nodes;
  for (const auto& node : graph.get_nodes()) {
    int node_id = node.get_id();
    int y = std::round(100.0 * positions.get_position_y(node_id));
    coordinate_y_to_nodes[y].insert(node_id);
  }
  return coordinate_y_to_nodes;
}

std::pair<int, int> compute_max_coordinates_integer(
    const Graph& graph, const NodesPositions& positions) {
  auto coordinate_y_to_nodes = build_coordinate_y_to_nodes_(graph, positions);
  auto coordinate_x_to_nodes = build_coordinate_x_to_nodes_(graph, positions);
  int actual_y = 0;
  int y_index = 0;
  while (coordinate_y_to_nodes.contains(actual_y)) {
    if (coordinate_y_to_nodes.contains(actual_y + 5)) {
      actual_y += 5;
    } else {
      actual_y += 100;
      ++y_index;
    }
  }
  int actual_x = 0;
  int x_index = 0;
  while (coordinate_x_to_nodes.contains(actual_x)) {
    if (coordinate_x_to_nodes.contains(actual_x + 5)) {
      actual_x += 5;
    } else {
      actual_x += 100;
      ++x_index;
    }
  }
  return std::make_pair(x_index - 1, y_index - 1);
}

int compute_total_area(const DrawingResult& result) {
  const auto& graph = *result.augmented_graph;
  const auto& positions = result.positions;
  auto [max_x, max_y] = compute_max_coordinates_integer(graph, positions);
  return (max_x + 1) * (max_y + 1);
}

int compute_total_crossings(const DrawingResult& result) {
  const auto& graph = *result.augmented_graph;
  const auto& positions = result.positions;
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
      if (i == k || i == l || j == k || j == l) continue;
      if (do_edges_cross(positions, i, j, k, l)) ++total_crossings;
    }
  }
  total_crossings /= 4;
  for (const GraphNode& node : graph.get_nodes()) {
    int degree = node.get_degree();
    if (degree > 4) total_crossings -= (degree - 4);
  }
  return total_crossings;
}

OrthogonalStats compute_all_orthogonal_stats(const DrawingResult& result) {
  return {
      compute_total_crossings(result),    compute_total_bends(result),
      compute_total_area(result),         compute_total_edge_length(result),
      compute_max_edge_length(result),    compute_edge_length_std_dev(result),
      compute_max_bends_per_edge(result), compute_bends_std_dev(result)};
}
