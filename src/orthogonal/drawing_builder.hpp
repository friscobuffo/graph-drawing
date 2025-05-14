#ifndef MY_DRAWING_BUILDER_H
#define MY_DRAWING_BUILDER_H

#include <cassert>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <optional>
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
    std::unordered_map<int, NodePosition> m_nodeid_to_position_map;
public:
    void set_position(int node, int position_x, int position_y);
    int get_position_x(int node) const;
    int get_position_y(int node) const;
    bool has_position(int node) const;
    void remove_position(int node);
    const NodePosition& get_position(int node) const;
};

int compute_total_area(
    const NodesPositions& positions,
    const Graph& graph
);

int compute_total_crossings(
    const NodesPositions& positions,
    const Graph& graph
);

std::tuple<int, int, double> compute_edge_length_metrics(
    const NodesPositions& positions,
    const Graph& graph,
    const GraphAttributes& attributes
);

std::tuple<int, double> compute_bends_metrics(
    const Graph& graph,
    const GraphAttributes& attributes
);

enum class BuildingResultType {
    OK,
    CYCLES_TO_BE_ADDED,
};

struct BuildingResult {
    std::optional<NodesPositions> positions;
    std::vector<std::vector<int>> cycles_to_be_added;
    BuildingResultType type;
};

BuildingResult build_nodes_positions(
    const Shape& shape,
    const Graph& graph
);

void node_positions_to_svg(
    const NodesPositions& positions,
    const Graph& graph,
    const GraphAttributes& attributes,
    const std::string& filename
);

struct DrawingResult {
    std::unique_ptr<Graph> augmented_graph;
    GraphAttributes attributes;
    Shape shape;
    NodesPositions positions;
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

DrawingResult make_rectilinear_drawing_incremental_basis(const Graph& graph);

DrawingResult make_rectilinear_drawing_incremental_no_cycles(const Graph& graph);

bool check_if_drawing_has_overlappings(
    const Graph& graph,
    const NodesPositions& positions
);

#endif
