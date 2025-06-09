#ifndef MY_GRAPH_ATTRIBUTES_H
#define MY_GRAPH_ATTRIBUTES_H

#include <any>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "core/utils.hpp"

enum class Attribute {
  NODES_COLOR,
  // EDGES_COLOR,
  // NODES_WEIGHT,
  // EDGES_WEIGHT,
  // NODES_STRING_LABEL,
  // EDGES_STRING_LABEL,
  // NODES_ANY_LABEL,
  EDGES_ANY_LABEL,
  CHAIN_EDGES,
};

class GraphAttributes {
 private:
  std::unordered_map<Attribute, std::unordered_map<int, std::any>>
      mattribute_to_node;
  IntPairHashSet m_identified_nodes;

 public:
  bool has_attribute(Attribute attribute) const;
  void add_attribute(Attribute attribute);
  bool has_attribute_by_id(Attribute attribute, int id) const;
  const std::any& get_attribute_by_id(Attribute attribute, int id) const;
  void set_node_color(int node_id, Color color);
  Color get_node_color(int node_id) const;
  void change_node_color(int node_id, Color color);
  void set_edge_any_label(int edge_id, const std::any& label);
  void set_chain_edges(int key, const std::tuple<int, int>& edge);
  std::vector<std::tuple<int, int>> get_chain_edges(int key);
  const std::any& get_edge_any_label(int edge_id) const;
  void remove_nodes_attribute(int node_id);
  bool are_nodes_identified(int node_id_1, int node_id_2) const;
  void identify_nodes(int node_id_1, int node_id_2);
};

#endif