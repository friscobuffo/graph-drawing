#ifndef MY_DRAWING_BUILDER_H
#define MY_DRAWING_BUILDER_H

#include <memory>
#include <optional>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "core/graph/graph.hpp"
#include "core/graph/graphs_algorithms.hpp"
#include "core/utils.hpp"
#include "drawing/linear_scale.hpp"
#include "drawing/svg_drawer.hpp"
#include "orthogonal/shape/shape.hpp"
#include "orthogonal/shape/shape_builder.hpp"

struct NodePosition {
  float m_x;
  float m_y;
  NodePosition(float x, float y) : m_x(x), m_y(y) {}
  bool operator==(const NodePosition& other) const {
    return m_x == other.m_x && m_y == other.m_y;
  }
};

class NodesPositions {
 private:
  std::unordered_map<int, NodePosition> m_nodeid_to_position_map;

 public:
  void set_position(int node, float position_x, float position_y);
  void change_position(int node, float position_x, float position_y);
  void change_position_x(int node, float position_x);
  void change_position_y(int node, float position_y);
  float get_position_x(int node) const;
  float get_position_y(int node) const;
  void x_right_shift(float x_pos);
  void x_left_shift(float x_pos);
  void y_up_shift(float y_pos);
  void y_down_shift(float y_pos);
  bool has_position(int node) const;
  void remove_position(int node);
  const NodePosition& get_position(int node) const;
};

void node_positions_to_svg(const NodesPositions& positions, const Graph& graph,
                           const GraphAttributes& attributes,
                           const std::string& filename);

struct DrawingResult {
  std::unique_ptr<Graph> augmented_graph;
  GraphAttributes attributes;
  Shape shape;
  NodesPositions positions;
  int initial_number_of_cycles;
  int number_of_added_cycles;
  int number_of_useless_bends;
};

DrawingResult make_orthogonal_drawing(const Graph& graph);

bool check_if_drawing_has_overlappings(const Graph& graph,
                                       const NodesPositions& positions);

DrawingResult make_orthogonal_drawing_sperimental(const Graph& graph);

void add_back_removed_edge(DrawingResult& result,
                           const std::pair<int, int>& edge);

bool do_edges_cross(const NodesPositions& positions, int i, int j, int k,
                    int l);

void prova_special();

#endif
