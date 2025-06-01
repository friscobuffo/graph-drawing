#include "core/graph/attributes.hpp"

bool GraphAttributes::has_attribute(Attribute attribute) const {
  return mattribute_to_node.contains(attribute);
}

void GraphAttributes::add_attribute(Attribute attribute) {
  if (has_attribute(attribute))
    throw std::runtime_error(
        "GraphAttributes::add_attribute: already has this attribute");
  mattribute_to_node[attribute] = {};
}

bool GraphAttributes::has_attribute_by_id(Attribute attribute, int id) const {
  if (!has_attribute(attribute))
    throw std::runtime_error(
        "GraphAttributes::has_attribute_by_id: does not have attribute");
  return mattribute_to_node.at(attribute).contains(id);
}

const std::any& GraphAttributes::get_attribute_by_id(Attribute attribute,
                                                     int id) const {
  if (!has_attribute_by_id(attribute, id))
    throw std::runtime_error(
        "GraphAttributes::get_attribute_by_id: does not have attribute");
  return mattribute_to_node.at(attribute).at(id);
}

void GraphAttributes::set_node_color(int node_id, Color color) {
  if (has_attribute_by_id(Attribute::NODES_COLOR, node_id))
    throw std::runtime_error(
        "GraphAttributes::set_node_color: the node already has color");
  mattribute_to_node.at(Attribute::NODES_COLOR)[node_id] = color;
}

void GraphAttributes::set_chain_edges(int key,
                                      const std::tuple<int, int>& edge) {
  if (!has_attribute(Attribute::CHAIN_EDGES))
    add_attribute(Attribute::CHAIN_EDGES);
  auto& chain_edges = mattribute_to_node[Attribute::CHAIN_EDGES];
  if (chain_edges.find(key) == chain_edges.end()) {
    chain_edges[key] = std::vector<std::tuple<int, int>>{};
  }
  auto& any_value = chain_edges[key];
  auto& vec = std::any_cast<std::vector<std::tuple<int, int>>&>(any_value);
  vec.push_back(edge);
}

std::vector<std::tuple<int, int>> GraphAttributes::get_chain_edges(int key) {
  if (!has_attribute_by_id(Attribute::CHAIN_EDGES, key))
    throw std::runtime_error(
        "GraphAttributes::get_chain_edges: the chain doesn't exist");
  const std::any& any_value =
      mattribute_to_node.at(Attribute::CHAIN_EDGES).at(key);
  return std::any_cast<const std::vector<std::tuple<int, int>>&>(any_value);
}

Color GraphAttributes::get_node_color(int node_id) const {
  if (!has_attribute_by_id(Attribute::NODES_COLOR, node_id))
    throw std::runtime_error(
        "GraphAttributes::get_node_color: the node does not have a color");
  return std::any_cast<Color>(
      mattribute_to_node.at(Attribute::NODES_COLOR).at(node_id));
}

void GraphAttributes::set_edge_any_label(int edge_id, const std::any& label) {
  if (has_attribute_by_id(Attribute::EDGES_ANY_LABEL, edge_id))
    throw std::runtime_error(
        "GraphAttributes::set_edge_any_label: the edge already has a label");
  mattribute_to_node.at(Attribute::EDGES_ANY_LABEL)[edge_id] = label;
}

const std::any& GraphAttributes::get_edge_any_label(int edge_id) const {
  if (!has_attribute_by_id(Attribute::EDGES_ANY_LABEL, edge_id))
    throw std::runtime_error(
        "GraphAttributes::get_edge_any_label: the edge does not have a label");
  return mattribute_to_node.at(Attribute::EDGES_ANY_LABEL).at(edge_id);
}