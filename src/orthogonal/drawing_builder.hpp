#ifndef MY_DRAWING_BUILDER_H
#define MY_DRAWING_BUILDER_H

#include <cassert>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <tuple>

#include "shape.hpp"
#include "../core/graph/graph.hpp"
#include "../core/utils.hpp"
#include "../core/graph/graphs_algorithms.hpp"
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

int compute_total_area(const NodesPositions& positions, const ColoredNodesGraph& graph);

int compute_total_crossings(const NodesPositions& positions, const ColoredNodesGraph& graph);

enum class BuildingResultType {
    OK,
    CYCLES_TO_BE_ADDED,
};

struct BuildingResult {
    const NodesPositions* positions{nullptr};
    std::vector<std::vector<size_t>> cycles_to_be_added;
    BuildingResultType type;
};

BuildingResult* build_nodes_positions(const Shape& shape, const ColoredNodesGraph& graph);

void node_positions_to_svg(
    const NodesPositions& positions,
    const ColoredNodesGraph& graph,
    const std::string& filename
);

struct DrawingResult {
    std::unique_ptr<const ColoredNodesGraph> augmented_graph;
    std::unique_ptr<const Shape> shape;
    std::unique_ptr<const NodesPositions> positions;
    int crossings;
    int bends;
    int area;
    int initial_number_of_cycles;
    int number_of_added_cycles;
};

template <GraphTrait T>
DrawingResult make_rectilinear_drawing_incremental(
    const T& graph, std::vector<std::vector<size_t>>& cycles
) {
    ColoredNodesGraph* colored_graph = new ColoredNodesGraph{};
    for (int i = 0; i < graph.size(); i++)
        colored_graph->add_node(Color::BLACK);
    for (int i = 0; i < graph.size(); i++) {
        assert(graph.get_node(i).get_degree() <= 4);
        for (auto& edge : graph.get_node(i).get_edges()) {
            int j = edge.get_to();
            colored_graph->add_edge(i, j);
        }
    }
    auto shape_added_corners = build_shape(*colored_graph, cycles);
    const Shape* shape = shape_added_corners.first;
    int number_of_added_corners = shape_added_corners.second;
    BuildingResult* result = build_nodes_positions(*shape, *colored_graph);
    int number_of_added_cycles = 0;
    while (result->type == BuildingResultType::CYCLES_TO_BE_ADDED) {
        for (auto& cycle_to_add : result->cycles_to_be_added)
            cycles.push_back(cycle_to_add);
        number_of_added_cycles += result->cycles_to_be_added.size();
        delete shape;
        delete result;
        auto shape_added_corners_2 = build_shape(*colored_graph, cycles);
        shape = shape_added_corners_2.first;
        number_of_added_corners += shape_added_corners_2.second;
        result = build_nodes_positions(*shape, *colored_graph);
    }
    const NodesPositions* positions = result->positions;
    delete result;
    return {
        std::unique_ptr<const ColoredNodesGraph>(colored_graph),
        std::unique_ptr<const Shape>(shape),
        std::unique_ptr<const NodesPositions>(positions),
        compute_total_crossings(*positions, *colored_graph),
        number_of_added_corners,
        compute_total_area(*positions, *colored_graph),
        (int)cycles.size() - number_of_added_cycles,
        number_of_added_cycles
    };
}

template <GraphTrait T>
auto make_rectilinear_drawing_incremental_basis(const T& graph) {
    auto cycles = compute_cycle_basis(graph);
    return make_rectilinear_drawing_incremental(graph, cycles);
}

template <GraphTrait T>
auto make_rectilinear_drawing_incremental_triplets(const T& graph) {
    auto cycles = compute_cycles_in_undirected_graph_triplets(graph);
    return make_rectilinear_drawing_incremental(graph, cycles);
}

template <GraphTrait T>
auto make_rectilinear_drawing_incremental_triplets_basis(const T& graph) {
    auto cycles = compute_cycles_in_undirected_graph_triplets(graph);
    auto cycles_basis = compute_cycle_basis(graph);
    for (auto& cycle : cycles_basis)
        cycles.push_back(cycle);
    return make_rectilinear_drawing_incremental(graph, cycles);
}

template <GraphTrait T>
auto make_rectilinear_drawing_incremental_no_cycles(const T& graph) {
    std::vector<std::vector<size_t>> cycles;
    return make_rectilinear_drawing_incremental(graph, cycles);
}

template <GraphTrait T>
auto make_rectilinear_drawing_incremental_pairs(const T& graph) {
    auto cycles = compute_smallest_cycle_between_pair_nodes(graph);
    return make_rectilinear_drawing_incremental(graph, cycles);
}

template <GraphTrait T>
auto make_rectilinear_drawing_incremental_disjoint_paths(const T& graph) {
    auto cycles = compute_cycles_disjoint_paths(graph);
    std::unordered_map<int, std::vector<std::vector<size_t>>> cycles_map;
    for (auto& cycle : cycles) {
        int sum = 0;
        for (auto& node : cycle) sum += node;
        if (cycles_map.find(sum) == cycles_map.end())
            cycles_map[sum] = std::vector<std::vector<size_t>>{};
        cycles_map[sum].push_back(cycle);
    }
    // lets find duplicates
    std::vector<std::vector<size_t>> cycles_filtered;
    for (auto& [key, cycles_same_sum] : cycles_map) {
        for (int i = 0; i < cycles_same_sum.size(); i++)
            for (int j = i + 1; j < cycles_same_sum.size(); j++)
                if (are_cycles_equivalent(cycles_same_sum[i], cycles_same_sum[j])) {
                    cycles_same_sum.erase(cycles_same_sum.begin() + j);
                    j--;
                }
        for (auto& cycle : cycles_same_sum)
            cycles_filtered.push_back(cycle);
    }
    return make_rectilinear_drawing_incremental(graph, cycles_filtered);
}

// template <GraphTrait T>
// void make_rectilinear_drawing_all_cycles(const T& graph) {
//     auto cycles = compute_all_cycles_in_undirected_graph(graph);
//     ColoredNodesGraph colored_graph{};
//     for (int i = 0; i < graph.size(); i++)
//         colored_graph.add_node(Color::BLACK);
//     for (int i = 0; i < graph.size(); i++) {
//         assert(graph.get_node(i).get_degree() <= 4);
//         for (auto& edge : graph.get_node(i).get_edges()) {
//             int j = edge.get_to();
//             colored_graph.add_edge(i, j);
//         }
//     }
//     const Shape* shape = build_shape(colored_graph, cycles);
//     BuildingResult* result = build_nodes_positions(*shape, colored_graph);
//     assert(result->type == BuildingResultType::OK);
//     delete shape;
//     delete result->positions;
//     delete result;
// }

#endif
