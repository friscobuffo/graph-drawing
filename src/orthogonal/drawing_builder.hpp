#ifndef MY_DRAWING_BUILDER_H
#define MY_DRAWING_BUILDER_H

#include <cassert>
#include <unordered_set>

#include "shape.hpp"
#include "../core/graph/graph.hpp"
#include "../core/graph/node.hpp"
#include "../core/utils.hpp"
#include "../core/graph/algorithms.hpp"
#include "../drawing/svg_drawer.hpp"
#include "../drawing/linear_scale.hpp"
#include "shape_builder.hpp"

class NodesPositions {
private:
    std::vector<std::pair<int, int>> m_positions;
public:
    void set_position_x(size_t node, size_t position);
    void set_position_y(size_t node, size_t position);
    int get_position_x(size_t node) const;
    int get_position_y(size_t node) const;
};

enum class BuildingResultType {
    OK,
    CYCLE_TO_BE_ADDED,
};

struct BuildingResult {
    const NodesPositions* positions{nullptr};
    std::vector<size_t> cycle_to_be_added;
    BuildingResultType type;
};

BuildingResult* build_nodes_positions(const Shape& shape, const ColoredNodesGraph& graph);

void node_positions_to_svg(const NodesPositions& positions, const ColoredNodesGraph& graph);

template <GraphTrait T>
void make_rectilinear_drawing_incremental(const T& graph, std::vector<std::vector<size_t>>& cycles) {
    std::cout << "Number of cycles: " << cycles.size() << std::endl;
    ColoredNodesGraph colored_graph{};
    for (int i = 0; i < graph.size(); i++)
        colored_graph.add_node(Color::BLACK);
    for (int i = 0; i < graph.size(); i++) {
        assert(graph.get_nodes()[i].get_edges().size() <= 4);
        for (auto& edge : graph.get_nodes()[i].get_edges()) {
            int j = edge.get_to();
            colored_graph.add_edge(i, j);
        }
    }
    auto shape_added_corners = build_shape(colored_graph, cycles);
    const Shape* shape = shape_added_corners.first;
    int number_of_added_corners = shape_added_corners.second;
    BuildingResult* result = build_nodes_positions(*shape, colored_graph);
    int number_of_added_cycles = 0;
    while (result->type == BuildingResultType::CYCLE_TO_BE_ADDED) {
        cycles.push_back(result->cycle_to_be_added);
        number_of_added_cycles++;
        delete shape;
        delete result;
        auto shape_added_corners_2 = build_shape(colored_graph, cycles);
        shape = shape_added_corners_2.first;
        number_of_added_corners += shape_added_corners_2.second;
        result = build_nodes_positions(*shape, colored_graph);
    }
    node_positions_to_svg(*result->positions, colored_graph);
    delete shape;
    delete result->positions;
    delete result;
    std::cout << "Number of added cycles: " << number_of_added_cycles << std::endl;
    std::cout << "Number of added corners: " << number_of_added_corners << std::endl;
}

template <GraphTrait T>
void make_rectilinear_drawing_incremental_basis(const T& graph) {
    auto cycles = compute_cycle_basis(graph);
    make_rectilinear_drawing_incremental(graph, cycles);
}

template <GraphTrait T>
void make_rectilinear_drawing_incremental_triplets(const T& graph) {
    auto cycles = compute_cycles_in_undirected_graph_triplets(graph);
    make_rectilinear_drawing_incremental(graph, cycles);
}

template <GraphTrait T>
void make_rectilinear_drawing_incremental_no_cycles(const T& graph) {
    std::vector<std::vector<size_t>> cycles;
    make_rectilinear_drawing_incremental(graph, cycles);
}

template <GraphTrait T>
void make_rectilinear_drawing_all_cycles(const T& graph) {
    auto cycles = compute_all_cycles_in_undirected_graph(graph);
    ColoredNodesGraph colored_graph{};
    for (int i = 0; i < graph.size(); i++)
        colored_graph.add_node(Color::BLACK);
    for (int i = 0; i < graph.size(); i++) {
        assert(graph.get_nodes()[i].get_edges().size() <= 4);
        for (auto& edge : graph.get_nodes()[i].get_edges()) {
            int j = edge.get_to();
            colored_graph.add_edge(i, j);
        }
    }
    const Shape* shape = build_shape(colored_graph, cycles);
    BuildingResult* result = build_nodes_positions(*shape, colored_graph);
    assert(result->type == BuildingResultType::OK);
    node_positions_to_svg(*result->positions, colored_graph);
    delete shape;
    delete result->positions;
    delete result;
}

#endif