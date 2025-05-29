#include "orthogonal/shape/shape_builder.hpp"

#include <algorithm>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "orthogonal/shape/clauses_functions.hpp"
#include "orthogonal/shape/variables_handler.hpp"
#include "sat/cnf_builder.hpp"
#include "sat/glucose.hpp"

Shape result_to_shape(const Graph& graph, const std::vector<int>& numbers,
                      VariablesHandler& handler) {
  for (int i = 0; i < numbers.size(); i++) {
    int var = numbers[i];
    if (var > 0)
      handler.set_variable_value(var, true);
    else
      handler.set_variable_value(-var, false);
  }
  Shape shape;
  for (const auto& node : graph.get_nodes()) {
    int i = node.get_id();
    for (auto& edge : node.get_edges()) {
      int j = edge.get_to().get_id();
      shape.set_direction(i, j, handler.get_direction_of_standard_edge(i, j));
    }
  }
  return std::move(shape);
}

int find_variable_of_edge_to_remove(const std::vector<std::string>& proof_lines,
                                    std::mt19937& random_engine,
                                    const VariablesHandler& handler) {
  std::vector<int> unit_clauses;
  for (int i = proof_lines.size() - 1; i >= 0; i--) {
    const std::string& line = proof_lines[i];
    // split line based on " "
    std::vector<int> tokens;
    std::string token;
    for (char c : line) {
      if (c == 'd') continue;
      if (c == ' ') {
        if (token == "") continue;
        tokens.push_back(std::stoi(token));
        token = "";
      } else
        token += c;
    }
    if (token != "0") throw std::runtime_error("Invalid proof line");
    if (tokens.size() == 1 && handler.is_variable_standard(abs(tokens[0])))
      unit_clauses.push_back(tokens[0]);
  }
  if (unit_clauses.size() == 0)
    throw std::runtime_error("Could not find the edge to remove");
  // pick one of the first two unit clauses
  int random_index = random_engine() % std::min((int)unit_clauses.size(), 2);
  return std::abs(unit_clauses[random_index]);
}

std::optional<Shape> build_shape_or_add_corner(
    Graph& graph, GraphAttributes& attributes,
    std::vector<std::vector<int>>& cycles, std::mt19937& random_engine);

Shape build_shape(Graph& graph, GraphAttributes& attributes,
                  std::vector<std::vector<int>>& cycles, bool randomize) {
  int seed = (randomize) ? std::random_device{}() : 42;
  std::mt19937 random_engine(seed);
  auto shape =
      build_shape_or_add_corner(graph, attributes, cycles, random_engine);
  while (!shape.has_value())
    shape = build_shape_or_add_corner(graph, attributes, cycles, random_engine);
  return std::move(shape.value());
}

std::optional<Shape> build_shape_or_add_corner(
    Graph& graph, GraphAttributes& attributes,
    std::vector<std::vector<int>>& cycles, std::mt19937& random_engine) {
  VariablesHandler handler(graph);
  CnfBuilder cnf_builder;
  cnf_builder.add_comment("constraints one direction per edge");
  add_constraints_one_direction_per_edge(graph, cnf_builder, handler);
  cnf_builder.add_comment("constraints nodes");
  add_nodes_constraints(graph, cnf_builder, handler);
  cnf_builder.add_comment("constraints cycles");
  add_cycles_constraints(graph, cnf_builder, cycles, handler);
  const std::string cnf = get_unique_filename("cnf");
  cnf_builder.convert_to_cnf(cnf);
  auto results = launch_glucose(cnf, false);
  remove(cnf.c_str());
  if (results.result == GlucoseResultType::UNSAT) {
    const int variable_edge = find_variable_of_edge_to_remove(
        results.proof_lines, random_engine, handler);
    const int i = handler.get_edge_of_variable(variable_edge).first;
    const int j = handler.get_edge_of_variable(variable_edge).second;
    const auto& new_node = graph.add_node();
    attributes.set_node_color(new_node.get_id(), Color::RED);
    graph.remove_undirected_edge(i, j);
    graph.add_undirected_edge(i, new_node.get_id());
    graph.add_undirected_edge(j, new_node.get_id());
    for (auto& cycle : cycles) {
      for (int k = 0; k < cycle.size(); k++) {
        if (cycle[k] == i && cycle[(k + 1) % cycle.size()] == j) {
          cycle.insert(cycle.begin() + k + 1, new_node.get_id());
          break;
        }
        if (cycle[k] == j && cycle[(k + 1) % cycle.size()] == i) {
          cycle.insert(cycle.begin() + k + 1, new_node.get_id());
          break;
        }
      }
    }
    return std::nullopt;
  }
  const std::vector<int>& variables = results.numbers;
  return result_to_shape(graph, variables, handler);
}

std::optional<Shape> build_shape_or_add_corner_special(
    Graph& graph, GraphAttributes& attributes,
    std::vector<std::vector<int>>& cycles, std::mt19937& random_engine);

Shape build_shape_special(Graph& graph, GraphAttributes& attributes,
                          std::vector<std::vector<int>>& cycles,
                          bool randomize) {
  int seed = (randomize) ? std::random_device{}() : 42;
  std::mt19937 random_engine(seed);
  auto shape = build_shape_or_add_corner_special(graph, attributes, cycles,
                                                 random_engine);
  while (!shape.has_value())
    shape = build_shape_or_add_corner_special(graph, attributes, cycles,
                                              random_engine);
  return std::move(shape.value());
}

std::optional<Shape> build_shape_or_add_corner_special(
    Graph& graph, GraphAttributes& attributes,
    std::vector<std::vector<int>>& cycles, std::mt19937& random_engine) {
  VariablesHandler handler(graph);
  CnfBuilder cnf_builder;
  cnf_builder.add_comment("constraints one direction per edge");
  add_constraints_one_direction_per_edge(graph, cnf_builder, handler);
  cnf_builder.add_comment("constraints nodes");
  add_nodes_constraints_special(graph, cnf_builder, handler);
  cnf_builder.add_comment("constraints special edges");
  add_constraints_special_edges(graph, cnf_builder, handler);
  cnf_builder.add_comment("constraints cycles");
  add_cycles_constraints_special(graph, cnf_builder, cycles, handler);
  const std::string cnf = get_unique_filename("cnf");
  cnf_builder.convert_to_cnf(cnf);
  auto results = launch_glucose(cnf, false);
  remove(cnf.c_str());
  if (results.result == GlucoseResultType::UNSAT) {
    const int variable_edge = find_variable_of_edge_to_remove(
        results.proof_lines, random_engine, handler);
    const int i = handler.get_edge_of_variable(variable_edge).first;
    const int j = handler.get_edge_of_variable(variable_edge).second;
    const auto& new_node = graph.add_node();
    attributes.set_node_color(new_node.get_id(), Color::RED);
    graph.remove_undirected_edge(i, j);
    graph.add_undirected_edge(i, new_node.get_id());
    graph.add_undirected_edge(j, new_node.get_id());
    for (auto& cycle : cycles) {
      for (int k = 0; k < cycle.size(); k++) {
        if (cycle[k] == i && cycle[(k + 1) % cycle.size()] == j) {
          cycle.insert(cycle.begin() + k + 1, new_node.get_id());
          break;
        }
        if (cycle[k] == j && cycle[(k + 1) % cycle.size()] == i) {
          cycle.insert(cycle.begin() + k + 1, new_node.get_id());
          break;
        }
      }
    }
    return std::nullopt;
  }
  const std::vector<int>& variables = results.numbers;
  return result_to_shape(graph, variables, handler);
}