#include "orthogonal/shape/clauses_functions.hpp"

void add_constraints_at_most_one_is_true(CnfBuilder& cnf_builder, int var_1,
                                         int var_2, int var_3, int var_4) {
  // at most one is true (at least three are false)
  // for every possible pair, at least one is false
  cnf_builder.add_clause({-var_1, -var_2});
  cnf_builder.add_clause({-var_1, -var_3});
  cnf_builder.add_clause({-var_1, -var_4});
  cnf_builder.add_clause({-var_2, -var_3});
  cnf_builder.add_clause({-var_2, -var_4});
  cnf_builder.add_clause({-var_3, -var_4});
}
void add_constraints_one_direction_per_edge(const Graph& graph,
                                            CnfBuilder& cnf_builder,
                                            const VariablesHandler& handler,
                                            int up, int down, int right,
                                            int left) {
  cnf_builder.add_clause({up, down, right, left});  // at least one is true
  add_constraints_at_most_one_is_true(cnf_builder, up, down, left, right);
}

void add_constraints_one_direction_per_edge(const Graph& graph,
                                            CnfBuilder& cnf_builder,
                                            const VariablesHandler& handler) {
  for (auto& node : graph.get_nodes()) {
    int i = node.get_id();
    for (auto& edge : node.get_edges()) {
      int j = edge.get_to().get_id();
      if (i > j) continue;
      int up = handler.get_up_variable(i, j);
      int down = handler.get_down_variable(i, j);
      int right = handler.get_right_variable(i, j);
      int left = handler.get_left_variable(i, j);
      add_constraints_one_direction_per_edge(graph, cnf_builder, handler, up,
                                             down, right, left);
    }
  }
}

void add_clause_at_least_one_in_direction(CnfBuilder& cnf_builder,
                                          const VariablesHandler& handler,
                                          const GraphNode& node,
                                          Direction direction) {
  std::vector<int> clause;
  int node_id = node.get_id();
  for (auto& edge : node.get_edges()) {
    int neighbor_id = edge.get_to().get_id();
    clause.push_back(handler.get_variable(node_id, neighbor_id, direction));
  }
  cnf_builder.add_clause(clause);
}

void add_one_edge_per_direction_clauses(CnfBuilder& cnf_builder,
                                        const VariablesHandler& handler,
                                        const Direction direction,
                                        const GraphNode& node) {
  int i = node.get_id();
  int degree = node.get_degree();
  if (degree == 4) {
    add_clause_at_least_one_in_direction(cnf_builder, handler, node, direction);
  } else if (degree == 3) {
    std::vector<int> variables;
    for (auto& edge : node.get_edges())
      variables.push_back(
          handler.get_variable(i, edge.get_to().get_id(), direction));
    // at most one is true (at least 2 are false)
    cnf_builder.add_clause({-variables[0], -variables[1]});
    cnf_builder.add_clause({-variables[0], -variables[2]});
    cnf_builder.add_clause({-variables[1], -variables[2]});
  } else if (degree == 2) {
    std::vector<int> clause;
    for (auto& edge : node.get_edges())
      clause.push_back(
          -handler.get_variable(i, edge.get_to().get_id(), direction));
    // at most one is true (at least 1 is false)
    cnf_builder.add_clause(clause);
  } else if (degree != 1) {
    throw std::runtime_error("degree of node is not valid");
  }
}

void add_nodes_constraints(const Graph& graph, CnfBuilder& cnf_builder,
                           const VariablesHandler& handler) {
  for (auto& node : graph.get_nodes()) {
    add_one_edge_per_direction_clauses(cnf_builder, handler, Direction::UP,
                                       node);
    add_one_edge_per_direction_clauses(cnf_builder, handler, Direction::DOWN,
                                       node);
    add_one_edge_per_direction_clauses(cnf_builder, handler, Direction::RIGHT,
                                       node);
    add_one_edge_per_direction_clauses(cnf_builder, handler, Direction::LEFT,
                                       node);
  }
}

void add_cycles_constraints(const Graph& graph, CnfBuilder& cnf_builder,
                            const std::vector<std::vector<int>>& cycles,
                            const VariablesHandler& handler) {
  for (auto& cycle : cycles) {
    auto at_least_one_down = std::vector<int>();
    auto at_least_one_up = std::vector<int>();
    auto at_least_one_right = std::vector<int>();
    auto at_least_one_left = std::vector<int>();
    for (int i = 0; i < cycle.size(); i++) {
      at_least_one_down.push_back(
          handler.get_down_variable(cycle[i], cycle[(i + 1) % cycle.size()]));
      at_least_one_up.push_back(
          handler.get_up_variable(cycle[i], cycle[(i + 1) % cycle.size()]));
      at_least_one_right.push_back(
          handler.get_right_variable(cycle[i], cycle[(i + 1) % cycle.size()]));
      at_least_one_left.push_back(
          handler.get_left_variable(cycle[i], cycle[(i + 1) % cycle.size()]));
    }
    cnf_builder.add_clause(at_least_one_down);
    cnf_builder.add_clause(at_least_one_up);
    cnf_builder.add_clause(at_least_one_right);
    cnf_builder.add_clause(at_least_one_left);
  }
}

void add_nodes_constraints_special(const Graph& graph, CnfBuilder& cnf_builder,
                                   const VariablesHandler& handler) {
  for (auto& node : graph.get_nodes()) {
    if (node.get_degree() <= 4) {
      add_one_edge_per_direction_clauses(cnf_builder, handler, Direction::UP,
                                         node);
      add_one_edge_per_direction_clauses(cnf_builder, handler, Direction::DOWN,
                                         node);
      add_one_edge_per_direction_clauses(cnf_builder, handler, Direction::RIGHT,
                                         node);
      add_one_edge_per_direction_clauses(cnf_builder, handler, Direction::LEFT,
                                         node);
    } else {
      add_clause_at_least_one_in_direction(cnf_builder, handler, node,
                                           Direction::UP);
      add_clause_at_least_one_in_direction(cnf_builder, handler, node,
                                           Direction::DOWN);
      add_clause_at_least_one_in_direction(cnf_builder, handler, node,
                                           Direction::RIGHT);
      add_clause_at_least_one_in_direction(cnf_builder, handler, node,
                                           Direction::LEFT);
    }
  }
}

void special_implies_same_directions_of_edges(
    VariablesHandler& handler, CnfBuilder& cnf_builder, int special_1,
    int special_2, int edge_1_dir_1, int edge_1_dir_2, int edge_2_dir_1,
    int edge_2_dir_2) {
  /*
  (¬special_1 ∨ e_1_dir_1 ∨ e_1_dir_2)
  ∧
  (¬special_1 ∨ e_1_dir_1 ∨ e_2_dir_2)
  ∧
  (¬special_1 ∨ e_2_dir_1 ∨ e_1_dir_2)
  ∧
  (¬special_1 ∨ e_2_dir_1 ∨ e_2_dir_2)
  ∧
  (¬special_2 ∨ e_1_dir_1 ∨ e_1_dir_2)
  ∧
  (¬special_2 ∨ e_1_dir_1 ∨ e_2_dir_2)
  ∧
  (¬special_2 ∨ e_2_dir_1 ∨ e_1_dir_2)
  ∧
  (¬special_2 ∨ e_2_dir_1 ∨ e_2_dir_2)
*/
  int aux_a = handler.get_new_auxiliary_variable();
  int aux_b = handler.get_new_auxiliary_variable();
  int aux_c = handler.get_new_auxiliary_variable();
  int aux_d = handler.get_new_auxiliary_variable();

  cnf_builder.add_clause({-aux_a, special_1, special_2});
  cnf_builder.add_clause({-special_1, aux_a});
  cnf_builder.add_clause({-special_2, aux_a});

  cnf_builder.add_clause({-aux_b, edge_1_dir_1});
  cnf_builder.add_clause({-aux_b, edge_2_dir_1});
  cnf_builder.add_clause({aux_b, -edge_1_dir_1, -edge_2_dir_1});

  cnf_builder.add_clause({-aux_c, edge_1_dir_2});
  cnf_builder.add_clause({-aux_c, edge_2_dir_2});
  cnf_builder.add_clause({aux_c, -edge_1_dir_2, -edge_2_dir_2});

  cnf_builder.add_clause({-aux_d, aux_b, aux_c});
  cnf_builder.add_clause({-aux_b, aux_d});
  cnf_builder.add_clause({-aux_c, aux_d});

  cnf_builder.add_clause({-aux_a, aux_d});
}

// implications that if edge1 and edge2 have same direction
// one of the special variables' perpendicular direction is true
// (i.e. if they are both right, special variable is either up or down)
// also, the opposite implication, i.e.
// if the special variable perpendicular is true, then edge1 and edge2 are in
// the same direction
void add_clauses_special_edges(const Graph& graph, CnfBuilder& cnf_builder,
                               VariablesHandler& handler, int edge_1_id,
                               int edge_2_id) {
  const std::array<Direction, 4> directions = {
      Direction::UP, Direction::DOWN, Direction::RIGHT, Direction::LEFT};
  const auto& edge_1 = graph.get_edge_by_id(edge_1_id);
  const auto& edge_2 = graph.get_edge_by_id(edge_2_id);
  if (edge_1.get_from().get_id() != edge_2.get_from().get_id())
    throw std::runtime_error("wrong edges");
  int edge_1_from_id = edge_1.get_from().get_id();
  int edge_1_to_id = edge_1.get_to().get_id();
  int edge_2_from_id = edge_2.get_from().get_id();
  int edge_2_to_id = edge_2.get_to().get_id();
  for (const Direction direction : directions) {
    const Direction perpendicular_direction = rotate_90_degrees(direction);
    int edge_1_var =
        handler.get_variable(edge_1_from_id, edge_1_to_id, direction);
    int edge_2_var =
        handler.get_variable(edge_2_from_id, edge_2_to_id, direction);
    int special_var_1 = handler.get_special_variable(edge_1_id, edge_2_id,
                                                     perpendicular_direction);
    int special_var_2 = handler.get_special_variable(
        edge_1_id, edge_2_id, opposite_direction(perpendicular_direction));
    // (edge_1_var and edge_2_var) implies (special_var_1 or special_var_2)
    cnf_builder.add_clause(
        {-edge_1_var, -edge_2_var, special_var_1, special_var_2});
  }
  int special_right = handler.get_special_right_variable(edge_1_id, edge_2_id);
  int special_left = handler.get_special_left_variable(edge_1_id, edge_2_id);
  int special_up = handler.get_special_up_variable(edge_1_id, edge_2_id);
  int special_down = handler.get_special_down_variable(edge_1_id, edge_2_id);
  add_constraints_at_most_one_is_true(cnf_builder, special_right, special_left,
                                      special_up, special_down);
  special_implies_same_directions_of_edges(
      handler, cnf_builder, special_right, special_left,
      handler.get_down_variable(edge_1_from_id, edge_1_to_id),
      handler.get_up_variable(edge_1_from_id, edge_1_to_id),
      handler.get_down_variable(edge_2_from_id, edge_2_to_id),
      handler.get_up_variable(edge_2_from_id, edge_2_to_id));
  special_implies_same_directions_of_edges(
      handler, cnf_builder, special_up, special_down,
      handler.get_right_variable(edge_1_from_id, edge_1_to_id),
      handler.get_left_variable(edge_1_from_id, edge_1_to_id),
      handler.get_right_variable(edge_2_from_id, edge_2_to_id),
      handler.get_left_variable(edge_2_from_id, edge_2_to_id));
}

void add_constraints_special_edges(const Graph& graph, CnfBuilder& cnf_builder,
                                   VariablesHandler& handler) {
  for (auto& node : graph.get_nodes()) {
    if (node.get_degree() <= 4) continue;
    std::vector<const GraphEdge*> edges;
    for (auto& edge : node.get_edges()) edges.push_back(&edge);
    for (int i = 0; i < edges.size() - 1; i++) {
      for (int j = i + 1; j < edges.size(); j++) {
        int edge_1_id = edges[i]->get_id();
        int edge_2_id = edges[j]->get_id();
        if (edge_1_id > edge_2_id) std::swap(edge_1_id, edge_2_id);
        add_clauses_special_edges(graph, cnf_builder, handler, edge_1_id,
                                  edge_2_id);
      }
    }
  }
}

void add_cycles_constraints_special(const Graph& graph, CnfBuilder& cnf_builder,
                                    const std::vector<std::vector<int>>& cycles,
                                    const VariablesHandler& handler) {
  for (auto& cycle : cycles) {
    auto at_least_one_down = std::vector<int>();
    auto at_least_one_up = std::vector<int>();
    auto at_least_one_right = std::vector<int>();
    auto at_least_one_left = std::vector<int>();
    for (int i = 0; i < cycle.size(); i++) {
      at_least_one_down.push_back(
          handler.get_down_variable(cycle[i], cycle[(i + 1) % cycle.size()]));
      at_least_one_up.push_back(
          handler.get_up_variable(cycle[i], cycle[(i + 1) % cycle.size()]));
      at_least_one_right.push_back(
          handler.get_right_variable(cycle[i], cycle[(i + 1) % cycle.size()]));
      at_least_one_left.push_back(
          handler.get_left_variable(cycle[i], cycle[(i + 1) % cycle.size()]));
    }
    for (int i = 0; i < cycle.size(); i++) {
      int node_id = cycle[i];
      const auto& node = graph.get_node_by_id(node_id);
      if (node.get_degree() <= 4) continue;
      int prev_node_id = cycle[(i - 1 + cycle.size()) % cycle.size()];
      int next_node_id = cycle[(i + 1) % cycle.size()];
      int edge_1_id = graph.get_edge(node_id, prev_node_id).get_id();
      int edge_2_id = graph.get_edge(node_id, next_node_id).get_id();
      if (edge_1_id > edge_2_id) std::swap(edge_1_id, edge_2_id);
      int special_var_down =
          handler.get_special_down_variable(edge_1_id, edge_2_id);
      int special_var_up =
          handler.get_special_up_variable(edge_1_id, edge_2_id);
      int special_var_right =
          handler.get_special_right_variable(edge_1_id, edge_2_id);
      int special_var_left =
          handler.get_special_left_variable(edge_1_id, edge_2_id);
      at_least_one_down.push_back(special_var_down);
      at_least_one_up.push_back(special_var_up);
      at_least_one_right.push_back(special_var_right);
      at_least_one_left.push_back(special_var_left);
    }
    cnf_builder.add_clause(at_least_one_down);
    cnf_builder.add_clause(at_least_one_up);
    cnf_builder.add_clause(at_least_one_right);
    cnf_builder.add_clause(at_least_one_left);
  }
}