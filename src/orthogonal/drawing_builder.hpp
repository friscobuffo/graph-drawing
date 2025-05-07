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

struct NodePosition {
    int m_x{-1};
    int m_y{-1};
    bool operator==(const NodePosition& other) const {
        return m_x == other.m_x && m_y == other.m_y;
    }
};

class NodesPositions {
private:
    std::vector<NodePosition> m_positions;
public:
    void set_position_x(int node, int position);
    void set_position_y(int node, int position);
    int get_position_x(int node) const;
    int get_position_y(int node) const;
    const NodePosition& get_position(int node) const;
};

int compute_total_area(const NodesPositions& positions, const ColoredNodesGraph& graph);

int compute_total_crossings(const NodesPositions& positions, const ColoredNodesGraph& graph);

std::tuple<int, int, double> compute_edge_length_metrics(const NodesPositions &positions, const ColoredNodesGraph &graph);

std::tuple<int, double> compute_bends_metrics(const ColoredNodesGraph &graph);

enum class BuildingResultType {
    OK,
    CYCLES_TO_BE_ADDED,
};

struct BuildingResult {
    const NodesPositions* positions{nullptr};
    std::vector<std::vector<int>> cycles_to_be_added;
    BuildingResultType type;
};

BuildingResult* build_nodes_positions(const Shape& shape, const ColoredNodesGraph& graph);

void node_positions_to_svg(
    const NodesPositions& positions,
    const ColoredNodesGraph& graph,
    const std::string& filename
);

// removes useless corners from the graph and from the shape
// (useless corners are red nodes with two horizontal or vertical edges)
void refine_result(
    const ColoredNodesGraph& graph,
    const Shape& shape,
    ColoredNodesGraph& refined_graph,
    Shape& refined_shape
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
    int total_edge_length;
    int max_edge_length;
    double edge_length_stddev;
    int max_bends_per_edge;
    double bends_stddev;
    int number_of_useless_bends;
};

NodesPositions* compact_area_x(
    const ColoredNodesGraph& graph,
    const Shape& shape,
    const NodesPositions& old_positions
);

NodesPositions* compact_area_y(
    const ColoredNodesGraph& graph,
    const Shape& shape,
    const NodesPositions& old_positions
);

template <GraphTrait T>
DrawingResult make_rectilinear_drawing_incremental(
    const T& graph, std::vector<std::vector<int>>& cycles
);

template <GraphTrait T>
auto make_rectilinear_drawing_incremental_basis(const T& graph) {
    auto cycles = compute_cycle_basis(graph);
    return make_rectilinear_drawing_incremental(graph, cycles);
}

template <GraphTrait T>
auto make_rectilinear_drawing_incremental_no_cycles(const T& graph) {
    std::vector<std::vector<int>> cycles;
    return make_rectilinear_drawing_incremental(graph, cycles);
}

bool check_if_drawing_has_overlappings(const ColoredNodesGraph& graph, const NodesPositions& positions);

#include "drawing_builder.ipp"

#endif
