#include "orthogonal/shape/variables_handler.hpp"

GraphEdgeHashMap<int>& VariablesHandler::direction_type_to_variable_map(
    Direction direction, VariableType type) {
  switch (direction) {
    case Direction::UP:
      return (type == VariableType::STANDARD) ? m_edge_up_variable
                                              : m_special_edge_up_variable;
    case Direction::DOWN:
      return (type == VariableType::STANDARD) ? m_edge_down_variable
                                              : m_special_edge_down_variable;
    case Direction::RIGHT:
      return (type == VariableType::STANDARD) ? m_edge_right_variable
                                              : m_special_edge_right_variable;
    case Direction::LEFT:
      return (type == VariableType::STANDARD) ? m_edge_left_variable
                                              : m_special_edge_left_variable;
    default:
      throw std::invalid_argument("Invalid direction");
  }
}

void VariablesHandler::add_variable(int i, int j, Direction direction,
                                    VariableType type) {
  GraphEdgeHashMap<int>& variable_map =
      direction_type_to_variable_map(direction, type);
  variable_map[{i, j}] = m_next_var;
  variable_to_type[m_next_var] = type;
  variable_to_edge[m_next_var] = std::make_pair(i, j);
  variable_to_direction[m_next_var] = direction;
  m_next_var++;
}

void VariablesHandler::add_edge_variables(int i, int j, VariableType type) {
  add_variable(i, j, Direction::UP, type);
  add_variable(i, j, Direction::DOWN, type);
  add_variable(i, j, Direction::LEFT, type);
  add_variable(i, j, Direction::RIGHT, type);
  m_edge_down_variable[{j, i}] = m_edge_up_variable[{i, j}];
  m_edge_up_variable[{j, i}] = m_edge_down_variable[{i, j}];
  m_edge_left_variable[{j, i}] = m_edge_right_variable[{i, j}];
  m_edge_right_variable[{j, i}] = m_edge_left_variable[{i, j}];
}

void VariablesHandler::add_high_degree_node_variables(const Graph& graph,
                                                      const GraphNode& node) {
  std::vector<const GraphEdge*> edges;
  for (auto& edge : node.get_edges()) edges.push_back(&edge);
  for (int i = 0; i < edges.size() - 1; i++) {
    int edge_1_id = edges[i]->get_id();
    for (int j = i + 1; j < edges.size(); j++) {
      int edge_2_id = edges[j]->get_id();
      if (edge_1_id < edge_2_id)
        add_edge_variables(edge_1_id, edge_2_id, VariableType::SPECIAL_EDGE);
      else
        add_edge_variables(edge_2_id, edge_1_id, VariableType::SPECIAL_EDGE);
    }
  }
}

VariablesHandler::VariablesHandler(const Graph& graph) {
  for (const auto& node : graph.get_nodes()) {
    int i = node.get_id();
    for (auto& edge : node.get_edges()) {
      int j = edge.get_to().get_id();
      if (i > j) continue;
      add_edge_variables(i, j, VariableType::STANDARD);
    }
    if (node.get_degree() > 4) add_high_degree_node_variables(graph, node);
  }
}

int VariablesHandler::get_special_up_variable(int i, int j) const {
  return m_special_edge_up_variable.at({i, j});
}

int VariablesHandler::get_special_down_variable(int i, int j) const {
  return m_special_edge_down_variable.at({i, j});
}

int VariablesHandler::get_special_left_variable(int i, int j) const {
  return m_special_edge_left_variable.at({i, j});
}

int VariablesHandler::get_special_right_variable(int i, int j) const {
  return m_special_edge_right_variable.at({i, j});
}

int VariablesHandler::get_up_variable(int i, int j) const {
  return m_edge_up_variable.at({i, j});
}

int VariablesHandler::get_down_variable(int i, int j) const {
  return m_edge_down_variable.at({i, j});
}

int VariablesHandler::get_left_variable(int i, int j) const {
  return m_edge_left_variable.at({i, j});
}

int VariablesHandler::get_right_variable(int i, int j) const {
  return m_edge_right_variable.at({i, j});
}

int VariablesHandler::get_variable(int i, int j, Direction direction) const {
  if (direction == Direction::UP) return get_up_variable(i, j);
  if (direction == Direction::DOWN) return get_down_variable(i, j);
  if (direction == Direction::LEFT) return get_left_variable(i, j);
  if (direction == Direction::RIGHT) return get_right_variable(i, j);
  throw std::invalid_argument("Invalid direction");
}

const std::pair<int, int>& VariablesHandler::get_edge_of_variable(
    int variable) const {
  return variable_to_edge.at(variable);
}

bool VariablesHandler::is_variable_standard(int variable) const {
  return variable_to_type.at(variable) == VariableType::STANDARD;
}

void VariablesHandler::set_variable_value(int variable, bool value) {
  if (variable_to_value.contains(variable))
    throw std::runtime_error("Variable already has a value");
  variable_to_value[variable] = value;
}

bool VariablesHandler::get_variable_value(int variable) const {
  if (!variable_to_value.contains(variable))
    throw std::runtime_error("Variable does not have a value");
  return variable_to_value.at(variable);
}

Direction VariablesHandler::get_direction_of_standard_edge(int i, int j) const {
  if (get_variable_value(get_up_variable(i, j))) return Direction::UP;
  if (get_variable_value(get_down_variable(i, j))) return Direction::DOWN;
  if (get_variable_value(get_left_variable(i, j))) return Direction::LEFT;
  if (get_variable_value(get_right_variable(i, j))) return Direction::RIGHT;
  throw std::runtime_error("No direction found for standard edge");
}

int VariablesHandler::get_special_variable(int i, int j,
                                           Direction direction) const {
  if (direction == Direction::UP) return get_special_up_variable(i, j);
  if (direction == Direction::DOWN) return get_special_down_variable(i, j);
  if (direction == Direction::LEFT) return get_special_left_variable(i, j);
  if (direction == Direction::RIGHT) return get_special_right_variable(i, j);
  throw std::invalid_argument("Invalid direction");
}

int VariablesHandler::get_new_auxiliary_variable() {
  variable_to_type[m_next_var] = VariableType::AUXILIARY;
  return m_next_var++;
}