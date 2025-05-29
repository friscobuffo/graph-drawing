#ifndef MY_VARIABLES_HANDLER_H
#define MY_VARIABLES_HANDLER_H

#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

#include "core/graph/graph.hpp"
#include "orthogonal/shape/shape.hpp"

class VariablesHandler {
 public:
  enum class VariableType {
    STANDARD,
    SPECIAL_EDGE,
    AUXILIARY,
  };

 private:
  int m_next_var = 1;  // 0 is reserved for the empty clause
  std::unordered_map<int, std::pair<int, int>> variable_to_edge;
  std::unordered_map<int, Direction> variable_to_direction;
  std::unordered_map<int, VariableType> variable_to_type;
  std::unordered_map<int, bool> variable_to_value;
  // normal variables related to graph edges
  GraphEdgeHashMap<int> m_edge_up_variable;
  GraphEdgeHashMap<int> m_edge_down_variable;
  GraphEdgeHashMap<int> m_edge_right_variable;
  GraphEdgeHashMap<int> m_edge_left_variable;
  // special variable for high degree nodes
  GraphEdgeHashMap<int> m_special_edge_up_variable;
  GraphEdgeHashMap<int> m_special_edge_down_variable;
  GraphEdgeHashMap<int> m_special_edge_right_variable;
  GraphEdgeHashMap<int> m_special_edge_left_variable;
  GraphEdgeHashMap<int>& direction_type_to_variable_map(Direction direction,
                                                        VariableType type);
  void add_variable(int i, int j, Direction direction, VariableType type);
  void add_edge_variables(int i, int j, VariableType type);
  void add_high_degree_node_variables(const Graph& graph,
                                      const GraphNode& node);

 public:
  VariablesHandler(const Graph& graph);
  int get_special_up_variable(int i, int j) const;
  int get_special_down_variable(int i, int j) const;
  int get_special_left_variable(int i, int j) const;
  int get_special_right_variable(int i, int j) const;
  int get_up_variable(int i, int j) const;
  int get_down_variable(int i, int j) const;
  int get_left_variable(int i, int j) const;
  int get_right_variable(int i, int j) const;
  int get_variable(int i, int j, Direction direction) const;
  const std::pair<int, int>& get_edge_of_variable(int variable) const;
  bool is_variable_standard(int variable) const;
  void set_variable_value(int variable, bool value);
  bool get_variable_value(int variable) const;
  Direction get_direction_of_standard_edge(int i, int j) const;
  int get_special_variable(int i, int j, Direction direction) const;
  int get_new_auxiliary_variable();
};

#endif