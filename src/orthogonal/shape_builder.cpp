#include "shape_builder.hpp"

#include "../sat/glucose.hpp"

struct VariablesHandler {
    std::vector<std::vector<int>> is_edge_up_variable;
    std::vector<std::vector<int>> is_edge_down_variable;
    std::vector<std::vector<int>> is_edge_right_variable;
    std::vector<std::vector<int>> is_edge_left_variable;
    std::vector<std::pair<int, int>> variable_to_edge;
};

const VariablesHandler _initialize_variables(const ColoredNodesGraph& graph) {
    VariablesHandler handler{
        std::vector<std::vector<int>>(graph.size(), std::vector<int>(graph.size(), -1)),
        std::vector<std::vector<int>>(graph.size(), std::vector<int>(graph.size(), -1)),
        std::vector<std::vector<int>>(graph.size(), std::vector<int>(graph.size(), -1)),
        std::vector<std::vector<int>>(graph.size(), std::vector<int>(graph.size(), -1)),
        std::vector<std::pair<int, int>>()
    };
    int next_var = 1;
    handler.variable_to_edge.push_back(std::make_pair(-1, -1));
    for (int i = 0; i < graph.size(); i++) {
        for (auto& edge : graph.getNodes()[i].getEdges()) {
            int j = edge.getTo();
            handler.is_edge_up_variable[i][j] = next_var;
            next_var++;
            handler.is_edge_down_variable[i][j] = next_var;
            next_var++;
            handler.is_edge_left_variable[i][j] = next_var;
            next_var++;
            handler.is_edge_right_variable[i][j] = next_var;
            next_var++;
            handler.variable_to_edge.push_back(std::make_pair(i, j));
            handler.variable_to_edge.push_back(std::make_pair(i, j));
            handler.variable_to_edge.push_back(std::make_pair(i, j));
            handler.variable_to_edge.push_back(std::make_pair(i, j));
        }
    }
    return handler;
}

// each edge can only be in one direction
void add_constraints_one_direction_per_edge(
    const ColoredNodesGraph& graph,
    CNFBuilder& cnf_builder,
    const VariablesHandler& handler
) {
    for (int i = 0; i < graph.size(); i++)
        for (auto& edge : graph.getNodes()[i].getEdges()) {
            int j = edge.getTo();
            auto up = handler.is_edge_up_variable[i][j];
            auto down = handler.is_edge_down_variable[i][j];
            auto right = handler.is_edge_right_variable[i][j];
            auto left = handler.is_edge_left_variable[i][j];
            // at least one is true
            cnf_builder.add_clause({up, down, right, left});
            // at most one is true (at least three are false)
            // for every possible pair, at least one is false
            cnf_builder.add_clause({-up, -down});
            cnf_builder.add_clause({-up, -right});
            cnf_builder.add_clause({-up, -left});
            cnf_builder.add_clause({-down, -right});
            cnf_builder.add_clause({-down, -left});
            cnf_builder.add_clause({-left, -right});
        }
}

// if edge i,j is up, edge j,i is down (etc)
void add_constraints_opposite_edges(
    const ColoredNodesGraph& graph,
    CNFBuilder& cnf_builder,
    const VariablesHandler& handler
) {
    for (int i = 0; i < graph.size(); i++)
        for (auto& edge : graph.getNodes()[i].getEdges()) {
            int j = edge.getTo();
            if (i > j) continue;
            int up = handler.is_edge_up_variable[i][j];
            int down = handler.is_edge_down_variable[i][j];
            int right = handler.is_edge_right_variable[i][j];
            int left = handler.is_edge_left_variable[i][j];
            int opposite_up = handler.is_edge_up_variable[j][i];
            int opposite_down = handler.is_edge_down_variable[j][i];
            int opposite_right = handler.is_edge_right_variable[j][i];
            int opposite_left = handler.is_edge_left_variable[j][i];
            // if up is true, opposite edge down is true (and viceversa)
            cnf_builder.add_clause({-up, opposite_down});
            cnf_builder.add_clause({-opposite_down, up});
            // if down is true, opposite edge up is true (and viceversa)
            cnf_builder.add_clause({-down, opposite_up});
            cnf_builder.add_clause({-opposite_up, down});
            // if right is true, opposite edge left is true (and viceversa)
            cnf_builder.add_clause({-right, opposite_left});
            cnf_builder.add_clause({-opposite_left, right});
            // if left is true, opposite edge right is true (and viceversa)
            cnf_builder.add_clause({-left, opposite_right});
            cnf_builder.add_clause({-opposite_right, left});
        }
}

void _one_edge_per_direction_clauses(const ColoredNodesGraph& graph, CNFBuilder& cnf_builder,
    const std::vector<std::vector<int>>& is_edge_in_direction
) {
    for (int i = 0; i < graph.size(); i++) {
        auto& neighbors = graph.getNodes()[i].getEdges();
        if (neighbors.size() == 4) {
            auto direction0 = is_edge_in_direction[i][neighbors[0].getTo()];
            auto direction1 = is_edge_in_direction[i][neighbors[1].getTo()];
            auto direction2 = is_edge_in_direction[i][neighbors[2].getTo()];
            auto direction3 = is_edge_in_direction[i][neighbors[3].getTo()];
            // at least one is true
            cnf_builder.add_clause({direction0, direction1, direction2, direction3});
        }
        if (neighbors.size() == 3) {
            auto direction0 = is_edge_in_direction[i][neighbors[0].getTo()];
            auto direction1 = is_edge_in_direction[i][neighbors[1].getTo()];
            auto direction2 = is_edge_in_direction[i][neighbors[2].getTo()];
            // at most one is true (at least 2 are false)
            cnf_builder.add_clause({-direction0, -direction1});
            cnf_builder.add_clause({-direction0, -direction2});
            cnf_builder.add_clause({-direction1, -direction2});
        }
        if (neighbors.size() == 2) {
            auto direction0 = is_edge_in_direction[i][neighbors[0].getTo()];
            auto direction1 = is_edge_in_direction[i][neighbors[1].getTo()];
            // at most one is true (at least 1 is false)
            cnf_builder.add_clause({-direction0, -direction1});
        }
    }
}

template <GraphTrait T>
void add_nodes_constraints(
    const T& graph,
    CNFBuilder& cnf_builder,
    const VariablesHandler& handler
) {
    for (int node = 0; node < graph.size(); node++) {
        _one_edge_per_direction_clauses(graph, cnf_builder, handler.is_edge_up_variable);
        _one_edge_per_direction_clauses(graph, cnf_builder, handler.is_edge_down_variable);
        _one_edge_per_direction_clauses(graph, cnf_builder, handler.is_edge_right_variable);
        _one_edge_per_direction_clauses(graph, cnf_builder, handler.is_edge_left_variable);
    }
}

void add_cycles_constraints(
    const ColoredNodesGraph& graph,
    CNFBuilder& cnf_builder,
    const std::vector<std::vector<size_t>>& cycles,
    const VariablesHandler& handler
) {
    for (auto& cycle : cycles) {
        auto at_least_one_down = std::vector<int>();
        auto at_least_one_up = std::vector<int>();
        auto at_least_one_right = std::vector<int>();
        auto at_least_one_left = std::vector<int>();
        for (size_t i = 0; i < cycle.size(); i++) {
            at_least_one_down.push_back(handler.is_edge_down_variable[cycle[i]][cycle[(i + 1) % cycle.size()]]);
            at_least_one_up.push_back(handler.is_edge_up_variable[cycle[i]][cycle[(i + 1) % cycle.size()]]);
            at_least_one_right.push_back(handler.is_edge_right_variable[cycle[i]][cycle[(i + 1) % cycle.size()]]);
            at_least_one_left.push_back(handler.is_edge_left_variable[cycle[i]][cycle[(i + 1) % cycle.size()]]);
        }
        cnf_builder.add_clause(at_least_one_down);
        cnf_builder.add_clause(at_least_one_up);
        cnf_builder.add_clause(at_least_one_right);
        cnf_builder.add_clause(at_least_one_left);
    }
}

const Shape* result_to_shape(
    const ColoredNodesGraph& graph,
    const std::vector<int>& numbers,
    const VariablesHandler& handler
) {
    auto variable_values = std::unordered_map<int, bool>();
    for (size_t i = 0; i < numbers.size(); i++) {
        int var = numbers[i];
        if (var > 0) variable_values[var] = true;
        else variable_values[-var] = false;
    }
    auto shape = new Shape();
    for (int i = 0; i < graph.size(); i++) {
        for (auto& edge : graph.getNodes()[i].getEdges()) {
            int j = edge.getTo();
            int up = handler.is_edge_up_variable[i][j];
            int down = handler.is_edge_down_variable[i][j];
            int right = handler.is_edge_right_variable[i][j];
            int left = handler.is_edge_left_variable[i][j];
            if (variable_values[up]) {
                shape->set_direction(i, j, Direction::UP);
            } else if (variable_values[down]) {
                shape->set_direction(i, j, Direction::DOWN);
            } else if (variable_values[right]) {
                shape->set_direction(i, j, Direction::RIGHT);
            } else if (variable_values[left]) {
                shape->set_direction(i, j, Direction::LEFT);
            } else {
                assert(false);
            }
        }
    }
    return shape;
}

const Shape* _build_shape(ColoredNodesGraph& colored_graph, std::vector<std::vector<size_t>>& cycles) {
    const VariablesHandler handler = _initialize_variables(colored_graph);
    CNFBuilder cnf_builder;
    add_constraints_one_direction_per_edge(colored_graph, cnf_builder, handler);
    add_constraints_opposite_edges(colored_graph, cnf_builder, handler);
    add_nodes_constraints(colored_graph, cnf_builder, handler);
    add_cycles_constraints(colored_graph, cnf_builder, cycles, handler);
    cnf_builder.convert_to_cnf(".conjunctive_normal_form.cnf");
    std::unique_ptr<const Result> results = std::unique_ptr<const Result>(launch_glucose());
    if (results->result == ResultType::UNSAT)
        return nullptr;
    const std::vector<int>& variables = results->numbers;
    const Shape* shape = result_to_shape(colored_graph, variables, handler);
    return shape;
}