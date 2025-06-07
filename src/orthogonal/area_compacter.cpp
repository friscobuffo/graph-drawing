#include "orthogonal/area_compacter.hpp"

#include <cfloat>

#include "orthogonal/equivalence_classes.hpp"

float get_x_coordinate(const NodesPositions& positions, int node_id) {
  return positions.get_position_x(node_id);
}

float get_y_coordinate(const NodesPositions& positions, int node_id) {
  return positions.get_position_y(node_id);
}

bool can_move_to_previous_coordinate(
    const std::unordered_set<int>& classes_at_coordinate,
    const std::unordered_set<int>& classes_at_previous_coordinate,
    const std::unordered_map<int, std::pair<float, float>>&
        class_to_min_max_coord) {
  if (classes_at_previous_coordinate.empty()) return true;
  for (int class_id_1 : classes_at_coordinate) {
    float min_1 = class_to_min_max_coord.at(class_id_1).first;
    float max_1 = class_to_min_max_coord.at(class_id_1).second;
    for (int class_id_2 : classes_at_previous_coordinate) {
      float min_2 = class_to_min_max_coord.at(class_id_2).first;
      float max_2 = class_to_min_max_coord.at(class_id_2).second;
      if (!(min_1 > max_2 || min_2 > max_1)) return false;
    }
  }
  return true;
}

bool does_class_contain_colored_node(const EquivalenceClasses& classes,
                                     int class_id, Color color,
                                     const GraphAttributes& attributes) {
  for (int node_id : classes.get_elems_of_class(class_id))
    if (attributes.get_node_color(node_id) == color) return true;
  return false;
}

template <typename Func>
auto build_coordinate_to_classes(const Graph& graph,
                                 const NodesPositions& old_positions,
                                 const EquivalenceClasses& classes,
                                 Func get_position, Color color,
                                 const GraphAttributes& attributes) {
  std::unordered_map<float, std::unordered_set<int>> coordinate_to_classes;
  for (int class_id : classes.get_all_classes()) {
    int node_id = *classes.get_elems_of_class(class_id).begin();
    float coordinate = get_position(old_positions, node_id);
    if (coordinate_to_classes.contains(coordinate))
      throw std::runtime_error("build_coordinate_to_classes: wtf");
    coordinate_to_classes[coordinate] = {class_id};
  }
  std::vector<float> sorted_coordinates;
  for (const auto& [coordinate, _] : coordinate_to_classes)
    sorted_coordinates.push_back(coordinate);
  std::sort(sorted_coordinates.begin(), sorted_coordinates.end());
  float previous_coordinate = sorted_coordinates[0];
  std::vector<float> true_sorted_coordinates;
  true_sorted_coordinates.push_back(previous_coordinate);
  for (int i = 1; i < sorted_coordinates.size(); ++i) {
    float coordinate = sorted_coordinates[i];
    int class_id = *coordinate_to_classes[coordinate].begin();
    if (does_class_contain_colored_node(classes, class_id, color, attributes)) {
      coordinate_to_classes[coordinate].clear();
      coordinate_to_classes[previous_coordinate].insert(class_id);
      continue;
    }
    previous_coordinate = coordinate;
    true_sorted_coordinates.push_back(coordinate);
  }
  return std::make_pair(std::move(coordinate_to_classes),
                        std::move(true_sorted_coordinates));
}

void shift_x_position(NodesPositions& positions, float shift_value,
                      int node_id) {
  float old_x = positions.get_position_x(node_id);
  positions.change_position(node_id, old_x - shift_value,
                            positions.get_position_y(node_id));
}

void shift_y_position(NodesPositions& positions, float shift_value,
                      int node_id) {
  float old_y = positions.get_position_y(node_id);
  positions.change_position(node_id, positions.get_position_x(node_id),
                            old_y - shift_value);
}

template <typename Func>
void compact_area(
    const Graph& graph, const Shape& shape, NodesPositions& old_positions,
    const GraphAttributes& attributes, const EquivalenceClasses& classes,
    std::unordered_map<float, std::unordered_set<int>>& coordinate_to_classes,
    std::vector<float>& sorted_coordinates,
    std::unordered_map<int, std::pair<float, float>>&
        class_to_min_max_other_coord,
    Func shifting_function) {
  for (int i = 1; i < sorted_coordinates.size(); ++i) {
    const float coordinate = sorted_coordinates[i];
    int previous_index = i - 1;
    float previous_coordinate = sorted_coordinates[previous_index];
    float new_coordinate = coordinate;
    const std::unordered_set<int>& classes_at_coordinate =
        coordinate_to_classes[coordinate];
    do {
      std::unordered_set<int>& classes_at_previous_coordinate =
          coordinate_to_classes[previous_coordinate];
      bool move = can_move_to_previous_coordinate(
          classes_at_coordinate, classes_at_previous_coordinate,
          class_to_min_max_other_coord);
      if (!move) break;
      new_coordinate = previous_coordinate;
      previous_index--;
      if (previous_index < 0) break;
      previous_coordinate = sorted_coordinates[previous_index];
    } while (true);
    if (new_coordinate != coordinate) {
      float shift = coordinate - new_coordinate;
      for (int class_id : classes_at_coordinate) {
        for (int node_id : classes.get_elems_of_class(class_id))
          shifting_function(old_positions, shift, node_id);
        coordinate_to_classes[new_coordinate].insert(class_id);
      }
      coordinate_to_classes.erase(coordinate);
    }
  }
}

void compact_area(const Graph& graph, const Shape& shape,
                  NodesPositions& old_positions,
                  const GraphAttributes& attributes) {
  auto [classes_x, classes_y] = build_equivalence_classes(shape, graph);
  auto [coordinate_to_classes_x, sorted_coordinates_x] =
      build_coordinate_to_classes(graph, old_positions, classes_x,
                                  get_x_coordinate, Color::BLUE, attributes);
  std::unordered_map<int, std::pair<float, float>> class_to_min_max_y;
  for (int class_id : classes_x.get_all_classes()) {
    float min = FLT_MAX;
    float max = 0;
    for (auto& node : classes_x.get_elems_of_class(class_id)) {
      min = std::min(min, get_y_coordinate(old_positions, node));
      max = std::max(max, get_y_coordinate(old_positions, node));
    }
    class_to_min_max_y[class_id] = {min, max};
  }
  compact_area(graph, shape, old_positions, attributes, classes_x,
               coordinate_to_classes_x, sorted_coordinates_x,
               class_to_min_max_y, shift_x_position);

  auto [coordinate_to_classes_y, sorted_coordinates_y] =
      build_coordinate_to_classes(graph, old_positions, classes_y,
                                  get_y_coordinate, Color::GREEN, attributes);

  std::unordered_map<int, std::pair<float, float>> class_to_min_max_x;
  for (int class_id : classes_y.get_all_classes()) {
    float min = FLT_MAX;
    float max = 0;
    for (auto& node : classes_y.get_elems_of_class(class_id)) {
      min = std::min(min, get_x_coordinate(old_positions, node));
      max = std::max(max, get_x_coordinate(old_positions, node));
    }
    class_to_min_max_x[class_id] = {min, max};
  }
  compact_area(graph, shape, old_positions, attributes, classes_y,
               coordinate_to_classes_y, sorted_coordinates_y,
               class_to_min_max_x, shift_y_position);
}