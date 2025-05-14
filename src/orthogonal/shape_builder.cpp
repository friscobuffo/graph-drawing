#include "shape_builder.hpp"

#include <cstdlib>
#include <algorithm>
#include <string>
#include <unordered_set>
#include <mutex>
#include <optional>
#include <cassert>
#include <utility>

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

class VariablesHandler {
private:
    std::unordered_map<int,std::unordered_map<int,int>> is_edge_up_variable;
    std::unordered_map<int,std::unordered_map<int,int>> is_edge_down_variable;
    std::unordered_map<int,std::unordered_map<int,int>> is_edge_right_variable;
    std::unordered_map<int,std::unordered_map<int,int>> is_edge_left_variable;
    std::unordered_map<int,std::pair<int,int>> variable_to_edge;
    std::unordered_map<int,Direction> variable_to_direction;
public:
    VariablesHandler(const Graph& graph) {
        for (auto& node : graph.get_nodes()) {
            is_edge_up_variable[node.get_id()] = std::unordered_map<int,int>();
            is_edge_down_variable[node.get_id()] = std::unordered_map<int,int>();
            is_edge_left_variable[node.get_id()] = std::unordered_map<int,int>();
            is_edge_right_variable[node.get_id()] = std::unordered_map<int,int>();
        }
        int next_var = 1;
        for (const auto& node : graph.get_nodes()) {
            int i = node.get_id();
            for (auto& edge : node.get_edges()) {
                int j = edge.get_to().get_id();
                if (i > j) continue;
                is_edge_up_variable[i][j] = next_var;
                variable_to_edge[next_var] = std::make_pair(i, j);
                variable_to_direction[next_var] = Direction::UP;
                next_var++;
                is_edge_down_variable[i][j] = next_var;
                variable_to_edge[next_var] = std::make_pair(i, j);
                variable_to_direction[next_var] = Direction::DOWN;
                next_var++;
                is_edge_left_variable[i][j] = next_var;
                variable_to_edge[next_var] = std::make_pair(i, j);
                variable_to_direction[next_var] = Direction::LEFT;
                next_var++;
                is_edge_right_variable[i][j] = next_var;
                variable_to_edge[next_var] = std::make_pair(i, j);
                variable_to_direction[next_var] = Direction::RIGHT;
                next_var++;
    
                is_edge_down_variable[j][i] = is_edge_up_variable[i][j];
                is_edge_up_variable[j][i] = is_edge_down_variable[i][j];
                is_edge_left_variable[j][i] = is_edge_right_variable[i][j];
                is_edge_right_variable[j][i] = is_edge_left_variable[i][j];
            }
        }
    }
    int get_up_variable(int i, int j) const {
        return is_edge_up_variable.at(i).at(j);
    }
    int get_down_variable(int i, int j) const {
        return is_edge_down_variable.at(i).at(j);
    }
    int get_left_variable(int i, int j) const {
        return is_edge_left_variable.at(i).at(j);
    }
    int get_right_variable(int i, int j) const {
        return is_edge_right_variable.at(i).at(j);
    }
    int get_variable(int i, int j, Direction direction) const {
        if (direction == Direction::UP)
            return get_up_variable(i, j);
        if (direction == Direction::DOWN)
            return get_down_variable(i, j);
        if (direction == Direction::LEFT)
            return get_left_variable(i, j);
        if (direction == Direction::RIGHT)
            return get_right_variable(i, j);
        throw std::invalid_argument("Invalid direction");
    }
    const std::pair<int, int>& get_edge_of_variable(int variable) const {
        return variable_to_edge.at(variable);
    }
};

// each edge can only be in one direction
void add_constraints_one_direction_per_edge(
    const Graph& graph,
    CnfBuilder& cnf_builder,
    const VariablesHandler& handler
) {
    for (auto& node : graph.get_nodes()) {
        int i = node.get_id();
        for (auto& edge : node.get_edges()) {
            int j = edge.get_to().get_id();
            if (i > j) continue;
            int up = handler.get_up_variable(i, j);
            int down = handler.get_down_variable(i, j);
            int right = handler.get_right_variable(i, j);
            int left = handler.get_left_variable(i, j);
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
}

void one_edge_per_direction_clauses(
    const Graph& graph,
    CnfBuilder& cnf_builder,
    const VariablesHandler& handler,
    const Direction direction
) {
    for (auto& node : graph.get_nodes()) {
        int i = node.get_id();
        int degree = node.get_degree();
        if (degree == 4) {
            std::vector<int> clause;
            for (auto& edge : node.get_edges())
                clause.push_back(handler.get_variable(i, edge.get_to().get_id(), direction));
            // at least one is true
            cnf_builder.add_clause(clause);
        }
        else if (degree == 3) {
            std::vector<int> variables;
            for (auto& edge : node.get_edges())
                variables.push_back(handler.get_variable(i, edge.get_to().get_id(), direction));
            // at most one is true (at least 2 are false)
            cnf_builder.add_clause({-variables[0], -variables[1]});
            cnf_builder.add_clause({-variables[0], -variables[2]});
            cnf_builder.add_clause({-variables[1], -variables[2]});
        }
        else if (degree == 2) {
            std::vector<int> clause;
            for (auto& edge : node.get_edges())
                clause.push_back(-handler.get_variable(i, edge.get_to().get_id(), direction));
            // at most one is true (at least 1 is false)
            cnf_builder.add_clause(clause);
        }
    }
}

void add_nodes_constraints(
    const Graph& graph,
    CnfBuilder& cnf_builder,
    const VariablesHandler& handler
) {
    one_edge_per_direction_clauses(graph, cnf_builder, handler, Direction::UP);
    one_edge_per_direction_clauses(graph, cnf_builder, handler, Direction::DOWN);
    one_edge_per_direction_clauses(graph, cnf_builder, handler, Direction::RIGHT);
    one_edge_per_direction_clauses(graph, cnf_builder, handler, Direction::LEFT);
}

void add_cycles_constraints(
    const Graph& graph,
    CnfBuilder& cnf_builder,
    const std::vector<std::vector<int>>& cycles,
    const VariablesHandler& handler
) {
    for (auto& cycle : cycles) {
        auto at_least_one_down = std::vector<int>();
        auto at_least_one_up = std::vector<int>();
        auto at_least_one_right = std::vector<int>();
        auto at_least_one_left = std::vector<int>();
        for (int i = 0; i < cycle.size(); i++) {
            at_least_one_down.push_back(handler.get_down_variable(cycle[i], cycle[(i + 1) % cycle.size()]));
            at_least_one_up.push_back(handler.get_up_variable(cycle[i], cycle[(i + 1) % cycle.size()]));
            at_least_one_right.push_back(handler.get_right_variable(cycle[i], cycle[(i + 1) % cycle.size()]));
            at_least_one_left.push_back(handler.get_left_variable(cycle[i], cycle[(i + 1) % cycle.size()]));
        }
        cnf_builder.add_clause(at_least_one_down);
        cnf_builder.add_clause(at_least_one_up);
        cnf_builder.add_clause(at_least_one_right);
        cnf_builder.add_clause(at_least_one_left);
    }
}

Shape result_to_shape(
    const Graph& graph,
    const std::vector<int>& numbers,
    const VariablesHandler& handler
) {
    auto variable_values = std::unordered_map<int, bool>();
    for (int i = 0; i < numbers.size(); i++) {
        int var = numbers[i];
        if (var > 0) variable_values[var] = true;
        else variable_values[-var] = false;
    }
    Shape shape;
    for (const auto& node : graph.get_nodes()) {
        int i = node.get_id();
        for (auto& edge : node.get_edges()) {
            int j = edge.get_to().get_id();
            int up = handler.get_up_variable(i, j);
            int down = handler.get_down_variable(i, j);
            int right = handler.get_right_variable(i, j);
            int left = handler.get_left_variable(i, j);
            if (variable_values[up]) {
                shape.set_direction(i, j, Direction::UP);
            } else if (variable_values[down]) {
                shape.set_direction(i, j, Direction::DOWN);
            } else if (variable_values[right]) {
                shape.set_direction(i, j, Direction::RIGHT);
            } else if (variable_values[left]) {
                shape.set_direction(i, j, Direction::LEFT);
            } else {
                throw std::runtime_error(
                    "No direction found for edge " + std::to_string(i) + " " + std::to_string(j)
                );
            }
        }
    }
    return std::move(shape);
}

int find_variable_of_edge_to_remove(const std::vector<std::string>& proof_lines) {
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
    // pick one of the first two unit clauses
    int random_index = rand() % std::min((int)unit_clauses.size(), 2);
    return std::abs(unit_clauses[random_index]);
}

std::optional<Shape> build_shape_or_add_corner(
    Graph& graph,
    GraphAttributes& attributes,
    std::vector<std::vector<int>>& cycles
);

Shape build_shape(
    Graph& graph, 
    GraphAttributes& attributes,
    std::vector<std::vector<int>>& cycles
) {
    auto shape = build_shape_or_add_corner(graph, attributes, cycles);
    while (!shape.has_value())
        shape = build_shape_or_add_corner(graph, attributes, cycles);
    return std::move(shape.value());
}

std::unordered_set<int> used_indices;
std::mutex mutex;

int get_index_to_use() {
    std::lock_guard<std::mutex> lock(mutex);
    int index = 0;
    while (used_indices.contains(index))
        index++;
    used_indices.insert(index);
    return index;
}

void free_index(int index) {
    std::lock_guard<std::mutex> lock(mutex);
    used_indices.erase(index);
}

std::string add_index_to_filename(const std::string& filename, const int index) {
    int last_slash = filename.find_last_of('/');
    std::string path = "";
    std::string filename_part = filename;
    if (last_slash != std::string::npos) {
        path = filename.substr(0, last_slash + 1);
        filename_part = filename.substr(last_slash + 1);
    }
    int dot_pos = filename_part.rfind('.');
    if (dot_pos == std::string::npos || dot_pos == 0) // No extension or hidden file without a real extension
        filename_part += "_" + std::to_string(index);
    else
        filename_part = filename_part.substr(0, dot_pos) + "_" + std::to_string(index) + filename_part.substr(dot_pos);
    return path + filename_part;
}

std::optional<Shape> build_shape_or_add_corner(
    Graph& graph,
    GraphAttributes& attributes,
    std::vector<std::vector<int>>& cycles
) {
    const VariablesHandler handler(graph);
    CnfBuilder cnf_builder;
    cnf_builder.add_comment("constraints one direction per edge");
    add_constraints_one_direction_per_edge(graph, cnf_builder, handler);
    cnf_builder.add_comment("constraints nodes");
    add_nodes_constraints(graph, cnf_builder, handler);
    cnf_builder.add_comment("constraints cycles");
    add_cycles_constraints(graph, cnf_builder, cycles, handler);
    const int index = get_index_to_use();
    const std::string cnf = add_index_to_filename(CONJUNCTIVE_NORMAL_FORM_FILE, index);
    const std::string output = add_index_to_filename(OUTPUT_FILE, index);
    const std::string proof = add_index_to_filename(PROOF_FILE, index);
    cnf_builder.convert_to_cnf(cnf);
    auto results = launch_glucose(cnf, output, proof);
    free_index(index);
    if (results.result == GlucoseResultType::UNSAT) {
        const int variable_edge = find_variable_of_edge_to_remove(results.proof_lines);
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