#include "shape_builder.hpp"

#include <cstdlib>
#include <algorithm>
#include <string>
#include <unordered_set>
#include <mutex>

#include "../sat/glucose.hpp"
#include "../sat/cnf_builder.hpp"

#ifdef __linux__
const std::string CONJUNCTIVE_NORMAL_FORM_FILE = "/dev/shm/.conjunctive_normal_form.cnf";
const std::string OUTPUT_FILE = "/dev/shm/.output.txt";
const std::string PROOF_FILE = "/dev/shm/.proof.txt";
#elif __APPLE__
const std::string CONJUNCTIVE_NORMAL_FORM_FILE = ".conjunctive_normal_form.cnf";
const std::string OUTPUT_FILE = ".output.txt";
const std::string PROOF_FILE = ".proof.txt";
#endif

struct VariablesHandler {
    std::vector<std::vector<int>> is_edge_up_variable;
    std::vector<std::vector<int>> is_edge_down_variable;
    std::vector<std::vector<int>> is_edge_right_variable;
    std::vector<std::vector<int>> is_edge_left_variable;
    std::vector<std::pair<int, int>> variable_to_edge;
    std::vector<Direction> variable_to_direction;
};

const VariablesHandler initialize_variables(const ColoredNodesGraph& graph) {
    VariablesHandler handler{
        std::vector<std::vector<int>>(graph.size(), std::vector<int>(graph.size(), -1)),
        std::vector<std::vector<int>>(graph.size(), std::vector<int>(graph.size(), -1)),
        std::vector<std::vector<int>>(graph.size(), std::vector<int>(graph.size(), -1)),
        std::vector<std::vector<int>>(graph.size(), std::vector<int>(graph.size(), -1)),
        std::vector<std::pair<int, int>>(),
        std::vector<Direction>()
    };
    int next_var = 1;
    handler.variable_to_edge.push_back(std::make_pair(-1, -1));
    for (int i = 0; i < graph.size(); i++) {
        for (auto& edge : graph.get_node(i).get_edges()) {
            int j = edge.get_to();
            handler.is_edge_up_variable[i][j] = next_var;
            handler.variable_to_edge.push_back(std::make_pair(i, j));
            handler.variable_to_direction.push_back(Direction::UP);
            next_var++;
            handler.is_edge_down_variable[i][j] = next_var;
            handler.variable_to_edge.push_back(std::make_pair(i, j));
            handler.variable_to_direction.push_back(Direction::DOWN);
            next_var++;
            handler.is_edge_left_variable[i][j] = next_var;
            handler.variable_to_edge.push_back(std::make_pair(i, j));
            handler.variable_to_direction.push_back(Direction::LEFT);
            next_var++;
            handler.is_edge_right_variable[i][j] = next_var;
            handler.variable_to_edge.push_back(std::make_pair(i, j));
            handler.variable_to_direction.push_back(Direction::RIGHT);
            next_var++;
        }
    }
    return handler;
}

// each edge can only be in one direction
void add_constraints_one_direction_per_edge(
    const ColoredNodesGraph& graph,
    CnfBuilder& cnf_builder,
    const VariablesHandler& handler
) {
    for (int i = 0; i < graph.size(); i++)
        for (auto& edge : graph.get_node(i).get_edges()) {
            int j = edge.get_to();
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
    CnfBuilder& cnf_builder,
    const VariablesHandler& handler
) {
    for (int i = 0; i < graph.size(); i++)
        for (auto& edge : graph.get_node(i).get_edges()) {
            int j = edge.get_to();
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

void one_edge_per_direction_clauses(const ColoredNodesGraph& graph, CnfBuilder& cnf_builder,
    const std::vector<std::vector<int>>& is_edge_in_direction
) {
    for (int i = 0; i < graph.size(); i++) {
        auto& neighbors = graph.get_node(i).get_edges();
        if (neighbors.size() == 4) {
            auto direction0 = is_edge_in_direction[i][neighbors[0].get_to()];
            auto direction1 = is_edge_in_direction[i][neighbors[1].get_to()];
            auto direction2 = is_edge_in_direction[i][neighbors[2].get_to()];
            auto direction3 = is_edge_in_direction[i][neighbors[3].get_to()];
            // at least one is true
            cnf_builder.add_clause({direction0, direction1, direction2, direction3});
        }
        if (neighbors.size() == 3) {
            auto direction0 = is_edge_in_direction[i][neighbors[0].get_to()];
            auto direction1 = is_edge_in_direction[i][neighbors[1].get_to()];
            auto direction2 = is_edge_in_direction[i][neighbors[2].get_to()];
            // at most one is true (at least 2 are false)
            cnf_builder.add_clause({-direction0, -direction1});
            cnf_builder.add_clause({-direction0, -direction2});
            cnf_builder.add_clause({-direction1, -direction2});
        }
        if (neighbors.size() == 2) {
            auto direction0 = is_edge_in_direction[i][neighbors[0].get_to()];
            auto direction1 = is_edge_in_direction[i][neighbors[1].get_to()];
            // at most one is true (at least 1 is false)
            cnf_builder.add_clause({-direction0, -direction1});
        }
    }
}

template <GraphTrait T>
void add_nodes_constraints(
    const T& graph,
    CnfBuilder& cnf_builder,
    const VariablesHandler& handler
) {
    one_edge_per_direction_clauses(graph, cnf_builder, handler.is_edge_up_variable);
    one_edge_per_direction_clauses(graph, cnf_builder, handler.is_edge_down_variable);
    one_edge_per_direction_clauses(graph, cnf_builder, handler.is_edge_right_variable);
    one_edge_per_direction_clauses(graph, cnf_builder, handler.is_edge_left_variable);
}

void add_cycles_constraints(
    const ColoredNodesGraph& graph,
    CnfBuilder& cnf_builder,
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
        for (auto& edge : graph.get_node(i).get_edges()) {
            int j = edge.get_to();
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

size_t find_variable_of_edge_to_remove(const std::vector<std::string>& proof_lines) {
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
        assert(token == "0");
        if (tokens.size() == 1) unit_clauses.push_back(tokens[0]);
    }
    if (unit_clauses.size() == 0)
        throw std::runtime_error("Could not find the edge to remove");
    // pick one of the first three unit clauses
    int random_index = rand() % std::min((int)unit_clauses.size(), 3);
    return std::abs(unit_clauses[random_index]);
}

const Shape* build_shape_or_add_corner(ColoredNodesGraph& colored_graph, std::vector<std::vector<size_t>>& cycles);

std::pair<const Shape*,int> build_shape(ColoredNodesGraph& colored_graph, std::vector<std::vector<size_t>>& cycles) {
    const Shape* shape = build_shape_or_add_corner(colored_graph, cycles);
    int added_corners = 0;
    while (shape == nullptr) {
        shape = build_shape_or_add_corner(colored_graph, cycles);
        ++added_corners;
    }
    return {shape,added_corners};
}

std::unordered_set<size_t> used_indices;
std::mutex mutex;

size_t get_index_to_use() {
    std::lock_guard<std::mutex> lock(mutex);
    size_t index = 0;
    while (used_indices.find(index) != used_indices.end())
        index++;
    used_indices.insert(index);
    return index;
}

void free_index(size_t index) {
    std::lock_guard<std::mutex> lock(mutex);
    used_indices.erase(index);
}

std::string add_index_to_filename(const std::string& filename, const size_t index) {
    size_t last_slash = filename.find_last_of('/');
    std::string path = "";
    std::string filename_part = filename;
    if (last_slash != std::string::npos) {
        path = filename.substr(0, last_slash + 1);
        filename_part = filename.substr(last_slash + 1);
    }
    size_t dot_pos = filename_part.rfind('.');
    if (dot_pos == std::string::npos || dot_pos == 0) // No extension or hidden file without a real extension
        filename_part += "_" + std::to_string(index);
    else
        filename_part = filename_part.substr(0, dot_pos) + "_" + std::to_string(index) + filename_part.substr(dot_pos);
    return path + filename_part;
}

const Shape* build_shape_or_add_corner(
    ColoredNodesGraph& colored_graph,
    std::vector<std::vector<size_t>>& cycles
) {
    const VariablesHandler handler = initialize_variables(colored_graph);
    CnfBuilder cnf_builder;
    cnf_builder.add_comment("constraints one direction per edge");
    add_constraints_one_direction_per_edge(colored_graph, cnf_builder, handler);
    cnf_builder.add_comment("constraints opposite edges");
    add_constraints_opposite_edges(colored_graph, cnf_builder, handler);
    cnf_builder.add_comment("constraints nodes");
    add_nodes_constraints(colored_graph, cnf_builder, handler);
    cnf_builder.add_comment("constraints cycles");
    add_cycles_constraints(colored_graph, cnf_builder, cycles, handler);
    const size_t index = get_index_to_use();
    const std::string cnf = add_index_to_filename(CONJUNCTIVE_NORMAL_FORM_FILE, index);
    const std::string output = add_index_to_filename(OUTPUT_FILE, index);
    const std::string proof = add_index_to_filename(PROOF_FILE, index);
    cnf_builder.convert_to_cnf(cnf);
    auto results = std::unique_ptr<const GlucoseResult>(launch_glucose(
        cnf,
        output,
        proof
    ));
    free_index(index);
    if (results->result == GlucoseResultType::UNSAT) {
        const size_t variable_edge = find_variable_of_edge_to_remove(results->proof_lines);
        const int i = handler.variable_to_edge[variable_edge].first;
        const int j = handler.variable_to_edge[variable_edge].second;
        const size_t new_node_index = colored_graph.size();
        colored_graph.remove_undirected_edge(i, j);
        colored_graph.add_node(Color::RED);
        colored_graph.add_undirected_edge(i, new_node_index);
        colored_graph.add_undirected_edge(j, new_node_index);
        for (auto& cycle : cycles) {
            for (size_t k = 0; k < cycle.size(); k++) {
                if (cycle[k] == i && cycle[(k + 1) % cycle.size()] == j) {
                    cycle.insert(cycle.begin() + k + 1, new_node_index);
                    break;
                }
                if (cycle[k] == j && cycle[(k + 1) % cycle.size()] == i) {
                    cycle.insert(cycle.begin() + k + 1, new_node_index);
                    break;
                }
            }
        }
        return nullptr;
    }
    const std::vector<int>& variables = results->numbers;
    const Shape* shape = result_to_shape(colored_graph, variables, handler);
    return shape;
}